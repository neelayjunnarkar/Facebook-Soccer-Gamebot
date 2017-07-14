// Bot that uses OpenCV to play the Facebook Messenger Soccer game
// author: Neelay Junnarkar
#include <Windows.h>
#include <iostream>
#include <thread>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include <array>

using namespace cv;
using namespace std;

//#define DISP_CURSOR_COORDS

void hwnd2mat(const HWND &hwnd, const unsigned shrink, Mat &out, const Rect &windowsize) {

	HDC hwindowDC, hwindowCompatibleDC;

	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	const int srcheight = windowsize.height;
	const int srcwidth = windowsize.width;
	const int srcx = windowsize.x;
	const int srcy = windowsize.y;
	const int height = windowsize.height / shrink;
	const int width = windowsize.width / shrink;

	out.create(height, width, CV_8UC4);

	// create a bitmap
	const HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, srcx, srcy, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, out.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow
																									   
	DeleteObject(hbwindow); DeleteDC(hwindowCompatibleDC); ReleaseDC(hwnd, hwindowDC);
}

void left_click(POINT &pos) {
	SetCursorPos(pos.x, pos.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

bool out_of_bounds(const POINT &pos, const Rect &bounds) {
	return (pos.x < bounds.x || pos.y < bounds.y || pos.x > bounds.x + bounds.width || pos.y > bounds.y + bounds.height);
}

//returns false on read error, true on success
bool read_gray_img(const unsigned shrink, Mat &out, const Rect &roi) {

	hwnd2mat(GetDesktopWindow(), shrink, out, roi);

	if(out.empty() || !out.data)
		return false;

	cvtColor(out, out, COLOR_RGB2GRAY);
	return true;
}

//Use HoughCircles to find circles in the img
void find_circles(const Mat &gray_img, vector<Vec3f> &out_circles, const int min_radius, const int max_radius) {
	HoughCircles(gray_img, out_circles, HOUGH_GRADIENT, 1, 10, 100, 30, min_radius, max_radius);
}

//draw the passed circles and their centers onto the imge
void draw_circles(const vector<Vec3f> &circles, Mat &img) {
	for(size_t i = 0; i < circles.size(); ++i) {
		const Vec3i &c = circles[i];
		circle(img, Point(c[0], c[1]), c[2], 0, 3, LINE_AA); //circle
		circle(img, Point(c[0], c[1]), 2, 0, 3, LINE_AA); //center
	}
}

double min(const double &a, const double &b) {
	return a < b ? a : b;
}
double max(const double &a, const double &b) {
	return a > b ? a : b;
}

int main() {

	using namespace std::chrono;
	const int shrink = 2;
	const double drop_factor = 2.5;
	const int min_radius = (int)floor(40.0 / shrink); // 40
	const int max_radius = (int)ceil(70.0 / shrink); //70

	const std::vector<int> dy_s = {0};

	Rect rect_fitted; //roi
	rect_fitted.x = 725;
	rect_fitted.y = 205;
	rect_fitted.width = 1125-725;
	rect_fitted.height = 845-205;

	Rect click_area{rect_fitted};
	const int shift = 0;
	click_area.y += shift;
	click_area.height -= shift;

	const double accel = 50 * pow(10, -15);
	//const double accel = 100 * pow(10, -15);
	_int64 v0 = -1;

	Mat img;
	vector<Vec3f> circles;
	POINT mouse;

	int last_y = -1;
	bool last_dropping = false;

	std::this_thread::sleep_for(std::chrono::seconds(5));

	cout << "Beginning" << endl;

	//const high_resolution_clock::time_point t0 = high_resolution_clock::now();

	while(true) {
#if defined(DISP_CURSOR_COORDS)
		GetCursorPos(&mouse);
		std::cout << mouse.x << " " << mouse.y << "\n";
#endif
		const high_resolution_clock::time_point t1 = high_resolution_clock::now();
		read_gray_img(shrink, img, rect_fitted);
		find_circles(img, circles, min_radius, max_radius);

		if(circles.size() < 1)
			continue;

		for(const Vec3f &pt : circles) {
			mouse.x = pt[0] * shrink + rect_fitted.x;
			mouse.y = pt[1] * shrink + rect_fitted.y;
			const int y_temp = mouse.y;

			if(last_y != -1 && mouse.y > last_y) {
				//left_click(POINT{mouse.x, (LONG)min(mouse.y + 100, click_area.y + click_area.height)});
				const high_resolution_clock::time_point t2 = high_resolution_clock::now();
				const _int64 dt = duration_cast<nanoseconds>(t2 - t1).count();
				if(last_dropping) { //if it has been dropping for 2+ frames, calculate new v0, otherwise use v0 from some old state
					v0 = (mouse.y - last_y) / dt;
				}
				const double dy = max(v0*dt + 0.5*accel * pow(dt, 2), 100);
				mouse.y += dy;
				mouse.y = min(mouse.y, click_area.y + click_area.height);
				last_dropping = true;

			} else if (mouse.y < last_y) {
				last_dropping = false;
			}
			for(int dy : dy_s) {
				POINT temp;
				temp.x = mouse.x;
				temp.y = mouse.y + dy;

				if(!out_of_bounds(temp, click_area)) {
					left_click(temp);
					last_y = y_temp;
				}
			}
		}
	}

	waitKey(0);
}
