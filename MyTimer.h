#pragma once


#include "cv.h"
#include "highgui.h"


class MyTimer
{
public:
	MyTimer(void);
	~MyTimer(void);

	void opencvStart();
	int opencvStop();


private:
	int64 timeStart, timeStop;
};
