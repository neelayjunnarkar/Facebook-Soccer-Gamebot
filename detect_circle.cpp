// Code to detect circles
// Used to implemet fb_soccer_bot
// Author: Neelay Junnarkar

#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

//returns false on read error, true on success
bool read_gray_img(const string &file, Mat &out, const Rect &roi) {
	out = imread(file, 1);
	if(out.empty() || !out.data)
		return false;
	out = out(roi);
	return true;
}

//Use HoughCircles to find circles in the img
void find_circles(const Mat &gray_img, vector<Vec3f> &out_circles, int min_radius, int max_radius) {
	HoughCircles(gray_img, out_circles, HOUGH_GRADIENT, 1, 10, 100, 30, min_radius, max_radius);
}

//draw the passed circles and their centers onto the imge
void draw_circles(const vector<Vec3f> &circles, Mat &out_img) {
	for(size_t i = 0; i < circles.size(); ++i) {
		Vec3i c = circles[i];
		circle(out_img, Point(c[0], c[1]), c[2], 0, 3, LINE_AA); //circle
		circle(out_img, Point(c[0], c[1]), 2, 0, 3, LINE_AA); //center
	}
}

int main() {
	string screen_fullsize = "ScreenCapture_SHRINK1.png";
	int shrink = 1;
	int dx = -90 / shrink;
	int dy = -20 / shrink;
	int left = floor(918.0 / shrink);
	int right = ceil(1328.0 / shrink);
	int ymin = floor(180.0 / shrink);
	int ymax = ceil(774.0 / shrink);
	int min_radius = floor(50.0 / shrink);
	int max_radius = ceil(55.0 / shrink);

	Rect roi{Point{left + dx, ymin + dy}, Point{right + dx, ymax + dy}};

	Mat img;
	if(!read_gray_img("ScreenCapture_SHRINK"+to_string(shrink)+".png", img, roi)) {
		std::cerr << "Could not load image" << std::endl;
		return 1;
	}

	imshow("Original", img);
	
	Mat hue_mask1, hue_mask2, hm3, hm4, hm5, hm6, hm7, hm8;
	inRange(img, Scalar(110, 110, 110), Scalar(115, 115, 115), hue_mask1);
	inRange(img, Scalar(35, 210, 250), Scalar(45, 215, 255), hue_mask2);
	inRange(img, Scalar(250, 70, 240), Scalar(255, 80, 250), hm3);
	inRange(img, Scalar(250, 230, 10), Scalar(255, 240, 20), hm4);
	inRange(img, Scalar(75, 215, 245), Scalar(95, 230, 255), hm5);
	inRange(img, Scalar(145, 230, 250), Scalar(150, 240, 255), hm6);
	inRange(img, Scalar(165, 230, 250), Scalar(175, 245, 255), hm8);
	inRange(img, Scalar(190, 240, 250), Scalar(200, 250, 255), hm7);

	Mat hue_mask = hue_mask1 | hue_mask2 | hm3 | hm4 | hm5 | hm6 | hm7 | hm8;

	Mat gray;
	cvtColor(img, gray, CV_BGR2GRAY);
	Mat out = gray | hue_mask;
	imshow("out", out);

	
	vector<Vec3f> circles;
	
	find_circles(img, circles, min_radius, max_radius);

	draw_circles(circles, img);

	imshow("Circles", img);
	
	waitKey(0);
}