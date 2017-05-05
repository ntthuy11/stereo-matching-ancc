#include "StdAfx.h"
#include "MyTimer.h"


MyTimer::MyTimer(void) { }
MyTimer::~MyTimer(void) { }


void MyTimer::opencvStart() {
	timeStart = cvGetTickCount();
}


int MyTimer::opencvStop() {
	timeStop = cvGetTickCount();
	return (int) ( (timeStop - timeStart) / (cvGetTickFrequency() * 1000) );
}
