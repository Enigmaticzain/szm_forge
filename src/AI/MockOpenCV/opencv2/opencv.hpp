#pragma once

#include <string>
#include <vector>
#include <cstdint>

#define CV_8UC3 16
#define CV_8UC4 24
#define CV_PI 3.14159265358979323846

namespace cv {

struct Scalar {
    double val[4];
    Scalar() { val[0]=val[1]=val[2]=val[3]=0; }
    Scalar(double v0, double v1, double v2, double v3=0) { val[0]=v0; val[1]=v1; val[2]=v2; val[3]=v3; }
    double& operator[](int i) { return val[i]; }
    const double& operator[](int i) const { return val[i]; }
};

struct Vec3b {
    uint8_t val[3];
    Vec3b() { val[0]=val[1]=val[2]=0; }
    Vec3b(uint8_t v0, uint8_t v1, uint8_t v2) { val[0]=v0; val[1]=v1; val[2]=v2; }
    uint8_t& operator[](int i) { return val[i]; }
    const uint8_t& operator[](int i) const { return val[i]; }
};

struct Vec4i {
    int val[4];
    Vec4i() { val[0]=val[1]=val[2]=val[3]=0; }
    Vec4i(int v0, int v1, int v2, int v3) { val[0]=v0; val[1]=v1; val[2]=v2; val[3]=v3; }
    int& operator[](int i) { return val[i]; }
    const int& operator[](int i) const { return val[i]; }
};

struct Size {
    int width, height;
    Size() : width(0), height(0) {}
    Size(int w, int h) : width(w), height(h) {}
    bool operator!=(const Size& other) const { return width != other.width || height != other.height; }
};

struct Size2f {
    float width, height;
    Size2f() : width(0), height(0) {}
    Size2f(float w, float h) : width(w), height(h) {}
};

class Mat {
public:
    Mat() {}
    Mat(int r, int c, int type) : rows(r), cols(c) {}
    Mat(int r, int c, int type, const Scalar& s) : rows(r), cols(c) {}
    int rows = 0;
    int cols = 0;
    bool empty() const { return true; }
    void copyTo(Mat& m) const {}
    int type() const { return 0; }
    Mat clone() const { return *this; }
    Size size() const { return Size(cols, rows); }
    
    template<typename T>
    T& at(int y, int x) { static T dummy; return dummy; }
    
    Mat operator()(const class Rect& roi) const { return *this; }
};

class Point {
public:
    Point() {}
    Point(int _x, int _y) : x(_x), y(_y) {}
    int x = 0;
    int y = 0;
    Point operator-(const Point& pt) const { return Point(x - pt.x, y - pt.y); }
};

class Rect {
public:
    Rect() {}
    Rect(int _x, int _y, int _w, int _h) : x(_x), y(_y), width(_w), height(_h) {}
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    bool contains(const Point& pt) const {
        return pt.x >= x && pt.x < x + width && pt.y >= y && pt.y < y + height;
    }
    int area() const { return width * height; }
    Rect operator-(const Point& pt) const { return Rect(x - pt.x, y - pt.y, width, height); }
};

class RotatedRect {
public:
    RotatedRect() {}
    Size2f size;
    Rect boundingRect() const { return Rect(0,0,(int)size.width,(int)size.height); }
};

class VideoWriter {
public:
    VideoWriter() {}
    VideoWriter(const std::string&, int, double, Size, bool isColor=true) {}
    bool open(const std::string& filename, int fourcc, double fps, int width, int height, bool isColor=true) { return false; }
    bool isOpened() const { return false; }
    void write(const Mat& image) {}
    void release() {}
    static int fourcc(char c1, char c2, char c3, char c4) { return 0; }
};

enum {
    COLOR_BGR2GRAY,
    COLOR_BGR2RGB,
    COLOR_BGRA2BGR,
    THRESH_BINARY_INV,
    THRESH_BINARY,
    RETR_EXTERNAL,
    CHAIN_APPROX_SIMPLE,
    IMREAD_COLOR,
    MORPH_RECT,
    MORPH_OPEN,
    MORPH_CLOSE
};

inline void cvtColor(const Mat& src, Mat& dst, int code) {}
inline void threshold(const Mat& src, Mat& dst, double thresh, double maxval, int type) {}
inline void findContours(const Mat& image, std::vector<std::vector<Point>>& contours, int mode, int method) {}
inline Rect boundingRect(const std::vector<Point>& points) { return Rect(0,0,0,0); }
inline void resize(const Mat& src, Mat& dst, Size dsize, double fx=0, double fy=0) {}

inline Scalar mean(const Mat& src) { return Scalar(0,0,0); }
inline void Canny(const Mat& image, Mat& edges, double threshold1, double threshold2) {}
inline int countNonZero(const Mat& src) { return 0; }
inline void absdiff(const Mat& src1, const Mat& src2, Mat& dst) {}
inline Scalar sum(const Mat& src) { return Scalar(0,0,0); }
inline double contourArea(const std::vector<Point>& contour) { return 0.0; }
inline bool imwrite(const std::string& filename, const Mat& img, const std::vector<int>& params = std::vector<int>()) { return false; }
inline Mat imread(const std::string& filename, int flags = IMREAD_COLOR) { return Mat(); }

inline RotatedRect minAreaRect(const std::vector<Point>& points) { return RotatedRect(); }
inline void GaussianBlur(const Mat& src, Mat& dst, Size ksize, double sigmaX) {}
inline Mat getStructuringElement(int shape, Size ksize) { return Mat(); }
inline void morphologyEx(const Mat& src, Mat& dst, int op, const Mat& kernel) {}
inline void HoughLinesP(const Mat& image, std::vector<Vec4i>& lines, double rho, double theta, int threshold, double minLineLength=0, double maxLineGap=0) {}

} // namespace cv
