#include "StdAfx.h"
#include "ANCCwithCSBP.h"


ANCCwithCSBP::ANCCwithCSBP(void) { }
ANCCwithCSBP::~ANCCwithCSBP(void) { }


void ANCCwithCSBP::match( /* images */				IplImage *leftImg, IplImage *rightImg, IplImage* disparityImg, 
						  /* disparity range */		int disparityRangeFrom, int disparityRangeTo, 
						  /* ANCC parameters */		int winSize, double sigmaDistance, double sigmaColor,
						  /* BP parameters */		int nPyramidLevels, int nIter, double discCostTruncate, double dataCostTruncate, double dataCostWeighting) {

	int imgW = leftImg->width;
	int imgH = leftImg->height;
	
	disparityRangeFrom	/= nPyramidLevels;
	disparityRangeTo	/= nPyramidLevels;
	winSize				/= nPyramidLevels;
	sigmaDistance		/= nPyramidLevels;
	int nDisparities = disparityRangeTo - disparityRangeFrom;



	// ------------------ create pyramid images ------------------

	IplImage **leftPyramidImages = new IplImage*[nPyramidLevels];
	IplImage **rightPyramidImages = new IplImage*[nPyramidLevels];

	createPyramidImages(leftImg, rightImg, nPyramidLevels, leftPyramidImages, rightPyramidImages);
	//cvShowImage("Pyramid image", leftPyramidImages[1]);


	// ------------------ init data costs ------------------

	CvMemStorage**** dataCostsStorage = new CvMemStorage***[nPyramidLevels];
	CvSeq**** dataCosts = new CvSeq***[nPyramidLevels];	

	initTheCoarsestLevelOfBPdataCostsUsingANCC(leftPyramidImages[nPyramidLevels - 1], rightPyramidImages[nPyramidLevels - 1], 
											dataCostsStorage, dataCosts,
											disparityRangeTo, disparityRangeFrom, 
											winSize, sigmaDistance, sigmaColor,
											nPyramidLevels, dataCostTruncate, dataCostWeighting);


	// ------------------ test: print the best match from data costs ------------------
	int testPyramidLevel = nPyramidLevels - 1;

	uchar* disparityImgData = (uchar *) disparityImg->imageData;
	int disparityImgStep = disparityImg->widthStep;
	int disparityMultiplier = Util::calcDisparityMultiplier(nDisparities);

	int sizeOfDisparityList = dataCosts[testPyramidLevel][0][0]->total;

	for(int i = 0; i < leftPyramidImages[testPyramidLevel]->height; i++) {
		for(int j = 0; j < leftPyramidImages[testPyramidLevel]->width - winSize; j++) {
			double bestAncc = -1;
			int bestDisparity = -1;			
			for(int d = 0; d < sizeOfDisparityList; d++) {
				CvPoint2D32f* ancc = (CvPoint2D32f*) cvGetSeqElem(dataCosts[testPyramidLevel][i][j], d);
				if(bestAncc < ancc->y) {
					bestAncc = ancc->y;
					bestDisparity = int(ancc->x);
				}
			}
			disparityImgData[i*disparityImgStep + (j + winSize/2)] = bestDisparity * disparityMultiplier;
		}
	}
}


// ===================================================================================================


