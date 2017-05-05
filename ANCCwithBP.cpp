/*
	The result of ANCC is then optimized by using Belief Propagation (BP):
	P. Felzenszwalb and D. Huttenlocher, "Efficient BP for early vision," IJCV 2006
*/


#include "StdAfx.h"
#include "ANCCwithBP.h"


ANCCwithBP::ANCCwithBP(void) { }
ANCCwithBP::~ANCCwithBP(void) { }


void ANCCwithBP::match( /* images */			IplImage *leftImg, IplImage *rightImg, IplImage* disparityImg, 
						/* disparity range */	int disparityRangeFrom, int disparityRangeTo, 
						/* ANCC parameters */	int winSize, double sigmaDistance, double sigmaColor,
						/* BP parameters */		int nPyramidLevels, int nIter, double discCostTruncate, double dataCostTruncate, double dataCostWeighting) {

	int imgW = leftImg->width;
	int imgH = leftImg->height;
	int nDisparities = disparityRangeTo - disparityRangeFrom;
	

	// ------------------ init data costs ------------------

	double**** dataCosts = new double***[nPyramidLevels];

	initTheFinestLevelOfBPdataCostsUsingANCC(leftImg, rightImg, dataCosts, 
											disparityRangeTo, disparityRangeFrom, 
											winSize, sigmaDistance, sigmaColor,
											dataCostTruncate, dataCostWeighting);

	
	// ------------------ init the pyramid of data costs ------------------

	int* dataCostsWidth = new int[nPyramidLevels];		dataCostsWidth[0] = imgW - winSize;
	int* dataCostsHeight = new int[nPyramidLevels];		dataCostsHeight[0] = imgH;	

	initPyramidBPdataCosts(dataCosts, dataCostsWidth, dataCostsHeight, 
							nPyramidLevels, imgH, imgW, nDisparities);	


	// ------------------ test: print the best match from data costs ------------------
	/*int testPyramidLevel = 0;

	uchar* disparityImgData = (uchar *) disparityImg->imageData;
	int disparityImgStep = disparityImg->widthStep;
	int disparityMultiplier = Util::calcDisparityMultiplier(nDisparities);

	for(int i = 0; i < dataCostsHeight[testPyramidLevel]; i++) {
		for(int j = 0; j < dataCostsWidth[testPyramidLevel]; j++) {
			double bestAncc = -1;
			int bestDisparity = -1;
			for(int d = 0; d < nDisparities; d++) {
				if(bestAncc < dataCosts[testPyramidLevel][i][j][d]) {
					bestAncc = dataCosts[testPyramidLevel][i][j][d];
					bestDisparity = d;
				}
			}
			disparityImgData[i*disparityImgStep + (j + winSize/2)] = bestDisparity * disparityMultiplier;
		}
	}*/


	// ------------------ run BP from coarse to fine ------------------

	double**** msgUp	= new double***[nPyramidLevels];
	double**** msgDown	= new double***[nPyramidLevels];
	double**** msgLeft	= new double***[nPyramidLevels];
	double**** msgRight	= new double***[nPyramidLevels];

	runBPonTheCoarsestLevel(msgUp, msgDown, msgLeft, msgRight, 
							dataCosts, dataCostsWidth, dataCostsHeight, 
							nPyramidLevels, nDisparities, 
							nIter, discCostTruncate);

	runBPonOtherLevels(msgUp, msgDown, msgLeft, msgRight, 
						dataCosts, dataCostsWidth, dataCostsHeight,
						nPyramidLevels, nDisparities, 
						nIter, discCostTruncate);

	outputTheFinalDisparityImg(msgUp, msgDown, msgLeft, msgRight, 
								dataCosts, dataCostsWidth, dataCostsHeight,
								nDisparities, winSize,
								disparityImg);
	

	// ------------------ RELEASE ------------------

	releaseBPdataCostsAndMsg(dataCosts, dataCostsWidth, dataCostsHeight, msgUp, msgDown, msgLeft, msgRight, nPyramidLevels);	
}


// ----------------------------------------------------------------------------------------------


