// TODO: use flann
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "SURFMatcher.h"
#include "Util.h"
#include "tinyxml/tinyxml.h"

SURFMatcher::SURFMatcher(Logger *logger) : logger_(logger) {
}

SURFMatcher::~SURFMatcher() {
	std::vector<ImageData *>::iterator it;
	for (it = referenceData_.begin(); it != referenceData_.end(); ++it) {
		cvReleaseImage(&((*it)->image));
		delete *it;
	}
}

// fileName contains list of images to build library with
int SURFMatcher::Build(std::string fileName) {
	logger_->Log(INFO, "Building SURF reference library with %s...\n", fileName);
	double tt = (double) -cvGetTickCount();

	// parse library file
	fileName = "data/library/index.xml";
	TiXmlDocument *doc = new TiXmlDocument(fileName.c_str());
	const char dlmtr = '/';
	size_t pos = fileName.rfind(dlmtr);
	string dirName = pos == string::npos ? "" : fileName.substr(0, pos) + dlmtr;

	doc->LoadFile();
	TiXmlNode *root = doc->FirstChild("images");
	TiXmlNode *child = NULL;
	while (child = root->IterateChildren(child)) {
		// skip comments
		if (child->Type() == TiXmlNode::TINYXML_COMMENT) continue;

		TiXmlElement *element = child->ToElement();
		
		ImageData *data = new ImageData();
		
		// TODO: add error checking
		element->QueryIntAttribute("id", &(data->id));
		data->name = string(element->Attribute("name"));
		data->path = dirName + string(element->Attribute("path"));
		std::cerr << "Path = " << data->path << std::endl;

		TiXmlText *description = child->FirstChild()->ToText();
		data->description = string(description->Value());
		std::cerr << "Description = " << data->description << std::endl;

		data->image = cvLoadImage((data->path).c_str(), CV_LOAD_IMAGE_GRAYSCALE);

		referenceData_.push_back(data);
	}

	// extract data for SURF
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSURFParams params = cvSURFParams(400, 1);

	vector<ImageData *>::const_iterator refIt;
	CvSeq *keyPoint, *descriptor;
	for (refIt = referenceData_.begin(); refIt != referenceData_.end(); ++refIt) {
		std::cerr << "refname " << (*refIt)->name << std::endl;
		cvExtractSURF((*refIt)->image, 0, &keyPoint, &descriptor, storage, params);
		refKD_.push_back(make_pair(keyPoint, descriptor));
	}

	if (refKD_.size() != referenceData_.size()) {
		logger_->Log(WARNING, "Only extracted SURF features from %d / %d images!\n",
			refKD_.size(), referenceData_.size());
	}

	logger_->Log(INFO, "Build Time for %d Images = %gm\n",
		refKD_.size(), ((tt + cvGetTickCount()) / cvGetTickFrequency()*1000.));
	
	return referenceData_.size();
/*
	string referenceDirName;
	Util::GetFileNamesFromFile(fileName, referenceDirName, referenceImageNames_);
	if (referenceImageNames_.empty()) {
		logger_->Log(ERR, "No images to build SURF library!\n");
		return 0;
	}

	int errCount = 0;
	vector<string>::const_iterator it;
	for (it = referenceImageNames_.begin(); it != referenceImageNames_.end(); ++it) {
		string name = referenceDirName + *it;
		IplImage *img = cvLoadImage(name.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
		if (!img) {
			++errCount;
			logger_->Log(ERR, "Reference image %s could not be read!\n", name.c_str());
		} else {
			referenceImages_.push_back(img);
		}
	}

	if (errCount) {
		logger_->Log(WARNING, "Error reading %d reference images!\n", errCount);
	}

	// TODO: prolly move as params to ctor	
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSURFParams params = cvSURFParams(400, 1);

	vector<IplImage *>::const_iterator refIt;
	CvSeq *keyPoint, *descriptor;
	for (refIt = referenceImages_.begin(); refIt != referenceImages_.end(); ++refIt) {
		cvExtractSURF(*refIt, 0, &keyPoint, &descriptor, storage, params);
		refKD_.push_back(make_pair(keyPoint, descriptor));
	}

	if (refKD_.size() != referenceImages_.size()) {
		logger_->Log(WARNING, "Only extracted %d / %d images!\n",
			refKD_.size(), referenceImages_.size());
	}

	logger_->Log(INFO, "Build Time for %d Images = %gm\n",
		refKD_.size(), ((tt + cvGetTickCount()) / cvGetTickFrequency()*1000.));
	
	return referenceImages_.size();
	*/
}

