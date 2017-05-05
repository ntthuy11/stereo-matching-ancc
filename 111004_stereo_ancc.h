
// 111004_stereo_ancc.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMy111004_stereo_anccApp:
// See 111004_stereo_ancc.cpp for the implementation of this class
//

class CMy111004_stereo_anccApp : public CWinApp
{
public:
	CMy111004_stereo_anccApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMy111004_stereo_anccApp theApp;