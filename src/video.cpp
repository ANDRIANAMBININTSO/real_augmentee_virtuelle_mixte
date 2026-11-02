#include "video.hpp"
#include <stdexcept>

Video::Video(const std::string& path) : capture_(path) {
  if (!capture_.isOpened())
    throw std::runtime_error("Impossible d'ouvrir la vidéo : " + path);
  fps_ = capture_.get(cv::CAP_PROP_FPS);
}
Video::Video(int num): capture_(num) {
  if (!capture_.isOpened())
    throw std::runtime_error("Impossible d'ouvrir la caméra");
  fps_ = capture_.get(cv::CAP_PROP_FPS);
}

bool Video::read(cv::Mat& frame, bool loop) {
  if (capture_.read(frame) && !frame.empty()) 
    return true;
  // si la video est fini et que loop est false, on coupe
  if (!loop) 
    return false;
  // sinon, on recommence
  capture_.set(cv::CAP_PROP_POS_FRAMES, 0);
  return capture_.read(frame) && !frame.empty();



}