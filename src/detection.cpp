#include "detection.hpp"
#include <iostream>

namespace detection {

    vector<Point2f> sortCorners(vector<Point2f>& pts, vector<Point2f>& prevPts) {
        // cas 1ere frame : on détermine l'ordre des coins dans le sens horaire
        if (prevPts.empty()) {
            vector<Point2f> sorted(4);
            double minSum = 1e18, maxSum = -1e18, minDiff = 1e18, maxDiff = -1e18;
            for (const Point2f& p : pts) {
                double sum = p.x + p.y;    // petit en haut à gauche, grand en bas à droite
                double diff = p.y - p.x;   // petit en haut à droite, grand en bas à gauche
                if (sum < minSum)   { minSum = sum;   sorted[0] = p; }  // haut-gauche
                if (diff < minDiff) { minDiff = diff; sorted[1] = p; }  // haut-droit
                if (sum > maxSum)   { maxSum = sum;   sorted[2] = p; }  // bas-droit
                if (diff > maxDiff) { maxDiff = diff; sorted[3] = p; }  // bas-gauche
            }
            prevPts = sorted;          
            return sorted;
        }
        // cas toutes les autres frames : on détermine l'ordre des coins via leur position dans la frame précedente
        else {
            // verifier qu'il y a bien eu sortCorners appelé une fois ? 
            vector<Point2f> newPts(4);
            for (const Point2f& p : pts){
                float diffMin = 1e18;
                int diffMinIdx;

                for (int i = 0; i < prevPts.size(); ++i){
                    const Point2f& pp = prevPts[i];
                    float diffX = p.x - pp.x;
                    float diffY = p.y - pp.y;
                    float diff = diffX*diffX + diffY*diffY;

                    if (diff < diffMin) {
                    diffMin = diff;
                    diffMinIdx = i;
                    }
                }
                newPts[diffMinIdx] = p;
            }
            prevPts = newPts;
            return newPts;
        }
    }

    
    vector<Point2f> findCorners(const vector<Point>& contour, vector<Point2f>& prevPts) {
    // on simplifie le contour en polygone : une feuille doit donner 4 sommets
        vector<Point> approx;
        approxPolyDP(contour, approx, 0.02 * arcLength(contour, true), true);
        
        vector<Point2f> pts;
        if (approx.size() == 4 && isContourConvex(approx)) {
            for (const Point& p : approx) {
                pts.push_back(Point2f(p));
            }
        } else {
            // contour bruité : on se rabat sur le rectangle orienté qui l'englobe (toujours 4 coins)
            Point2f box[4];
            minAreaRect(contour).points(box);
            pts.assign(box, box + 4);
        }
        return sortCorners(pts, prevPts);
    }

    
    Mat segmentSheet(const Mat& frame){
        Mat grey, blurred, binary;
        cvtColor(frame, grey, COLOR_BGR2GRAY);
        GaussianBlur(grey, blurred, Size(9,9), 0);
        //threshold(blurred, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
        // pour tricher
        threshold(blurred, binary, 100, 255, THRESH_BINARY);
        return binary;
    }

    bool findLargestContour(const Mat& binary, vector<Point>& largest){
        vector<vector<Point>> contours;
        findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        double maxArea = 0.0;
        for (const auto& c: contours){
            Rect r = boundingRect(c);
            bool touchesBorder = r.x <= 0.0 || r.y <= 0.0 || r.x + r.width >= binary.cols || r.y + r.height >= binary.rows;
            double area = contourArea(c);
            if(area > maxArea && !touchesBorder){
                maxArea = area;
                largest = c;
            }
        }
        return maxArea > 0.0;
    }
    void drawSheet(Mat& frame, const vector<Point2f>& corners){
        vector<Point> quad(corners.begin(), corners.end());
        polylines(frame, quad, true, Scalar(0,0,255), 2);
        for (int i=0; i<4; i++){
            circle(frame, corners[i], 6, Scalar(0,255,255), FILLED);
            putText(frame, to_string(i), corners[i] + Point2f(8,-8),FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,255,255),2);
        }
    }
}

