#pragma once

#include "cv.h"
#include "highgui.h"

#include "Util.h"


class ANCConly
{
public:
	ANCConly(void);
	~ANCConly(void);

	void match(IplImage *leftImg, IplImage *rightImg, IplImage* disparityImg, 
				int disparityRangeFrom, int disparityRangeTo,
				int winSize, double sigmaDistance, double sigmaColor);
};
