#pragma once

#include "cv.h"
#include "highgui.h"

#include "Util.h"


#define MINUS_INFINITIVE	-1E20


class ANCCwithBP
{
public:
	ANCCwithBP(void);
	~ANCCwithBP(void);


	void match(IplImage *leftImg, IplImage *rightImg, IplImage* disparityImg, 
				int disparityRangeFrom, int disparityRangeTo,
				int winSize, double sigmaDistance, double sigmaColor,
				int nPyramidLevels, int nIter, double discCostTruncate, double dataCostTruncate, double dataCostWeighting);


private:


	// ------ release ------

	void releaseBPdataCostsAndMsg(double**** dataCosts, 
								  int* dataCostsWidth, int* dataCostsHeight,
								  double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight,
								  int nPyramidLevels);


	// ------ run ------

	void outputTheFinalDisparityImg(double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight, 
									double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
									int nDisparities, int winSize,
									IplImage* disparityImg);

	void runBPonOtherLevels(double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight, 
							double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
							int nPyramidLevels, int nDisparities, 
							int nIter, double discCostTruncate);

	void runBPonTheCoarsestLevel(double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight, 
								double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
								int nPyramidLevels, int nDisparities, 
								int nIter, double discCostTruncate);

	void bp(double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight, 
			double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
			int levelToRun, int nDisparities, int nIter, double discCostTruncate);

	void msg(double* s1, double* s2, double* s3, double* s4, double* dst,
			 int nDisparities, double discCostTruncate);

	void dt(double* f, int nDisparities);


	// ------ init ------	

	void initPyramidBPdataCosts(double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
								int nPyramidLevels, int imgH, int imgW, int nDisparities);

	void initTheFinestLevelOfBPdataCostsUsingANCC(IplImage *leftImg, IplImage *rightImg, double**** dataCosts, 
												int disparityRangeTo, int disparityRangeFrom,
												int winSize, double sigmaDistance, double sigmaColor,
												double dataCostTruncate, double dataCostWeighting);
};
