#include "VisionSystem.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#elif __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#elif __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

namespace SZM::AI::Vision {

VisionSystem::~VisionSystem() {
    if (m_VideoWriter) {
        delete m_VideoWriter;
        m_VideoWriter = nullptr;
    }
}

VisionSystem& VisionSystem::GetInstance() {
    static VisionSystem instance;
    return instance;
}

void VisionSystem::Initialize() {
    if (m_IsInitialized) return;
    
    // OpenCV is already included via header
    m_IsInitialized = true;
    std::cout << "[VisionSystem] Initialized successfully" << std::endl;
}

void VisionSystem::Shutdown() {
    if (!m_IsInitialized) return;
    
    StopRecording();
    ClearCache();
    m_IsInitialized = false;
    
    std::cout << "[VisionSystem] Shutdown complete" << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SCREEN CAPTURE IMPLEMENTATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ScreenCapture VisionSystem::CaptureScreen(int monitor_index) {
    ScreenCapture capture;
    capture.id = "capture_" + std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    capture.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    // Platform-specific capture
    capture.image = CaptureScreenImpl(monitor_index);
    
    if (capture.image.empty()) {
        // Create placeholder if capture failed
        capture.image = cv::Mat(m_Config.capture_height, m_Config.capture_width, CV_8UC3, cv::Scalar(40, 40, 40));
        capture.window_title = "Unknown";
    } else {
        capture.window_title = GetWindowTitleImpl(monitor_index);
    }
    
    capture.thumbnail = GenerateThumbnail(capture.image);
    
    // Detect UI elements
    if (m_Config.enable_element_detection) {
        capture.detected_elements = DetectElements(capture.image);
    }
    
    CacheCapture(capture);
    return capture;
}

ScreenCapture VisionSystem::CaptureRegion(const cv::Rect& region, int monitor_index) {
    ScreenCapture full_capture = CaptureScreen(monitor_index);
    
    ScreenCapture region_capture;
    region_capture.id = full_capture.id + "_region";
    region_capture.timestamp = full_capture.timestamp;
    region_capture.window_title = full_capture.window_title;
    
    // Crop to region
    if (region.x >= 0 && region.y >= 0 && 
        region.x + region.width <= full_capture.image.cols &&
        region.y + region.height <= full_capture.image.rows) {
        region_capture.image = full_capture.image(region).clone();
    } else {
        region_capture.image = full_capture.image;
    }
    
    region_capture.thumbnail = GenerateThumbnail(region_capture.image);
    
    // Adjust element positions to region
    for (const auto& elem : full_capture.detected_elements) {
        cv::Rect adjusted_box = elem.bounding_box - cv::Point(region.x, region.y);
        if (adjusted_box.x + adjusted_box.width > 0 && adjusted_box.y + adjusted_box.height > 0) {
            VisualElement adjusted_elem = elem;
            adjusted_elem.bounding_box = adjusted_box;
            region_capture.detected_elements.push_back(adjusted_elem);
        }
    }
    
    return region_capture;
}

ScreenCapture VisionSystem::CaptureWindow(const std::string& window_title) {
    return CaptureScreen(0); // Simplified - would need window enumeration
}

std::vector<ScreenCapture> VisionSystem::CaptureAllMonitors() {
    std::vector<ScreenCapture> captures;
    int monitor_count = 1; // Default to 1, would need platform-specific code for multi-monitor
    
#ifdef _WIN32
    monitor_count = GetSystemMetrics(SM_CMONITORS);
#endif
    
    for (int i = 0; i < monitor_count; i++) {
        captures.push_back(CaptureScreen(i));
    }
    
    return captures;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PLATFORM-SPECIFIC IMPLEMENTATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

cv::Mat VisionSystem::CaptureScreenImpl(int monitor_index) {
#ifdef _WIN32
    // Windows screen capture using GDI+
    HWND hwnd = GetDesktopWindow();
    HDC hdcScreen = GetDC(hwnd);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    
    int screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
    int screen_height = GetSystemMetrics(SM_CYFULLSCREEN);
    
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screen_width, screen_height);
    SelectObject(hdcMem, hBitmap);
    
    BitBlt(hdcMem, 0, 0, screen_width, screen_height, hdcScreen, 0, 0, SRCCOPY);
    
    // Convert HBITMAP to cv::Mat
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screen_width;
    bi.biHeight = -screen_height; // Negative for top-down
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    
    cv::Mat mat(screen_height, screen_width, CV_8UC3);
    GetDIBits(hdcMem, hBitmap, 0, screen_height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    
    // Convert BGR to RGB
    cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
    
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcScreen);
    
    return mat;
    
#elif __linux__
    // Linux X11 screen capture
    Display* display = XOpenDisplay(nullptr);
    if (!display) return cv::Mat();
    
    Window root = DefaultRootWindow(display);
    XWindowAttributes attributes;
    XGetWindowAttributes(display, root, &attributes);
    
    XImage* image = XGetImage(display, root, 0, 0, 
                               attributes.width, attributes.height, AllPlanes, ZPixmap);
    
    if (!image) {
        XCloseDisplay(display);
        return cv::Mat();
    }
    
    cv::Mat mat(attributes.height, attributes.width, CV_8UC3);
    
    for (int y = 0; y < attributes.height; y++) {
        for (int x = 0; x < attributes.width; x++) {
            unsigned long pixel = XGetPixel(image, x, y);
            mat.at<cv::Vec3b>(y, x) = cv::Vec3b(
                pixel & 0xff,
                (pixel >> 8) & 0xff,
                (pixel >> 16) & 0xff
            );
        }
    }
    
    XDestroyImage(image);
    XCloseDisplay(display);
    
    return mat;
    
#elif __APPLE__
    // macOS screen capture
    CGDirectDisplayID displayID = CGMainDisplayID();
    CGImageRef cgImage = CGDisplayCreateImage(displayID);
    
    if (!cgImage) return cv::Mat();
    
    int width = CGImageGetWidth(cgImage);
    int height = CGImageGetHeight(cgImage);
    
    cv::Mat mat(height, width, CV_8UC4);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(
        mat.data, width, height, 8, mat.step[0],
        colorSpace, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little
    );
    
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    CGImageRelease(cgImage);
    
    cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
    
    return mat;
    
#else
    // Fallback: return placeholder
    return cv::Mat(m_Config.capture_height, m_Config.capture_width, CV_8UC3, cv::Scalar(40, 40, 40));
#endif
}

std::string VisionSystem::GetWindowTitleImpl(int monitor_index) {
#ifdef _WIN32
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        char title[256];
        GetWindowText(hwnd, title, sizeof(title));
        return std::string(title);
    }
#elif __linux__
    // Would need X11 window manager integration
#elif __APPLE__
    // Would need Cocoa integration
#endif
    return "Unknown Window";
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ELEMENT DETECTION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

std::vector<VisualElement> VisionSystem::DetectElements(const cv::Mat& frame) {
    std::vector<VisualElement> all_elements;
    
    auto buttons = DetectButtons(frame);
    auto text_fields = DetectTextFields(frame);
    auto menus = DetectMenus(frame);
    auto panels = DetectPanels(frame);
    
    all_elements.insert(all_elements.end(), buttons.begin(), buttons.end());
    all_elements.insert(all_elements.end(), text_fields.begin(), text_fields.end());
    all_elements.insert(all_elements.end(), menus.begin(), menus.end());
    all_elements.insert(all_elements.end(), panels.begin(), panels.end());
    
    return all_elements;
}

std::vector<VisualElement> VisionSystem::DetectButtons(const cv::Mat& frame) {
    std::vector<VisualElement> buttons;
    
    // Convert to grayscale for processing
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    // Detect rectangular shapes with rounded corners appearance
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    
    // Edge detection
    cv::Mat edges;
    cv::Canny(blurred, edges, 50, 150);
    
    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    int button_id = 0;
    for (const auto& contour : contours) {
        cv::RotatedRect rect = cv::minAreaRect(contour);
        cv::Size2f size = rect.size;
        
        // Filter by size (buttons are typically small-medium)
        if (size.width > 30 && size.width < 500 && 
            size.height > 15 && size.height < 100 &&
            size.width > size.height * 0.5 && size.width < size.height * 10) {
            
            VisualElement button;
            button.id = "button_" + std::to_string(button_id++);
            button.type = ElementType::BUTTON;
            
            // Get bounding box
            cv::Rect bbox = rect.boundingRect();
            button.bounding_box = bbox;
            
            // Extract text if OCR is enabled
            if (m_Config.enable_ocr && 
                bbox.x >= 0 && bbox.y >= 0 &&
                bbox.x + bbox.width < frame.cols &&
                bbox.y + bbox.height < frame.rows) {
                button.text_content = ExtractTextFromRegion(frame, bbox);
            }
            
            button.confidence = 0.7f;
            button.possible_actions = {"click", "hover", "focus"};
            
            buttons.push_back(button);
        }
    }
    
    return buttons;
}

std::vector<VisualElement> VisionSystem::DetectTextFields(const cv::Mat& frame) {
    std::vector<VisualElement> text_fields;
    
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    // Detect rectangular text input areas (usually have borders)
    cv::Mat thresh;
    cv::threshold(gray, thresh, 200, 255, cv::THRESH_BINARY_INV);
    
    cv::Mat horizontal_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 1));
    cv::Mat vertical_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 5));
    
    cv::Mat horizontal_lines, vertical_lines;
    cv::morphologyEx(thresh, horizontal_lines, cv::MORPH_OPEN, horizontal_kernel);
    cv::morphologyEx(thresh, vertical_lines, cv::MORPH_OPEN, vertical_kernel);
    
    int field_id = 0;
    
    // Simple detection based on white rectangular regions
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        cv::Rect bbox = cv::boundingRect(contour);
        
        // Filter for text field shapes (wider than tall, minimum size)
        if (bbox.width > 50 && bbox.height > 15 && 
            bbox.width > bbox.height * 2 && bbox.width < 1000) {
            
            VisualElement text_field;
            text_field.id = "textfield_" + std::to_string(field_id++);
            text_field.type = ElementType::TEXT_FIELD;
            text_field.bounding_box = bbox;
            text_field.confidence = 0.6f;
            text_field.possible_actions = {"type", "click", "focus", "select_all"};
            
            text_fields.push_back(text_field);
        }
    }
    
    return text_fields;
}