double SURFMatcher::MatchAgainstLibrary(const char *queryImageName,
		const IplImage *queryImage, const CvSeq *queryKeyPoints, const CvSeq *queryDescriptors) const {
	logger_->Log(INFO, "Using SURF to match %s to image library\n", queryImageName);
	IplImage *referenceImage = NULL;
	double matchPercentage = 0;
	vector<int> ptPairs;

	int indexBestMatch = -1;
	double bestPercentage = 0;
	double tt = (double) -cvGetTickCount();
	for (int i = 0; i < referenceData_.size(); ++i) {
		referenceImage = referenceData_[i]->image;
		SURFFeatures_t features = refKD_[i];

		ptPairs.clear();
		matchPercentage = FindPairs(queryKeyPoints,
			queryDescriptors, features.first, features.second, ptPairs);

		logger_->Log(VERBOSE, "\t%s matched %2.2f%% with %s\n",
			queryImageName, matchPercentage, referenceData_[i]->name.c_str());

		if (matchPercentage > bestPercentage) {
			indexBestMatch = i;
			bestPercentage = matchPercentage;
		}
	}
	
	logger_->Log(INFO, "Match Time for %s = %gm\n",
		queryImageName, ((tt + cvGetTickCount()) / cvGetTickFrequency()*1000.));
	

	// TODO: need to put threshold on match. There may not always be a matching image in the library. min must be > 10%?
	// TODO: this is also a bug if referenceImages_.size != referenceImageNames_.size
	if (indexBestMatch >= 0 && indexBestMatch < referenceData_.size()) {
		logger_->Log(INFO, "%s was best matched with %s\n",
			queryImageName, referenceData_[indexBestMatch]->name.c_str());
	} else {
		logger_->Log(INFO, "Unable to find a match for %s\n", queryImageName);
	}

	return bestPercentage;
}

// object = the query image
// image = scene in which to find 'object'
double SURFMatcher::FindPairs(const CvSeq* objectKeypoints, const CvSeq* objectDescriptors,
		const CvSeq* imageKeypoints, const CvSeq* imageDescriptors, vector<int>& ptpairs) const {
    int i;
    CvSeqReader reader, kreader;
    cvStartReadSeq(objectKeypoints, &kreader);
    cvStartReadSeq(objectDescriptors, &reader);
    ptpairs.clear();

	double numMatched = 0.;
    for (i = 0; i < objectDescriptors->total; i++) {
        const CvSURFPoint* kp = (const CvSURFPoint*) kreader.ptr;
        const float* descriptor = (const float *) reader.ptr;
        CV_NEXT_SEQ_ELEM(kreader.seq->elem_size, kreader);
        CV_NEXT_SEQ_ELEM(reader.seq->elem_size, reader);
        int nearest_neighbor = NaiveNearestNeighbor(descriptor, kp->laplacian, imageKeypoints, imageDescriptors);
        if (nearest_neighbor >= 0) {
            ptpairs.push_back(i);
            ptpairs.push_back(nearest_neighbor);
			++numMatched;
        }
    }

	// TODO: SUPER NAIVE match :( We should care more about the concentration of the match I think rather than percentage matched
	return (numMatched / objectDescriptors->total) * 100;
}

// Using brute force
// vec = the query descriptor
// model_* = the library descriptors to match against.
int SURFMatcher::NaiveNearestNeighbor(const float* vec, int laplacian,
                      const CvSeq* model_keypoints,
					  const CvSeq* model_descriptors) const {
    int length = (int) (model_descriptors->elem_size / sizeof(float));
    int i, neighbor = -1;
    double d, dist1 = 1e6, dist2 = 1e6;
    CvSeqReader reader, kreader;
    cvStartReadSeq(model_keypoints, &kreader, 0);
    cvStartReadSeq(model_descriptors, &reader, 0);

    for (i = 0; i < model_descriptors->total; i++) {
        const CvSURFPoint* kp = (const CvSURFPoint*) kreader.ptr;
        const float* mvec = (const float*) reader.ptr;
    	CV_NEXT_SEQ_ELEM(kreader.seq->elem_size, kreader);
        CV_NEXT_SEQ_ELEM(reader.seq->elem_size, reader);
        if (laplacian != kp->laplacian)
            continue;
        d = CompareSURFDescriptors(vec, mvec, dist2, length);
        if (d < dist1) {
            dist2 = dist1;
            dist1 = d;
            neighbor = i;
        } else if (d < dist2) {
            dist2 = d;
		}
    }

	return (dist1 < 0.6*dist2) ? neighbor : -1;
}

double SURFMatcher::CompareSURFDescriptors( const float* d1, const float* d2, double best, int length ) const {
    double total_cost = 0;
    assert( length % 4 == 0 );
    for( int i = 0; i < length; i += 4 )
    {
        double t0 = d1[i] - d2[i];
        double t1 = d1[i+1] - d2[i+1];
        double t2 = d1[i+2] - d2[i+2];
        double t3 = d1[i+3] - d2[i+3];
        total_cost += t0*t0 + t1*t1 + t2*t2 + t3*t3;
        if( total_cost > best )
            break;
    }
    return total_cost;
}

