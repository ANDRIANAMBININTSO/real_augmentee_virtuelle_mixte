#ifndef VIDEO_HPP
#define VIDEO_HPP
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <string>
class Video{
public:
    explicit Video(const std::string& path);
    explicit Video(int num);
    bool read(cv::Mat& frame, bool loop = false);
    double fps() const {return fps_;};
    bool capture(cv::Mat& frame);
private:
    cv::VideoCapture capture_;
    double fps_;
};
#endif
