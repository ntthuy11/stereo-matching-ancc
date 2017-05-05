#pragma once

#include "cv.h"
#include "highgui.h"


class Util
{
public:
	Util(void);
public:
	~Util(void);


	// ============================================ GENERAL PURPOSE ============================================


	// used to make the disparity image having better view
	static int calcDisparityMultiplier(int nDisparities) {
		return 256/nDisparities;
	}


	// to find the difference in distance
	static double euclideanDist2D(CvPoint p1, CvPoint p2) {
		int x1x2 = p1.x - p2.x;
		int y1y2 = p1.y - p2.y;
		return sqrt( (x1x2*x1x2 + y1y2*y1y2) * 1.0 );
	}


	// to find the difference in 3-channel color
	static double euclideanDist3D(CvPoint3D32f p1, CvPoint3D32f p2) {
		double x1x2 = p1.x - p2.x;
		double y1y2 = p1.y - p2.y;
		double z1z2 = p1.z - p2.z;
		return sqrt(x1x2*x1x2 + y1y2*y1y2 + z1z2*z1z2);
	}


	// get a 3-channel color at a specific position in the an image
	static CvPoint3D32f getColor(uchar* imgData, int imgWidthStep, int imgNChannels, int iRow, int jColumn) {
		int currPosColor = iRow*imgWidthStep + jColumn*imgNChannels;
		int leftCenterBlue = imgData[currPosColor];
		int leftCenterGreen = imgData[currPosColor + 1];
		int leftCenterRed = imgData[currPosColor + 2];
		return cvPoint3D32f((double) leftCenterBlue, (double) leftCenterGreen, (double) leftCenterRed);
	}


	// normalize
	static void normalizeWeights(double *listWeights, double *normalizedListWeights, int listSize) {
		double sumWeight = 0;

		// take sum of weights
		for(int i = 0; i < listSize; i++)
			sumWeight += listWeights[i];

		// normalize
		for(int i = 0; i < listSize; i++)
			normalizedListWeights[i] = listWeights[i] / sumWeight;
	}


	// add dummy values to 2 remaining channels of a gray image
	static void addDummyValuesToGrayLeftAndRightImages(IplImage* leftImg, IplImage* rightImg, 
													   IplImage* leftImgWith2DummyChannels, IplImage* rightImgWith2DummyChannels,
													   int dummyValue) {		
		// for dummy images
		int imgH = leftImgWith2DummyChannels->height;
		int imgW = leftImgWith2DummyChannels->width;
		int imgS = leftImgWith2DummyChannels->widthStep;
		int imgC = leftImgWith2DummyChannels->nChannels;
		uchar* leftImgWith2DummyChannelsData = (uchar *) leftImgWith2DummyChannels->imageData;
		uchar* rightImgWith2DummyChannelsData = (uchar *) rightImgWith2DummyChannels->imageData;
		
		// for original gray images
		int imgSorg = leftImg->widthStep;
		int imgCorg = leftImg->nChannels;
		uchar* leftImgData = (uchar *) leftImg->imageData;
		uchar* rightImgData = (uchar *) rightImg->imageData;

		// run
		for(int i = 0; i < leftImg->height; i++) {
			for(int j = 0; j < leftImg->width; j++) {
				int pos = i*imgS + j*imgC;
				int posOrg = i*imgSorg + j*imgCorg;
				leftImgWith2DummyChannelsData[pos] = leftImgData[posOrg];		leftImgWith2DummyChannelsData[pos + 1] = dummyValue;		leftImgWith2DummyChannelsData[pos + 2] = dummyValue*2;
				rightImgWith2DummyChannelsData[pos] = rightImgData[posOrg];		rightImgWith2DummyChannelsData[pos + 1] = dummyValue;		rightImgWith2DummyChannelsData[pos + 2] = dummyValue*2;
			}
		}
	}


	// ============================================ ANCC PURPOSE ============================================


	// calculate the bilater filter based weight for every pixel inside a window (having a center)
	static double calcWeightOfPixelInsideWin(CvPoint aPixelPos, CvPoint centerPixelPos, 
											CvPoint3D32f aPixelColor, CvPoint3D32f centerPixelColor,
											double sigmaDistance,
											double sigmaColor) {
		double distanceComponent = Util::euclideanDist2D(aPixelPos, centerPixelPos) / (2 * sigmaDistance * sigmaDistance); 
		double colorComponent = Util::euclideanDist3D(aPixelColor, centerPixelColor) / (2 * sigmaColor * sigmaColor);
		double weight = exp(- distanceComponent - colorComponent);
		return weight;
	}


	// calculate the chromaticity normalization coefficient
	static double calcChromNormCoeff(double color1Channel, CvPoint3D32f color3Channels) {
		return color1Channel / pow(color3Channels.x * color3Channels.y * color3Channels.z, 1*0/3);
	}