std::vector<VisualElement> VisionSystem::DetectMenus(const cv::Mat& frame) {
    std::vector<VisualElement> menus;
    
    // Menu detection based on horizontal arrangement of buttons
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(3, 3), 0);
    
    // Detect edges
    cv::Mat edges;
    cv::Canny(blur, edges, 30, 90);
    
    // Detect horizontal lines (typical of menu bars)
    cv::Mat horizontal_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(20, 1));
    cv::Mat horizontal_lines;
    cv::morphologyEx(edges, horizontal_lines, cv::MORPH_OPEN, horizontal_kernel);
    
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(horizontal_lines, lines, 1, CV_PI/180, 50, 50, 10);
    
    int menu_id = 0;
    for (const auto& line : lines) {
        int length = abs(line[2] - line[0]);
        if (length > 200) { // Long horizontal lines are likely menu bars
            VisualElement menu;
            menu.id = "menu_" + std::to_string(menu_id++);
            menu.type = ElementType::MENU;
            
            int min_x = std::min(line[0], line[2]);
            int min_y = std::min(line[1], line[3]);
            menu.bounding_box = cv::Rect(min_x, min_y - 20, length, 25);
            
            menu.confidence = 0.5f;
            menu.possible_actions = {"select"};
            
            menus.push_back(menu);
        }
    }
    
    return menus;
}