// Save image to disk with query located in reference image as well as the query
// image on top of the reference image with lines drawn between them.
void SURFMatcher::Visualize(string dirName, string queryImageName,
	IplImage *queryImage, CvSeq *queryKeyPoints, CvSeq *queryDescriptors) const {

	CvPoint src_corners[4] = {{0,0}, 
							  {queryImage->width, 0}, 
							  {queryImage->width, queryImage->height},
						      {0, queryImage->height}};
    CvPoint dst_corners[4];
    IplImage *correspond, *referenceImage;

	// TODO: Also assumes we have as many reference images as we supplied in the text file
	// Very presumptious and prone to error... UGHHH WHY DOESN'T IT STRIP THE EXTENSION
	size_t end = queryImageName.rfind('.');
	size_t begin = queryImageName.rfind('/');
    string queryName = queryImageName.substr(begin+1, end);
	string out_base = dirName + "/res_" + queryName + "_";

	double tt = (double) -cvGetTickCount();
	for (int i = 0; i < referenceData_.size(); ++i) {
		referenceImage = referenceData_[i]->image;

		correspond = cvCreateImage(cvSize(referenceImage->width, queryImage->height + referenceImage->height), 8, 1);
		cvSetImageROI(correspond, cvRect(0, 0, queryImage->width, queryImage->height));
	    cvCopy(queryImage, correspond);
	    cvSetImageROI(correspond, cvRect(0, queryImage->height, correspond->width, correspond->height));
	    cvCopy(referenceImage, correspond);
	    cvResetImageROI(correspond);

		SURFFeatures_t features = refKD_[i];
		
		// Draw a square in the training image corresponding to the query
		vector<int> ptpairs;
		FindPairs(queryKeyPoints, queryDescriptors, features.first, features.second, ptpairs);
	
		
		if (LocatePlanarObject(queryKeyPoints, queryDescriptors, features.first, features.second, ptpairs,
				src_corners, dst_corners)) {
	        for (int c = 0; c < 4; ++c) {
	            CvPoint r1 = dst_corners[c%4];
	            CvPoint r2 = dst_corners[(c+1)%4];
	            cvLine(correspond, cvPoint(r1.x, r1.y + queryImage->height),
					cvPoint(r2.x, r2.y + queryImage->height ), colors[8], 2);
	        }
		}

		ConnectMatchingFeatures(correspond, queryKeyPoints, features.first, ptpairs, queryImage->height);

		string out_name = out_base + referenceData_[i]->name + ".jpg";
		if (!cvSaveImage(out_name.c_str(), correspond)) {
			logger_->Log(WARNING, "Could not save image: %s", out_name.c_str());
		}
	}

	
	logger_->Log(INFO, "Visualization Time for %s = %gm\n",
		queryImageName.c_str(), ((tt + cvGetTickCount()) / cvGetTickFrequency()*1000.));
}

// Draw lines between matching features of object and image, where
// object is presumed to be in image. correspond is the combo of the object pic on top of
// the image pic. So the lines are drawn between those 2 pics.
void SURFMatcher::ConnectMatchingFeatures(IplImage *correspond, const CvSeq *objectKeypoints,
		const CvSeq *imageKeypoints, const vector<int>& ptpairs, int objectHeight) const {
			
	for (int i = 0; i < (int) ptpairs.size(); i += 2) {
        CvSURFPoint* r1 = (CvSURFPoint*)cvGetSeqElem(objectKeypoints, ptpairs[i]);
        CvSURFPoint* r2 = (CvSURFPoint*)cvGetSeqElem(imageKeypoints, ptpairs[i+1]);
        cvLine(correspond, cvPointFrom32f(r1->pt),
            cvPoint(cvRound(r2->pt.x), cvRound(r2->pt.y + objectHeight)), colors[8]);
    }
}


int SURFMatcher::LocatePlanarObject(const CvSeq* objectKeypoints, const CvSeq* objectDescriptors,
                    const CvSeq* imageKeypoints, const CvSeq* imageDescriptors,
					const vector<int>& ptpairs, const CvPoint src_corners[4], CvPoint dst_corners[4]) const {
    double h[9];
    CvMat _h = cvMat(3, 3, CV_64F, h);
    vector<CvPoint2D32f> pt1, pt2;
    CvMat _pt1, _pt2;
    int i, n;

    n = ptpairs.size()/2;
    if( n < 4 )
        return 0;

    pt1.resize(n);
    pt2.resize(n);
    for( i = 0; i < n; i++ )
    {
        pt1[i] = ((CvSURFPoint*)cvGetSeqElem(objectKeypoints,ptpairs[i*2]))->pt;
        pt2[i] = ((CvSURFPoint*)cvGetSeqElem(imageKeypoints,ptpairs[i*2+1]))->pt;
    }

    _pt1 = cvMat(1, n, CV_32FC2, &pt1[0] );
    _pt2 = cvMat(1, n, CV_32FC2, &pt2[0] );
    if( !cvFindHomography( &_pt1, &_pt2, &_h, CV_RANSAC, 5 ))
        return 0;

    for( i = 0; i < 4; i++ )
    {
        double x = src_corners[i].x, y = src_corners[i].y;
        double Z = 1./(h[6]*x + h[7]*y + h[8]);
        double X = (h[0]*x + h[1]*y + h[2])*Z;
        double Y = (h[3]*x + h[4]*y + h[5])*Z;
        dst_corners[i] = cvPoint(cvRound(X), cvRound(Y));
    }

    return 1;
}
