
// 111004_stereo_anccDlg.h : header file
//

#pragma once

#include "cv.h"
#include "highgui.h"

#include "MyTimer.h"
#include "ANCConly.h"
#include "ANCCwithBP.h"
#include "ANCCwithCSBP.h"


// ---------------------------------------------


#define WIN_LEFT_IMG						"Left image"
#define WIN_RIGHT_IMG						"Right image"
#define WIN_DISPARITY_IMG_ANCC_ONLY			"Disparity image - ANCC only"
#define WIN_DISPARITY_IMG_ANCC_BP			"Disparity image - ANCC+BP"
#define WIN_DISPARITY_IMG_ANCC_CSBP			"Disparity image - ANCC+CSBP"

#define WIN_DISPARITY_IMG_ANCC_ONLY_SMOOTH	"Disparity image - ANCC only (smooth)"
#define WIN_DISPARITY_IMG_ANCC_BP_SMOOTH	"Disparity image - ANCC+BP (smooth)"

#define WIN_DISPARITY_IMG_ANCC_ONLY_LAB		"Disparity image - ANCC only - CIELab"
#define WIN_DISPARITY_IMG_ANCC_BP_LAB		"Disparity image - ANCC+BP - CIELab"

#define WIN_DISPARITY_IMG_ANCC_ONLY_DUMMY_CHANNELS	"Disparity image - ANCC only - Dummy channels"
#define WIN_DISPARITY_IMG_ANCC_BP_DUMMY_CHANNELS	"Disparity image - ANCC+BP - Dummy channels"


// ---------------------------------------------


#define DISPARITY_RANGE_FROM	0
#define DISPARITY_RANGE_TO		70

#define MATCHING_WIN_SIZE		21	//31
#define SIGMA_DISTANCE			10	//14
#define SIGMA_COLOR				3.8

#define BP_N_PYRAMID_LEVELS		4	//5
#define BP_N_ITER				5
#define BP_DISC_COST_TRUNCATE	5.0
#define BP_DATA_COST_TRUNCATE	0.05
#define BP_DATA_COST_WEIGHTING	1.0

#define DUMMY_VALUE				50


// ---------------------------------------------


// CMy111004_stereo_anccDlg dialog
class CMy111004_stereo_anccDlg : public CDialogEx
{
// Construction
public:
	CMy111004_stereo_anccDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MY111004_STEREO_ANCC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonLoadImg();

	afx_msg void OnBnClickedButtonAnccOnly();
	afx_msg void OnBnClickedButtonAnccOnlySmooth();

	afx_msg void OnBnClickedButtonAnccBp();
	afx_msg void OnBnClickedButtonAnccBpSmooth();

	afx_msg void OnBnClickedButtonAnccOnlyLab();
	afx_msg void OnBnClickedButtonAnccBpLab();

	afx_msg void OnBnClickedButtonAnccOnlyDummyChannels();
	afx_msg void OnBnClickedButtonAnccBpDummyChannels();

	afx_msg void OnBnClickedButtonAnccCsbp();


private:
	void initVar();
	void initGUI();
	void getInputParam();
	void displayResult();


private:
	MyTimer myTimer;
	ANCConly anccOnly;
	ANCCwithBP anccWithBP;
	ANCCwithCSBP anccWithCSBP;


	// ----- util -----
	char charArray[256];
	bool isLoadImg;
	int processingTime;


	// ----- input parameters -----
	int disparityRangeFrom, disparityRangeTo, matchingWinSize, bpNPyramidLevels, bpNIter;
	double sigmaDistance, sigmaColor, bpDiscCostTruncate, bpDataCostTruncate, bpDataCostWeighting;


	// ----- images -----
	IplImage *leftImg, *rightImg, *disparityImg;
	IplImage *leftImgSmooth, *rightImgSmooth;	
	IplImage *leftImgLab, *rightImgLab;
	IplImage *leftImgWith2DummyChannels, *rightImgWith2DummyChannels;
};
