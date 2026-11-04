#ifndef DETECTION_HPP
#define DETECTION_HPP
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

namespace detection {
    vector<Point2f> sortCorners(vector<Point2f>& pts, vector<Point2f>& prevPts);
    vector<Point2f> findCorners(const vector<Point>& contour, vector<Point2f>& prevPts);
    Mat segmentSheet(const Mat& frame);
    bool findLargestContour(const Mat& binary, vector<Point>& largest);
    void drawSheet(Mat& frame, const vector<Point2f>& corners);
}
#endif