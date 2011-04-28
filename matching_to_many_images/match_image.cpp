// So we have one query image and several images in a library image. For each keypoint descriptor of query image
// the one nearest descriptor is found the entire collection of library images. As an extra, to aid in debugging,
// we can visualize the result of matching by saving resultant match images... each of which combines query and
// library image with matches between them (if they exist). The match is drawn as a line between corresponding
// points.Count of all matches is equal to count of query keypoints.
//
// TODO: use FAST
#include "stdafx.h"

#include <fstream>
#include <iostream>

#include <opencv/cv.h>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "util.h"
#include "Logger.h"
#include "SURFMatcher.h"

using namespace cv;
using namespace std;

// TODO: change the fact that query images must be smaller than training image
const string defaultQueryImageName = "data/boston_closeup.jpg";
const string defaultFileWithTrainImages = "data/training/trainingImages.txt";
const string defaultDirToSaveResImages = "data/results";


bool readImages(const string& queryImageName, IplImage **queryImage) {
    cout << "< Reading the images..." << endl;
    *queryImage = cvLoadImage(queryImageName.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
    if (!*queryImage) {
        cerr << "Query image can not be read." << endl << ">" << endl;
        return false;
    }

    cout << ">" << endl;

    return true;
}

int _tmain(int argc, char** argv) {
    string queryImageName = defaultQueryImageName;
    string fileWithTrainImages = defaultFileWithTrainImages;
    string dirToSaveResImages = defaultDirToSaveResImages;

	Logger logger;
    IplImage *queryImage;
    if (!readImages(queryImageName, &queryImage)) {
        return -1;
    }

	SURFMatcher surfMatcher(&logger);
	surfMatcher.Build(fileWithTrainImages);

	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSURFParams params = cvSURFParams(400, 1);

	CvSeq *queryKeyPoints = 0, *queryDescriptors = 0;
	double tt = (double) -cvGetTickCount();
	cvExtractSURF(queryImage, 0, &queryKeyPoints, &queryDescriptors, storage, params);
	printf("Query Descriptors %d\nQuery Extraction Time = %gm\n", queryDescriptors->total,
		((tt + cvGetTickCount()) / cvGetTickFrequency()*1000.));

	
	surfMatcher.MatchAgainstLibrary(queryImageName.c_str(), queryImage, queryKeyPoints, queryDescriptors);
	surfMatcher.Visualize(dirToSaveResImages, queryImageName, queryImage, queryKeyPoints, queryDescriptors);
	return 0;
}