void ANCCwithCSBP::initTheCoarsestLevelOfBPdataCostsUsingANCC(IplImage *leftImg, IplImage *rightImg, 
															CvMemStorage**** dataCostsStorage, CvSeq**** dataCosts,
															int disparityRangeTo, int disparityRangeFrom,
															int winSize, double sigmaDistance, double sigmaColor,
															int nPyramidLevels, double dataCostTruncate, double dataCostWeighting) {
	/*CStdioFile dataCostsFile;
	dataCostsFile.Open(L"dataCosts.txt", CFile::modeCreate | CFile::modeWrite);
	CString text;*/


	// images' info
	uchar* leftImgData = (uchar *) leftImg->imageData;
	uchar* rightImgData = (uchar *) rightImg->imageData;	
	int imgS = leftImg->widthStep;
	int imgC = leftImg->nChannels;
	int imgW = leftImg->width;
	int imgH = leftImg->height;


	// scanning info
	int halfWinSize = winSize/2;
	int startX = halfWinSize;			int endX = imgW - halfWinSize - 1;
	int startY = 0;						int endY = imgH - 1;


	// main run
	double *list_left_aPixelWeight					= new double[winSize];
	double *list_right_aPixelWeight					= new double[winSize];

	double *list_left_aPixelChromNormCoeff_blue		= new double[winSize];
	double *list_left_aPixelChromNormCoeff_green	= new double[winSize];
	double *list_left_aPixelChromNormCoeff_red		= new double[winSize];

	double *list_right_aPixelChromNormCoeff_blue	= new double[winSize];
	double *list_right_aPixelChromNormCoeff_green	= new double[winSize];
	double *list_right_aPixelChromNormCoeff_red		= new double[winSize];

	double *list_left_aPixelWeight_normalized		= new double[winSize];
	double *list_right_aPixelWeight_normalized		= new double[winSize];	
	
	int coarsestLevelID = nPyramidLevels - 1;

	dataCostsStorage[coarsestLevelID]	= new CvMemStorage**[endY - startY + 1];
	dataCosts[coarsestLevelID]			= new CvSeq**[endY - startY + 1];													// <---	


	for(int i = startY; i <= endY; i++) {

		int iMinusStartY = i - startY;

		dataCostsStorage[coarsestLevelID][iMinusStartY]	= new CvMemStorage*[endX - startX + 1];
		dataCosts[coarsestLevelID][iMinusStartY]		= new CvSeq*[endX - startX + 1];										// <---		


		for(int j = startX; j <= endX; j++) {

			int jMinusStartX = j - startX;

			dataCostsStorage[coarsestLevelID][iMinusStartY][jMinusStartX]	= cvCreateMemStorage(0);
			dataCosts[coarsestLevelID][iMinusStartY][jMinusStartX]			= cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(CvPoint2D32f), dataCostsStorage[coarsestLevelID][iMinusStartY][jMinusStartX]);


			for(int d = disparityRangeFrom; d < disparityRangeTo; d++) {

				int jPlusD = j + d;

				// get the center pixel's position
				CvPoint left_centerPixelPos		= cvPoint(jPlusD, i);
				CvPoint right_centerPixelPos	= cvPoint(j, i);

				// get the center pixel's color
				CvPoint3D32f left_centerPixelColor	= Util::getColor(leftImgData, imgS, imgC, i, jPlusD);
				CvPoint3D32f right_centerPixelColor = Util::getColor(rightImgData, imgS, imgC, i, j);							

				// calculate weights and chromaticity normalization coefficients in advance
				Util::calcWeightsAndChromNormCoeffs(halfWinSize, i, j, d,
													leftImgData, rightImgData, imgS, imgC, 
													left_centerPixelPos, right_centerPixelPos,
													left_centerPixelColor, right_centerPixelColor,
													sigmaDistance, sigmaColor,
													list_left_aPixelWeight, list_right_aPixelWeight,
													list_left_aPixelChromNormCoeff_blue, list_left_aPixelChromNormCoeff_green, list_left_aPixelChromNormCoeff_red,
													list_right_aPixelChromNormCoeff_blue, list_right_aPixelChromNormCoeff_green, list_right_aPixelChromNormCoeff_red);

				// normalize weights
				Util::normalizeWeights(list_left_aPixelWeight, list_left_aPixelWeight_normalized, winSize);
				Util::normalizeWeights(list_right_aPixelWeight, list_right_aPixelWeight_normalized, winSize);

				// calculate the chromaticity normalization coefficients with weights
				double left_chromNormCoeffWithWeight_blue	= Util::calcChromNormCoeffWithWeight(list_left_aPixelWeight_normalized, list_left_aPixelChromNormCoeff_blue, winSize);
				double left_chromNormCoeffWithWeight_green	= Util::calcChromNormCoeffWithWeight(list_left_aPixelWeight_normalized, list_left_aPixelChromNormCoeff_green, winSize);
				double left_chromNormCoeffWithWeight_red	= Util::calcChromNormCoeffWithWeight(list_left_aPixelWeight_normalized, list_left_aPixelChromNormCoeff_red, winSize);

				double right_chromNormCoeffWithWeight_blue	= Util::calcChromNormCoeffWithWeight(list_right_aPixelWeight_normalized, list_right_aPixelChromNormCoeff_blue, winSize);
				double right_chromNormCoeffWithWeight_green = Util::calcChromNormCoeffWithWeight(list_right_aPixelWeight_normalized, list_right_aPixelChromNormCoeff_green, winSize);
				double right_chromNormCoeffWithWeight_red	= Util::calcChromNormCoeffWithWeight(list_right_aPixelWeight_normalized, list_right_aPixelChromNormCoeff_red, winSize);

				// calculate ANCC for 3 channels of left and right images
				double anccBlue		= Util::calcANCC(list_left_aPixelWeight, list_right_aPixelWeight,
													list_left_aPixelChromNormCoeff_blue, list_right_aPixelChromNormCoeff_blue,
													left_chromNormCoeffWithWeight_blue, right_chromNormCoeffWithWeight_blue,
													winSize);
				double anccGreen	= Util::calcANCC(list_left_aPixelWeight, list_right_aPixelWeight,
													list_left_aPixelChromNormCoeff_green, list_right_aPixelChromNormCoeff_green,
													left_chromNormCoeffWithWeight_green, right_chromNormCoeffWithWeight_green,
													winSize);
				double anccRed		= Util::calcANCC(list_left_aPixelWeight, list_right_aPixelWeight,
													list_left_aPixelChromNormCoeff_red, list_right_aPixelChromNormCoeff_red,
													left_chromNormCoeffWithWeight_red, right_chromNormCoeffWithWeight_red,
													winSize);

				double totalOfAncc	= anccBlue + anccGreen + anccRed; // might be changed to another function instead of SUM
				//double totalOfAncc	= anccBlue * anccGreen * anccRed; 


				// store the cost totalOfAncc to dataCosts
				CvPoint2D32f cost = cvPoint2D32f( d - disparityRangeFrom, dataCostWeighting * MAX(totalOfAncc, dataCostTruncate) );
				cvSeqPush(dataCosts[coarsestLevelID][iMinusStartY][jMinusStartX], &cost);


				//
				/*if(d == disparityRangeFrom) {
					text.Format(L"%1.3f ", totalOfAncc);
					dataCostsFile.WriteString(text);
				}*/
			}
		}
	}

	//dataCostsFile.Close();


	// release
	delete[] list_left_aPixelWeight;
	delete[] list_right_aPixelWeight;

	delete[] list_left_aPixelChromNormCoeff_blue;
	delete[] list_left_aPixelChromNormCoeff_green;
	delete[] list_left_aPixelChromNormCoeff_red;
	
	delete[] list_right_aPixelChromNormCoeff_blue;
	delete[] list_right_aPixelChromNormCoeff_green;
	delete[] list_right_aPixelChromNormCoeff_red;

	delete[] list_left_aPixelWeight_normalized;
	delete[] list_right_aPixelWeight_normalized;
}