std::vector<VisualElement> VisionSystem::DetectPanels(const cv::Mat& frame) {
    std::vector<VisualElement> panels;
    
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    // Detect rectangular regions with borders
    cv::Mat thresh;
    cv::threshold(gray, thresh, 200, 255, cv::THRESH_BINARY_INV);
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    int panel_id = 0;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        
        // Filter for large rectangular regions (panels)
        if (area > 10000 && area < frame.cols * frame.rows * 0.8) {
            cv::Rect bbox = cv::boundingRect(contour);
            
            // Check if it's roughly rectangular
            float aspect_ratio = (float)bbox.width / bbox.height;
            if (aspect_ratio > 0.5 && aspect_ratio < 5.0) {
                VisualElement panel;
                panel.id = "panel_" + std::to_string(panel_id++);
                panel.type = ElementType::PANEL;
                panel.bounding_box = bbox;
                panel.confidence = 0.6f;
                panel.possible_actions = {"resize", "move"};
                
                panels.push_back(panel);
            }
        }
    }
    
    return panels;
}

std::vector<VisualElement> VisionSystem::DetectClickableElements(const cv::Mat& frame) {
    auto all_elements = DetectElements(frame);
    
    std::vector<VisualElement> clickable;
    for (const auto& elem : all_elements) {
        if (std::find(elem.possible_actions.begin(), elem.possible_actions.end(), "click") 
            != elem.possible_actions.end()) {
            clickable.push_back(elem);
        }
    }
    
    return clickable;
}

