/*	
	Implement the idea of using Adaptive Normalized Cross-Correlation (ANCC) in the paper:
	Y. S. Heo, K. M. Lee, and S. U. Lee, "Robust Stereo Matching using ANCC," PAMI 2011

	The result of ANCC is then optimized by using Belief Propagation (BP):
	P. Felzenszwalb and D. Huttenlocher, "Efficient BP for early vision," IJCV 2006
*/


#include "StdAfx.h"
#include "ANCConly.h"


ANCConly::ANCConly(void) { }
ANCConly::~ANCConly(void) { }


void ANCConly::match(IplImage *leftImg, IplImage *rightImg, IplImage* disparityImg, 
					int disparityRangeFrom, int disparityRangeTo, 
					int winSize, double sigmaDistance, double sigmaColor) {
	
	// images' info
	uchar* leftImgData = (uchar *) leftImg->imageData;
	uchar* rightImgData = (uchar *) rightImg->imageData;	
	int imgS = leftImg->widthStep;
	int imgC = leftImg->nChannels;
	int imgW = leftImg->width;
	int imgH = leftImg->height;

	uchar* disparityImgData = (uchar *) disparityImg->imageData;
	int disparityImgStep = disparityImg->widthStep;

	int disparityMultiplier = Util::calcDisparityMultiplier(disparityRangeTo - disparityRangeFrom);


	// scanning info
	int halfWinSize = winSize/2;
	int startX = halfWinSize;			int endX = imgW - halfWinSize;
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

	for(int i = startY; i <= endY; i++) {
		for(int j = startX; j <= endX; j++) {

			double bestAncc = -1E20;
			int bestDisparity = -1;

			for(int d = disparityRangeFrom; d <= disparityRangeTo; d++) {

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

				// find which ANCC value is the best one, and the disparity as well
				if(bestAncc < totalOfAncc) {
					bestAncc = totalOfAncc;
					bestDisparity = d;
				}
			}

			// assign the best disparity to disparityImg
			disparityImgData[i*disparityImgStep + j] = bestDisparity * disparityMultiplier;
		}
	}


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