void ANCCwithBP::releaseBPdataCostsAndMsg(double**** dataCosts, 
										  int* dataCostsWidth, int* dataCostsHeight,
										  double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight,
										  int nPyramidLevels) {
	for(int l = 0; l < nPyramidLevels; l++) {
		for(int i = 0; i < dataCostsHeight[l]; i++) {
			for(int j = 0; j < dataCostsWidth[l]; j++) {
				delete[] dataCosts[l][i][j]; 
				delete[] msgUp[l][i][j];
				delete[] msgDown[l][i][j];
				delete[] msgLeft[l][i][j];
				delete[] msgRight[l][i][j];
			}
			delete[] dataCosts[l][i];
			delete[] msgUp[l][i];
			delete[] msgDown[l][i];
			delete[] msgLeft[l][i];
			delete[] msgRight[l][i];			
		}
		delete[] dataCosts[l];
		delete[] msgUp[l];
		delete[] msgDown[l];
		delete[] msgLeft[l];
		delete[] msgRight[l];
	}
	delete[] dataCosts;
	delete[] msgUp;
	delete[] msgDown;
	delete[] msgLeft;
	delete[] msgRight;

	// release dataCostsWidth and dataCostsHeight
	delete[] dataCostsWidth;
	delete[] dataCostsHeight;
}


// ----------------------------------------------------------------------------------------------


void ANCCwithBP::outputTheFinalDisparityImg(double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight, 
											double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
											int nDisparities, int winSize,
											IplImage* disparityImg) {
	/*CStdioFile dataCostsFile;
	dataCostsFile.Open(L"dataCosts.txt", CFile::modeCreate | CFile::modeWrite);
	CString text;*/

	// image info
	uchar* disparityImgData = (uchar *) disparityImg->imageData;
	int disparityImgStep = disparityImg->widthStep;
	int disparityMultiplier = Util::calcDisparityMultiplier(nDisparities);

	//
	int coarsestLevel = 0;	
	int imgH = dataCostsHeight[coarsestLevel];
	int imgW = dataCostsWidth[coarsestLevel];

	//
	for(int i = 1; i < imgH - 1; i++) {
		for(int j = 1; j < imgW - 1; j++) {
			double bestAncc = MINUS_INFINITIVE;
			int bestDisparity = -1;

			double ancc = -1;
			for(int d = 0; d < nDisparities; d++) {
				ancc = msgUp[coarsestLevel][i+1][j][d] + msgDown[coarsestLevel][i-1][j][d] + msgLeft[coarsestLevel][i][j+1][d] + msgRight[coarsestLevel][i][j-1][d] + dataCosts[coarsestLevel][i][j][d];
				//ancc = dataCosts[coarsestLevel][i][j][d];
				if(bestAncc < ancc) {
					bestAncc = ancc;
					bestDisparity = d;
				}
			}

			disparityImgData[i*disparityImgStep + (j + winSize/2)] = bestDisparity * disparityMultiplier;

			/*if(bestDisparity == -1) {
				text.Format(L"%d %d %f \n", i, j, ancc);
				dataCostsFile.WriteString(text);
			}*/
		}
	}

	//dataCostsFile.Close();
}


void ANCCwithBP::runBPonOtherLevels(double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight, 
									double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
									int nPyramidLevels, int nDisparities, 
									int nIter, double discCostTruncate) {

	for(int l = nPyramidLevels - 2; l >=0; l--) {
		int imgH = dataCostsHeight[l];
		int imgW = dataCostsWidth[l];

		msgUp[l]	= new double**[imgH];
		msgDown[l]	= new double**[imgH];
		msgLeft[l]	= new double**[imgH];
		msgRight[l] = new double**[imgH];

		for(int i = 0; i < imgH; i++) {
			msgUp[l][i]		= new double*[imgW];
			msgDown[l][i]	= new double*[imgW];
			msgLeft[l][i]	= new double*[imgW];
			msgRight[l][i]	= new double*[imgW];

			for(int j = 0; j < imgW; j++) {
				msgUp[l][i][j]		= new double[nDisparities];
				msgDown[l][i][j]	= new double[nDisparities];
				msgLeft[l][i][j]	= new double[nDisparities];
				msgRight[l][i][j]	= new double[nDisparities];

				for(int d = 0; d < nDisparities; d++) {
					msgUp[l][i][j][d]		= msgUp[l+1][i/2][j/2][d];
					msgDown[l][i][j][d]		= msgDown[l+1][i/2][j/2][d];
					msgLeft[l][i][j][d]		= msgLeft[l+1][i/2][j/2][d];
					msgRight[l][i][j][d]	= msgRight[l+1][i/2][j/2][d];
				}
			}
		}

		bp(msgUp, msgDown, msgLeft, msgRight, dataCosts, dataCostsWidth, dataCostsHeight, l, nDisparities, nIter, discCostTruncate);
	}
}


