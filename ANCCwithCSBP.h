#pragma once

#include "cv.h"
#include "highgui.h"

#include "Util.h"


class ANCCwithCSBP
{
public:
	ANCCwithCSBP(void);
	~ANCCwithCSBP(void);


	void match(IplImage *leftImg, IplImage *rightImg, IplImage* disparityImg, 
				int disparityRangeFrom, int disparityRangeTo,
				int winSize, double sigmaDistance, double sigmaColor,
				int nPyramidLevels, int nIter, double discCostTruncate, double dataCostTruncate, double dataCostWeighting);


private:

	void initTheCoarsestLevelOfBPdataCostsUsingANCC(IplImage *leftImg, IplImage *rightImg, 
													CvMemStorage**** dataCostsStorage, CvSeq**** dataCosts,
													int disparityRangeTo, int disparityRangeFrom,
													int winSize, double sigmaDistance, double sigmaColor,
													int nPyramidLevels, double dataCostTruncate, double dataCostWeighting);

	void createPyramidImages(IplImage *leftImg, IplImage *rightImg, int nPyramidLevels, 
							 IplImage **leftPyramidImages, IplImage **rightPyramidImages);
};

