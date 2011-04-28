#include "stdafx.h"

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

void help()
{
	printf("\nThis program demonstrates using features2d detector, descriptor extractor and simple matcher\n"
			"Using the SURF desriptor:\n"
			"\n"
			"Usage:\n matcher_simple <image1> <image2>\n");
}
/*
int _tmain(int argc, char** argv)
{
	//if(argc != 3)
	//{
	//	help();
	//	return -1;
	//}

	//Mat img1 = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	//Mat img2 = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);
	lImage *img1 = cvLoadImage("imgs/query.png", CV_LOAD_IMAGE_GRAYSCALE);
	lImage *img2 = cvLoadImage("imgs/train/1.png", CV_LOAD_IMAGE_GRAYSCALE);
	if(img1.empty() || img2.empty())
	{
		printf("Can't read one of the images\n");
		return -1;
	}

	
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *img1Keypoints = 0, *img1Descriptors = 0;
    CvSeq *img2Keypoints = 0, *img2Descriptors = 0;
    CvSURFParams params = cvSURFParams(500, 1);
    int i;
	
	double tt = (double) cvGetTickCount();
	cvExtractSURF(img1, 0, &img1Keypoints, &img1Descriptors, storage, params);
	printf("img1 descriptors: %d\n", img1Descriptors->total);

	cvExtractSURF(img2, 0, &img2Keypoints, &img2Descriptors, storage, params);
	printf("img2 descriptors: %d\n", img2Descriptors->total);
	tt = (double) cvGetTickCount() - tt;
	printf("Extraction time: %gms\n", tt/(cvGetTickFrequency()*1000.));

cout << "HERE1";
	// detecting keypoints
	SurfFeatureDetector detector(1);
cout << "HERE1almost";
	vector<KeyPoint> keypoints1, keypoints2;
	detector.detect(img1, keypoints1);
cout << "HERE1a";
	detector.detect(img2, keypoints2);
cout << "HERE2";
	// computing descriptors
	SurfDescriptorExtractor extractor;
	Mat descriptors1, descriptors2;
	extractor.compute(img1, keypoints1, descriptors1);
	extractor.compute(img2, keypoints2, descriptors2);
cout << "HERE3";
	// matching descriptors
	BruteForceMatcher<L2<float> > matcher;
    vector<DMatch> matches;
    matcher.match(descriptors1, descriptors2, matches);
cout << "HERE4";
	// drawing the results
	namedWindow("matches", 1);
	Mat img_matches;
	drawMatches(img1, keypoints1, img2, keypoints2, matches, img_matches);
	imshow("matches", img_matches);
	waitKey(0);
cout << "HERE5";
	return 0;
}
*/