void ANCCwithBP::runBPonTheCoarsestLevel(double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight, 
										double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
										int nPyramidLevels, int nDisparities, 
										int nIter, double discCostTruncate) {
	int coarsestLevel = nPyramidLevels - 1;

	int imgH = dataCostsHeight[coarsestLevel];
	int imgW = dataCostsWidth[coarsestLevel];

	msgUp[coarsestLevel]	= new double**[imgH];
	msgDown[coarsestLevel]	= new double**[imgH];
	msgLeft[coarsestLevel]	= new double**[imgH];
	msgRight[coarsestLevel] = new double**[imgH];

	for(int i = 0; i < imgH; i++) {
		msgUp[coarsestLevel][i]		= new double*[imgW];
		msgDown[coarsestLevel][i]	= new double*[imgW];
		msgLeft[coarsestLevel][i]	= new double*[imgW];
		msgRight[coarsestLevel][i]	= new double*[imgW];

		for(int j = 0; j < imgW; j++) {
			msgUp[coarsestLevel][i][j]		= new double[nDisparities];
			msgDown[coarsestLevel][i][j]	= new double[nDisparities];
			msgLeft[coarsestLevel][i][j]	= new double[nDisparities];
			msgRight[coarsestLevel][i][j]	= new double[nDisparities];

			for(int d = 0; d < nDisparities; d++) {
				msgUp[coarsestLevel][i][j][d]		= 0;
				msgDown[coarsestLevel][i][j][d]		= 0;
				msgLeft[coarsestLevel][i][j][d]		= 0;
				msgRight[coarsestLevel][i][j][d]	= 0;
			}
		}
	}
	
	bp(msgUp, msgDown, msgLeft, msgRight, dataCosts, dataCostsWidth, dataCostsHeight, coarsestLevel, nDisparities, nIter, discCostTruncate);
}


void ANCCwithBP::bp(double**** msgUp, double**** msgDown, double**** msgLeft, double**** msgRight, 
					double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
					int levelToRun, int nDisparities, int nIter, double discCostTruncate) {
	int imgH = dataCostsHeight[levelToRun];
	int imgW = dataCostsWidth[levelToRun];

	for(int t = 0; t < nIter; t++) {
		for(int i = 1; i < imgH - 1; i++) {
			for(int j = ((i + t) % 2) + 1; j < imgW - 1; j += 2) { // the equation at the start of page 4, in the paper Felzenszwalb CVPR'04
				msg(msgUp[levelToRun][i+1][j],		msgLeft[levelToRun][i][j+1],	msgRight[levelToRun][i][j-1],	dataCosts[levelToRun][i][j],	msgUp[levelToRun][i][j],	nDisparities,	discCostTruncate); // msgUp[levelToRun][i+1][j] means 
				msg(msgDown[levelToRun][i-1][j],	msgLeft[levelToRun][i][j+1],	msgRight[levelToRun][i][j-1],	dataCosts[levelToRun][i][j],	msgDown[levelToRun][i][j],	nDisparities,	discCostTruncate); //		the message is sent from 
				msg(msgUp[levelToRun][i+1][j],		msgDown[levelToRun][i-1][j],	msgRight[levelToRun][i][j-1],	dataCosts[levelToRun][i][j],	msgRight[levelToRun][i][j], nDisparities,	discCostTruncate); //		(x, y+1)=(j, i+1) to the 
				msg(msgUp[levelToRun][i+1][j],		msgDown[levelToRun][i-1][j],	msgLeft[levelToRun][i][j+1],	dataCosts[levelToRun][i][j],	msgLeft[levelToRun][i][j],	nDisparities,	discCostTruncate); //		up position
			}
		}
	}
}


