// Detects black pixels
// author: Neelay Junnarkar

#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;


Mat mask(Mat &in, Scalar min, Scalar max) {
	Mat mask(in.size().height, in.size().width, CV_8UC1);
	inRange(in, Scalar(100, 30, 12, 0), Scalar(105, 80, 75, 256), mask);
	Mat out;
	in.copyTo(out, mask);
	return out;
}


int main() {
	string screen = "ScreenCapture.png";

	Mat img = imread(screen, CV_LOAD_IMAGE_COLOR);
	if(!img.data) {
		std::cerr << "couldn't find image" << std::endl;
		return 1;
	}

	int dx = -90;
	int dy = -20;
	Rect roi{Point{918+dx, 180+dy}, Point{1328+dx, 774+dy}};
	img = img(roi);

	Mat hsv;
	cvtColor(img, hsv, CV_BGR2HSV);

	imshow("Original", img);
	imshow("HSV", hsv);

	Mat out = ::mask(hsv, Scalar(100, 30, 12, 0), Scalar(105, 80, 75, 256));
	imshow("Masked", out);

	for(int y = 0; y < out.rows; ++y) {
		Vec3b* row = out.ptr<Vec3b>(y);
		for(int x = 0; x < out.cols; ++x) {
			Vec3b& pixel = row[x];
			if(pixel.val[2] != 0) {
				pixel.val[2] = 255;
				std::cout << x+918+dx << ", " << y+180+dy << std::endl;
			}
		}
	}
	imshow("Masked", out);

	waitKey(0);
	return 1;
}