void ANCCwithCSBP::createPyramidImages(IplImage *leftImg, IplImage *rightImg, int nPyramidLevels, 
									   IplImage **leftPyramidImages, IplImage **rightPyramidImages) {
	leftPyramidImages[0] = leftImg;
	rightPyramidImages[0] = rightImg;

	for(int i = 1; i < nPyramidLevels; i++) {
		int coarserLevelID = i - 1;
		int currentLevelID = i;

		int imgW = leftPyramidImages[coarserLevelID]->width / 2;
		int imgH = leftPyramidImages[coarserLevelID]->height / 2;

		leftPyramidImages[currentLevelID] = cvCreateImage(cvSize(imgW, imgH), IPL_DEPTH_8U, leftImg->nChannels);
		rightPyramidImages[currentLevelID] = cvCreateImage(cvSize(imgW, imgH), IPL_DEPTH_8U, leftImg->nChannels);

		//cvPyrDown(leftPyramidImages[coarserLevelID], leftPyramidImages[currentLevelID]);
		//cvPyrDown(rightPyramidImages[coarserLevelID], rightPyramidImages[currentLevelID]);
		cvResize(leftPyramidImages[coarserLevelID], leftPyramidImages[currentLevelID]);
		cvResize(rightPyramidImages[coarserLevelID], rightPyramidImages[currentLevelID]);
	}
}