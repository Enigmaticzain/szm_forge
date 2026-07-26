#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

namespace SZM::AI::Vision {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// VISUAL ELEMENT TYPES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class ElementType {
    BUTTON,
    TEXT_FIELD,
    CHECKBOX,
    RADIO_BUTTON,
    DROPDOWN,
    MENU,
    TAB,
    ICON,
    IMAGE,
    LINK,
    SLIDER,
    TOGGLE,
    WINDOW,
    PANEL,
    TABLE,
    LIST,
    UNKNOWN
};

struct VisualElement {
    std::string id;
    ElementType type;
    std::string label;
    std::string text_content;
    cv::Rect bounding_box;
    float confidence;
    std::vector<std::string> possible_actions;
    json metadata;
};

struct ScreenCapture {
    std::string id;
    cv::Mat image;
    cv::Mat thumbnail;
    uint64_t timestamp;
    std::vector<VisualElement> detected_elements;
    std::string window_title;
    std::string application_name;
    json context;
};

struct ImageRegion {
    cv::Rect region;
    std::string description;
    std::vector<std::string> detected_objects;
    float importance_score;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SCREEN CAPTURE CONFIGURATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

struct CaptureConfig {
    int capture_width = 1920;
    int capture_height = 1080;
    int fps = 5;
    int quality = 85;
    bool capture_cursor = true;
    bool capture_overlays = true;
    std::string output_format = "png";
    int max_cache_size = 100;
    bool enable_element_detection = true;
    bool enable_ocr = true;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// VISION SYSTEM
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class VisionSystem {
public:
    static VisionSystem& GetInstance();

    // Initialization
    void Initialize();
    void Shutdown();
    bool IsInitialized() const { return m_IsInitialized; }

    // Screen Capture
    ScreenCapture CaptureScreen(int monitor_index = 0);
    ScreenCapture CaptureRegion(const cv::Rect& region, int monitor_index = 0);
    ScreenCapture CaptureWindow(const std::string& window_title);
    std::vector<ScreenCapture> CaptureAllMonitors();

    // Element Detection
    std::vector<VisualElement> DetectElements(const cv::Mat& frame);
    std::vector<VisualElement> DetectClickableElements(const cv::Mat& frame);
    VisualElement* FindElementByLabel(const std::string& label, const cv::Mat& frame);
    VisualElement* FindElementAtPosition(const cv::Point& position, const cv::Mat& frame);

    // Image Analysis
    json AnalyzeScreenContent(const cv::Mat& frame);
    std::vector<std::string> DetectUIFramework(const cv::Mat& frame);
    std::string DescribeScreen(const cv::Mat& frame);
    std::vector<ImageRegion> SegmentScreen(const cv::Mat& frame);

    // OCR
    std::string ExtractText(const cv::Mat& frame);
    std::string ExtractTextFromRegion(const cv::Mat& frame, const cv::Rect& region);
    std::map<std::string, cv::Rect> ExtractTextWithPositions(const cv::Mat& frame);

    // Comparison
    float CompareFrames(const cv::Mat& frame1, const cv::Mat& frame2);
    std::vector<cv::Rect> FindChangedRegions(const cv::Mat& before, const cv::Mat& after);
    bool IsScreenStatic(const cv::Mat& frame, float threshold = 0.95f);

    // Recording
    void StartRecording(const std::string& output_path);
    void StopRecording();
    bool IsRecording() const { return m_IsRecording; }
    void AddFrameToRecording(const cv::Mat& frame);

    // Configuration
    void SetConfig(const CaptureConfig& config);
    CaptureConfig GetConfig() const { return m_Config; }

    // Cache Management
    ScreenCapture GetCachedCapture(const std::string& id);
    void CacheCapture(const ScreenCapture& capture);
    void ClearCache();
    std::vector<ScreenCapture> GetRecentCaptures(int count = 10);

private:
    VisionSystem() = default;
    ~VisionSystem();

    VisionSystem(const VisionSystem&) = delete;
    VisionSystem& operator=(const VisionSystem&) = delete;

    bool m_IsInitialized = false;
    bool m_IsRecording = false;
    CaptureConfig m_Config;
    
    cv::VideoWriter* m_VideoWriter = nullptr;
    std::string m_RecordingPath;
    std::vector<ScreenCapture> m_CaptureCache;
    
    // Platform-specific implementation
    cv::Mat CaptureScreenImpl(int monitor_index);
    std::string GetWindowTitleImpl(int monitor_index);

    // Element detection helpers
    std::vector<VisualElement> DetectButtons(const cv::Mat& frame);
    std::vector<VisualElement> DetectTextFields(const cv::Mat& frame);
    std::vector<VisualElement> DetectMenus(const cv::Mat& frame);
    std::vector<VisualElement> DetectPanels(const cv::Mat& frame);

    // Image processing helpers
    cv::Mat PreprocessForDetection(const cv::Mat& frame);
    std::vector<cv::Rect> FindContours(const cv::Mat& frame);
    cv::Mat GenerateThumbnail(const cv::Mat& frame, int max_size = 256);
};

} // namespace SZM::AI::Vision