	// calculate weights and chromaticity normalization coefficients in advance
	static void calcWeightsAndChromNormCoeffs(int halfWinSize, int i, int j, int d,
											 uchar* leftImgData, uchar* rightImgData, int imgS, int imgC, 
											 CvPoint left_centerPixelPos, CvPoint right_centerPixelPos, 
											 CvPoint3D32f left_centerPixelColor, CvPoint3D32f right_centerPixelColor,
											 double sigmaDistance, double sigmaColor,
											 double *list_left_aPixelWeight, double *list_right_aPixelWeight,
											 double *list_left_aPixelChromNormCoeff_blue, double *list_left_aPixelChromNormCoeff_green, double *list_left_aPixelChromNormCoeff_red,
											 double *list_right_aPixelChromNormCoeff_blue, double *list_right_aPixelChromNormCoeff_green, double *list_right_aPixelChromNormCoeff_red) {
		int count = 0;	

		// scan according to the window
		for(int k = j - halfWinSize; k <= j + halfWinSize; k++) {

			int kPlusD = k + d;

			// get a pixel's poistion and color
			CvPoint left_aPixelPos = cvPoint(kPlusD, i);
			CvPoint right_aPixelPos = cvPoint(k, i);

			CvPoint3D32f left_aPixelColor = Util::getColor(leftImgData, imgS, imgC, i, kPlusD);
			CvPoint3D32f right_aPixelColor = Util::getColor(rightImgData, imgS, imgC, i, k);

			// calculate the weight for every pixel in the window
			list_left_aPixelWeight[count] = Util::calcWeightOfPixelInsideWin(left_aPixelPos, left_centerPixelPos, 
																		left_aPixelColor, left_centerPixelColor, 
																		sigmaDistance, sigmaColor);
			list_right_aPixelWeight[count] = Util::calcWeightOfPixelInsideWin(right_aPixelPos, right_centerPixelPos, 
																		right_aPixelColor, right_centerPixelColor, 
																		sigmaDistance, sigmaColor);

			// calculate the chromaticity normalization coefficients
			list_left_aPixelChromNormCoeff_blue[count]	= Util::calcChromNormCoeff(left_aPixelColor.x, left_aPixelColor);
			list_left_aPixelChromNormCoeff_green[count] = Util::calcChromNormCoeff(left_aPixelColor.y, left_aPixelColor);
			list_left_aPixelChromNormCoeff_red[count]	= Util::calcChromNormCoeff(left_aPixelColor.z, left_aPixelColor);

			list_right_aPixelChromNormCoeff_blue[count]	= Util::calcChromNormCoeff(right_aPixelColor.x, right_aPixelColor);
			list_right_aPixelChromNormCoeff_green[count]= Util::calcChromNormCoeff(right_aPixelColor.y, right_aPixelColor);
			list_right_aPixelChromNormCoeff_red[count]	= Util::calcChromNormCoeff(right_aPixelColor.z, right_aPixelColor);

			//
			count++;
		}
	}


	// calculate the chromaticity normalization coefficients with weights
	static double calcChromNormCoeffWithWeight(double *list_aPixelWeight_normalized, double *list_aPixelChromNormCoeff, int listSize) {
		double sum = 0;
		for(int i = 0; i < listSize; i++)
			sum += list_aPixelWeight_normalized[i] * list_aPixelChromNormCoeff[i];
		return sum;
	}


	// calculate the complete ANCC (adaptive normalized cross-correlation) value
	static double calcANCC(double *list_left_aPixelWeight, double *list_right_aPixelWeight,
							double *list_left_aPixelChromNormCoeff, double *list_right_aPixelChromNormCoeff,
							double left_chromNormCoeffWithWeight, double right_chromNormCoeffWithWeight,
							int listSize) {	
		double numerator = 0;
		double approxHalfOfDenominator1 = 0;
		double approxHalfOfDenominator2 = 0;

		for(int i = 0; i < listSize; i++) {
			double tmpL = list_left_aPixelChromNormCoeff[i] - left_chromNormCoeffWithWeight;
			double tmpR = list_right_aPixelChromNormCoeff[i] - right_chromNormCoeffWithWeight;

			numerator += list_left_aPixelWeight[i] * list_right_aPixelWeight[i] * tmpL * tmpR;

			approxHalfOfDenominator1 += pow(list_left_aPixelWeight[i] * tmpL, 2.0);
			approxHalfOfDenominator2 += pow(list_right_aPixelWeight[i] * tmpR, 2.0);
		}	
	
		double denominator = (sqrt(approxHalfOfDenominator1) * sqrt(approxHalfOfDenominator2));
		if(denominator == 0) 
			return 0;
		else
			return numerator / denominator;
	}
};
