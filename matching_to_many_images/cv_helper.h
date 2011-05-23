#ifndef CV_HELPER__
#define CV_HELPER__
#include <opencv2/core/core.hpp>

IplImage* resizeImage(const IplImage *origImg, int newWidth,
	int newHeight, bool keepAspectRatio);
IplImage* cropImage(const IplImage *img, const CvRect region);

#endif