#pragma once
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <string>
class Video{
public:
    explicit Video(const std::string& path);
    bool read(cv::Mat& frame);
    double fps() const {return fps_;};
private:
    cv::VideoCapture capture_;
    double fps_;
};