VisualElement* VisionSystem::FindElementByLabel(const std::string& label, const cv::Mat& frame) {
    auto elements = DetectElements(frame);
    
    for (auto& elem : elements) {
        std::string elem_label = elem.label.empty() ? elem.text_content : elem.label;
        if (elem_label.find(label) != std::string::npos) {
            return &elem;
        }
    }
    
    return nullptr;
}

VisualElement* VisionSystem::FindElementAtPosition(const cv::Point& position, const cv::Mat& frame) {
    auto elements = DetectElements(frame);
    
    for (auto& elem : elements) {
        if (elem.bounding_box.contains(position)) {
            return &elem;
        }
    }
    
    return nullptr;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// IMAGE ANALYSIS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

json VisionSystem::AnalyzeScreenContent(const cv::Mat& frame) {
    json analysis;
    
    // Color analysis
    cv::Scalar mean = cv::mean(frame);
    analysis["dominant_color"] = {mean[0], mean[1], mean[2]};
    
    // Brightness
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    analysis["average_brightness"] = cv::mean(gray)[0] / 255.0;
    
    // Element count
    auto elements = DetectElements(frame);
    analysis["element_count"] = elements.size();
    
    // Element breakdown
    std::map<ElementType, int> type_counts;
    for (const auto& elem : elements) {
        type_counts[elem.type]++;
    }
    
    json breakdown = json::object();
    for (const auto& [type, count] : type_counts) {
        breakdown[std::to_string((int)type)] = count;
    }
    analysis["element_breakdown"] = breakdown;
    
    // Complexity score (based on number of elements and edges)
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);
    double edge_density = cv::countNonZero(edges) / (double)(frame.cols * frame.rows);
    analysis["complexity_score"] = edge_density * 10;
    
    return analysis;
}

std::vector<std::string> VisionSystem::DetectUIFramework(const cv::Mat& frame) {
    std::vector<std::string> detected_frameworks;
    
    // This is a simplified framework detection based on visual patterns
    // In production, this would use ML-based detection
    
    // Check for common patterns
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    // Check for flat design (few edges, solid colors)
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);
    double edge_density = cv::countNonZero(edges) / (double)(frame.cols * frame.rows);
    
    if (edge_density < 0.05) {
        detected_frameworks.push_back("modern_flat_design");
    }
    
    // Check for web-like patterns (consistent spacing)
    detected_frameworks.push_back("generic_ui");
    
    return detected_frameworks;
}

