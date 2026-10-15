// video.cpp
#include "video.hpp"
#include <stdexcept>

Video::Video(const std::string& path) : capture_(path) {
  if (!capture_.isOpened())
    throw std::runtime_error("Impossible d'ouvrir la vidéo : " + path);
  fps_ = capture_.get(cv::CAP_PROP_FPS);
}

bool Video::read(cv::Mat& frame) {
  return capture_.read(frame) && !frame.empty();
}