void ANCCwithBP::msg(double* s1, double* s2, double* s3, double* s4, double* dst,
					 int nDisparities, double discCostTruncate) { // compute message

	// aggregate and find max  
	double maximum = MINUS_INFINITIVE; // -infinitive (small cost)
	for (int d = 0; d < nDisparities; d++) {
		dst[d] = s1[d] + s2[d] + s3[d] + s4[d];
		if (maximum < dst[d])
			maximum = dst[d];
	}

	// distance transform
	dt(dst, nDisparities); // two for-loops (equation the start of the 2nd column in page 3, in the paper Felzenszwalb CVPR'04)

	// truncate 
	maximum -= discCostTruncate; // DISC_K;
	for (int d = 0; d < nDisparities; d++) 
		if (maximum > dst[d])
			dst[d] = maximum; // the equation at the end of page 3, in the paper Felzenszwalb CVPR'04

	// normalize
	double val = 0;				for(int d = 0; d < nDisparities; d++)  val += dst[d];
	val /= nDisparities*1.0;	for(int d = 0; d < nDisparities; d++)  dst[d] -= val;
}


void ANCCwithBP::dt(double* f, int nDisparities) {	// dt of 1d function (dt: distance transform)
	double val = 0.1;								// <--- PARAM
	for (int q = 1; q < nDisparities; q++) {
		double prev = f[q-1] - val;
		if (prev > f[q])
			f[q] = prev;
	}
	for (int q = nDisparities - 2; q >= 0; q--) {
		double prev = f[q+1] - val;
		if (prev > f[q])
			f[q] = prev;
	}
}


// ----------------------------------------------------------------------------------------------


void ANCCwithBP::initPyramidBPdataCosts(double**** dataCosts, int* dataCostsWidth, int* dataCostsHeight,
										int nPyramidLevels, int imgH, int imgW, int nDisparities) {
	for(int l = 1; l < nPyramidLevels; l++) {
		int oldImgW = dataCostsWidth[l-1];
		int oldImgH = dataCostsHeight[l-1];
		int newImgW = (int) ceil(oldImgW / 2.0);		dataCostsWidth[l] = newImgW;
		int newImgH = (int) ceil(oldImgH / 2.0);		dataCostsHeight[l] = newImgH;		

		dataCosts[l] = new double**[newImgH];
		for(int i = 0; i < oldImgH; i++) {
			dataCosts[l][i/2] = new double*[newImgW];
			for(int j = 0; j < oldImgW; j++) {
				dataCosts[l][i/2][j/2] = new double[nDisparities];
				for(int d = 0; d < nDisparities; d++) {
					dataCosts[l][i/2][j/2][d] += dataCosts[l-1][i][j][d]; // equation (6) in the paper Felzenszwalb CVPR'04
				}
			}
		}
	}	
}


void ANCCwithBP::initTheFinestLevelOfBPdataCostsUsingANCC(IplImage *leftImg, IplImage *rightImg, double**** dataCosts, 
														int disparityRangeTo, int disparityRangeFrom,
														int winSize, double sigmaDistance, double sigmaColor,
														double dataCostTruncate, double dataCostWeighting) {
	//CStdioFile dataCostsFile;
	//dataCostsFile.Open(L"dataCosts.txt", CFile::modeCreate | CFile::modeWrite);
	//CString text;


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

	dataCosts[0] = new double**[endY - startY + 1];													// <---

	for(int i = startY; i <= endY; i++) {

		dataCosts[0][i-startY] = new double*[endX - startX + 1];									// <---

		for(int j = startX; j <= endX; j++) {

			dataCosts[0][i-startY][j-startX] = new double[disparityRangeTo - disparityRangeFrom];	// <---

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
				dataCosts[0][i-startY][j-startX][d-disparityRangeFrom] = dataCostWeighting * MAX(totalOfAncc, dataCostTruncate);


				//
				//if(d == disparityRangeFrom) {
				//	text.Format(L"%1.3f ", totalOfAncc);
				//	dataCostsFile.WriteString(text);
				//}
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