std::string VisionSystem::DescribeScreen(const cv::Mat& frame) {
    json analysis = AnalyzeScreenContent(frame);
    
    std::ostringstream description;
    
    // Generate natural language description
    int button_count = analysis["element_breakdown"].contains("1") ? 
                       analysis["element_breakdown"]["1"].get<int>() : 0;
    int textfield_count = analysis["element_breakdown"].contains("2") ? 
                          analysis["element_breakdown"]["2"].get<int>() : 0;
    int panel_count = analysis["element_breakdown"].contains("11") ? 
                      analysis["element_breakdown"]["11"].get<int>() : 0;
    
    description << "Screen with " << analysis["element_count"].get<int>() << " detected elements. ";
    
    if (button_count > 0) {
        description << "Contains " << button_count << " buttons. ";
    }
    if (textfield_count > 0) {
        description << "Has " << textfield_count << " text input fields. ";
    }
    if (panel_count > 0) {
        description << "Features " << panel_count << " content panels. ";
    }
    
    description << "Overall complexity: " << 
                (analysis["complexity_score"].get<float>() < 0.3 ? "simple" : 
                 analysis["complexity_score"].get<float>() < 0.6 ? "moderate" : "complex");
    
    return description.str();
}

std::vector<ImageRegion> VisionSystem::SegmentScreen(const cv::Mat& frame) {
    std::vector<ImageRegion> regions;
    
    // Simple grid-based segmentation
    int grid_cols = 3;
    int grid_rows = 3;
    
    int region_width = frame.cols / grid_cols;
    int region_height = frame.rows / grid_rows;
    
    for (int row = 0; row < grid_rows; row++) {
        for (int col = 0; col < grid_cols; col++) {
            ImageRegion region;
            region.region = cv::Rect(
                col * region_width,
                row * region_height,
                region_width,
                region_height
            );
            
            region.description = "Screen region " + std::to_string(row) + "," + std::to_string(col);
            region.importance_score = 0.5f; // Would be determined by content analysis
            
            regions.push_back(region);
        }
    }
    
    // Also segment based on detected elements
    auto elements = DetectElements(frame);
    for (const auto& elem : elements) {
        ImageRegion region;
        region.region = elem.bounding_box;
        region.description = "Element: " + elem.text_content;
        region.importance_score = elem.confidence;
        regions.push_back(region);
    }
    
    return regions;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// OCR (Simplified - would integrate with Tesseract in production)
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

std::string VisionSystem::ExtractText(const cv::Mat& frame) {
    return ExtractTextFromRegion(frame, cv::Rect(0, 0, frame.cols, frame.rows));
}

std::string VisionSystem::ExtractTextFromRegion(const cv::Mat& frame, const cv::Rect& region) {
    // Placeholder for OCR integration
    // In production, this would use Tesseract or cloud-based OCR
    return "[OCR text would appear here]";
}

std::map<std::string, cv::Rect> VisionSystem::ExtractTextWithPositions(const cv::Mat& frame) {
    std::map<std::string, cv::Rect> text_positions;
    // Placeholder - would integrate OCR with position tracking
    return text_positions;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// COMPARISON
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

float VisionSystem::CompareFrames(const cv::Mat& frame1, const cv::Mat& frame2) {
    if (frame1.size() != frame2.size()) {
        return 0.0f;
    }
    
    cv::Mat gray1, gray2;
    cv::cvtColor(frame1, gray1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(frame2, gray2, cv::COLOR_BGR2GRAY);
    
    cv::Mat diff;
    cv::absdiff(gray1, gray2, diff);
    
    double diff_sum = cv::sum(diff)[0];
    double max_diff = 255.0 * frame1.cols * frame1.rows;
    
    return 1.0f - (diff_sum / max_diff);
}

std::vector<cv::Rect> VisionSystem::FindChangedRegions(const cv::Mat& before, const cv::Mat& after) {
    std::vector<cv::Rect> changes;
    
    if (before.size() != after.size()) return changes;
    
    cv::Mat gray1, gray2, diff;
    cv::cvtColor(before, gray1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(after, gray2, cv::COLOR_BGR2GRAY);
    
    cv::absdiff(gray1, gray2, diff);
    cv::threshold(diff, diff, 30, 255, cv::THRESH_BINARY);
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(diff, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        cv::Rect bbox = cv::boundingRect(contour);
        if (bbox.area() > 100) { // Filter small changes
            changes.push_back(bbox);
        }
    }
    
    return changes;
}

bool VisionSystem::IsScreenStatic(const cv::Mat& frame, float threshold) {
    static cv::Mat last_frame;
    static bool first_call = true;
    
    if (first_call) {
        last_frame = frame.clone();
        first_call = false;
        return true;
    }
    
    float similarity = CompareFrames(last_frame, frame);
    last_frame = frame.clone();
    
    return similarity >= threshold;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// RECORDING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void VisionSystem::StartRecording(const std::string& output_path) {
    if (m_IsRecording) return;
    
    m_RecordingPath = output_path;
    
    // Create video writer
    cv::Size frame_size(m_Config.capture_width, m_Config.capture_height);
    m_VideoWriter = new cv::VideoWriter(
        output_path,
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        m_Config.fps,
        frame_size
    );
    
    m_IsRecording = m_VideoWriter->isOpened();
    
    if (m_IsRecording) {
        std::cout << "[VisionSystem] Started recording to: " << output_path << std::endl;
    }
}

void VisionSystem::StopRecording() {
    if (!m_IsRecording) return;
    
    if (m_VideoWriter) {
        m_VideoWriter->release();
        delete m_VideoWriter;
        m_VideoWriter = nullptr;
    }
    
    m_IsRecording = false;
    std::cout << "[VisionSystem] Stopped recording" << std::endl;
}

void VisionSystem::AddFrameToRecording(const cv::Mat& frame) {
    if (!m_IsRecording || !m_VideoWriter) return;
    
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(m_Config.capture_width, m_Config.capture_height));
    m_VideoWriter->write(resized);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CACHE MANAGEMENT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void VisionSystem::CacheCapture(const ScreenCapture& capture) {
    m_CaptureCache.push_back(capture);
    
    // Limit cache size
    while (m_CaptureCache.size() > (size_t)m_Config.max_cache_size) {
        m_CaptureCache.erase(m_CaptureCache.begin());
    }
}

ScreenCapture VisionSystem::GetCachedCapture(const std::string& id) {
    for (const auto& capture : m_CaptureCache) {
        if (capture.id == id) {
            return capture;
        }
    }
    return ScreenCapture{};
}

void VisionSystem::ClearCache() {
    m_CaptureCache.clear();
}

std::vector<ScreenCapture> VisionSystem::GetRecentCaptures(int count) {
    std::vector<ScreenCapture> recent;
    
    int start = std::max(0, (int)m_CaptureCache.size() - count);
    for (int i = start; i < (int)m_CaptureCache.size(); i++) {
        recent.push_back(m_CaptureCache[i]);
    }
    
    return recent;
}

void VisionSystem::SetConfig(const CaptureConfig& config) {
    m_Config = config;
}

cv::Mat VisionSystem::PreprocessForDetection(const cv::Mat& frame) {
    cv::Mat processed;
    
    // Resize if too large
    if (frame.cols > 1920 || frame.rows > 1080) {
        float scale = std::min(1920.0f / frame.cols, 1080.0f / frame.rows);
        cv::resize(frame, processed, cv::Size(), scale, scale);
    } else {
        processed = frame.clone();
    }
    
    return processed;
}

std::vector<cv::Rect> VisionSystem::FindContours(const cv::Mat& frame) {
    cv::Mat gray, thresh;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, thresh, 200, 255, cv::THRESH_BINARY_INV);
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    std::vector<cv::Rect> rects;
    for (const auto& contour : contours) {
        rects.push_back(cv::boundingRect(contour));
    }
    
    return rects;
}

cv::Mat VisionSystem::GenerateThumbnail(const cv::Mat& frame, int max_size) {
    cv::Mat thumbnail;
    
    float scale = std::min((float)max_size / frame.cols, (float)max_size / frame.rows);
    cv::resize(frame, thumbnail, cv::Size(), scale, scale);
    
    return thumbnail;
}

} // namespace SZM::AI::Vision
