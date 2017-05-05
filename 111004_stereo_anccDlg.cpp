
// 111004_stereo_anccDlg.cpp : implementation file
//

#include "stdafx.h"
#include "111004_stereo_ancc.h"
#include "111004_stereo_anccDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMy111004_stereo_anccDlg dialog




CMy111004_stereo_anccDlg::CMy111004_stereo_anccDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMy111004_stereo_anccDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMy111004_stereo_anccDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMy111004_stereo_anccDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_LOAD_IMG, &CMy111004_stereo_anccDlg::OnBnClickedButtonLoadImg)
	ON_BN_CLICKED(IDC_BUTTON_ANCC_ONLY, &CMy111004_stereo_anccDlg::OnBnClickedButtonAnccOnly)
	ON_BN_CLICKED(IDC_BUTTON_ANCC_BP, &CMy111004_stereo_anccDlg::OnBnClickedButtonAnccBp)
	ON_BN_CLICKED(IDC_BUTTON_ANCC_ONLY_SMOOTH, &CMy111004_stereo_anccDlg::OnBnClickedButtonAnccOnlySmooth)
	ON_BN_CLICKED(IDC_BUTTON_ANCC_BP_SMOOTH, &CMy111004_stereo_anccDlg::OnBnClickedButtonAnccBpSmooth)
	ON_BN_CLICKED(IDC_BUTTON_ANCC_ONLY_LAB, &CMy111004_stereo_anccDlg::OnBnClickedButtonAnccOnlyLab)
	ON_BN_CLICKED(IDC_BUTTON_ANCC_BP_LAB, &CMy111004_stereo_anccDlg::OnBnClickedButtonAnccBpLab)
	ON_BN_CLICKED(IDC_BUTTON_ANCC_ONLY_DUMMY_CHANNELS, &CMy111004_stereo_anccDlg::OnBnClickedButtonAnccOnlyDummyChannels)
	ON_BN_CLICKED(IDC_BUTTON_ANCC_BP_DUMMY_CHANNELS, &CMy111004_stereo_anccDlg::OnBnClickedButtonAnccBpDummyChannels)
	ON_BN_CLICKED(IDC_BUTTON_ANCC_CSBP, &CMy111004_stereo_anccDlg::OnBnClickedButtonAnccCsbp)
END_MESSAGE_MAP()


// CMy111004_stereo_anccDlg message handlers

BOOL CMy111004_stereo_anccDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	initVar();
	initGUI();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMy111004_stereo_anccDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMy111004_stereo_anccDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// ==========================================================================================


void CMy111004_stereo_anccDlg::OnClose() {
	cvDestroyAllWindows();	
	if(isLoadImg) {
		cvReleaseImage(&leftImg);
		cvReleaseImage(&rightImg);
		//cvReleaseImage(&disparityImg);
	}
	CDialogEx::OnClose();
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonLoadImg() {
	CFileDialog dlg(TRUE, _T("*.txt"), _T(""), OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY, _T("Image list (*.txt)|*.txt|All Files (*.*)|*.*||"), NULL);

	if (dlg.DoModal() == IDOK) {		
		CStdioFile imgSeqFile;
		imgSeqFile.Open((LPCTSTR)dlg.GetPathName(), CFile::modeRead);

		CString imgFn;

		imgSeqFile.ReadString(imgFn);		WideCharToMultiByte(CP_ACP, 0, imgFn, -1, charArray, 256, NULL, NULL); 		leftImg = cvLoadImage(charArray); 		cvShowImage(WIN_LEFT_IMG, leftImg);
		imgSeqFile.ReadString(imgFn);		WideCharToMultiByte(CP_ACP, 0, imgFn, -1, charArray, 256, NULL, NULL); 		rightImg = cvLoadImage(charArray); 		cvShowImage(WIN_RIGHT_IMG, rightImg);

		imgSeqFile.Close();
	}
	
	isLoadImg = true;	
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonAnccOnly() {

	// ----- init -----
	getInputParam();

	disparityImg = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 1);
	cvZero(disparityImg);


	// ----- run -----
	myTimer.opencvStart();

	anccOnly.match(leftImg, rightImg, disparityImg, 
					disparityRangeFrom, disparityRangeTo, 
					matchingWinSize, sigmaDistance, sigmaColor);

	processingTime = myTimer.opencvStop();


	// ----- display -----
	displayResult();
	cvShowImage(WIN_DISPARITY_IMG_ANCC_ONLY, disparityImg);


	// ----- release -----
	cvReleaseImage(&disparityImg);
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonAnccOnlySmooth() {

	// ----- init -----
	getInputParam();

	disparityImg = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 1);
	leftImgSmooth = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	rightImgSmooth = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	cvZero(disparityImg);


	// ----- run -----
	myTimer.opencvStart();

	//cvSmooth(leftImg, leftImgSmooth);
	cvSmooth(leftImg, leftImgSmooth, CV_BILATERAL, 31, 0, 3.8, 14);
	//cvSmooth(rightImg, rightImgSmooth);
	cvSmooth(rightImg, rightImgSmooth, CV_BILATERAL, 31, 0, 3.8, 14);
	
	anccOnly.match(leftImgSmooth, rightImgSmooth, disparityImg, 
					disparityRangeFrom, disparityRangeTo, 
					matchingWinSize, sigmaDistance, sigmaColor);

	processingTime = myTimer.opencvStop();


	// ----- display -----
	displayResult();
	cvShowImage(WIN_LEFT_IMG, leftImgSmooth);
	cvShowImage(WIN_RIGHT_IMG, rightImgSmooth);
	cvShowImage(WIN_DISPARITY_IMG_ANCC_ONLY_SMOOTH, disparityImg);


	// ----- release -----
	cvReleaseImage(&leftImgSmooth);
	cvReleaseImage(&rightImgSmooth);
	cvReleaseImage(&disparityImg);
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonAnccBp() {

	// ----- init -----
	getInputParam();

	disparityImg = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 1);
	cvZero(disparityImg);


	// ----- run -----
	myTimer.opencvStart();

	anccWithBP.match(leftImg, rightImg, disparityImg, 
					disparityRangeFrom, disparityRangeTo, 
					matchingWinSize, sigmaDistance, sigmaColor,
					bpNPyramidLevels, bpNIter, bpDiscCostTruncate, bpDataCostTruncate, bpDataCostWeighting);

	processingTime = myTimer.opencvStop();


	// ----- display -----
	displayResult();
	cvShowImage(WIN_DISPARITY_IMG_ANCC_BP, disparityImg);


	// ----- release -----
	cvReleaseImage(&disparityImg);
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonAnccBpSmooth() {

	// ----- init -----
	getInputParam();

	disparityImg = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 1);
	leftImgSmooth = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	rightImgSmooth = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	cvZero(disparityImg);


	// ----- run -----
	myTimer.opencvStart();

	//cvSmooth(leftImg, leftImgSmooth);
	cvSmooth(leftImg, leftImgSmooth, CV_BILATERAL, 31, 0, 3.8, 14);
	//cvSmooth(rightImg, rightImgSmooth);
	cvSmooth(rightImg, rightImgSmooth, CV_BILATERAL, 31, 0, 3.8, 14);

	anccWithBP.match(leftImgSmooth, rightImgSmooth, disparityImg, 
					disparityRangeFrom, disparityRangeTo, 
					matchingWinSize, sigmaDistance, sigmaColor,
					bpNPyramidLevels, bpNIter, bpDiscCostTruncate, bpDataCostTruncate, bpDataCostWeighting);

	processingTime = myTimer.opencvStop();


	// ----- display -----
	displayResult();
	cvShowImage(WIN_LEFT_IMG, leftImgSmooth);
	cvShowImage(WIN_RIGHT_IMG, rightImgSmooth);
	cvShowImage(WIN_DISPARITY_IMG_ANCC_BP_SMOOTH, disparityImg);


	// ----- release -----
	cvReleaseImage(&leftImgSmooth);
	cvReleaseImage(&rightImgSmooth);
	cvReleaseImage(&disparityImg);
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonAnccOnlyLab() {

	// ----- init -----
	getInputParam();

	disparityImg = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 1);
	leftImgLab = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	rightImgLab = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	cvZero(disparityImg);


	// ----- run -----
	myTimer.opencvStart();

	cvCvtColor(leftImg, leftImgLab, CV_RGB2Lab);
	cvCvtColor(rightImg, rightImgLab, CV_RGB2Lab);

	anccOnly.match(leftImgLab, rightImgLab, disparityImg, 
					disparityRangeFrom, disparityRangeTo, 
					matchingWinSize, sigmaDistance, sigmaColor);

	processingTime = myTimer.opencvStop();


	// ----- display -----
	displayResult();
	cvShowImage(WIN_LEFT_IMG, leftImgLab);
	cvShowImage(WIN_RIGHT_IMG, rightImgLab);
	cvShowImage(WIN_DISPARITY_IMG_ANCC_ONLY_LAB, disparityImg);


	// ----- release -----
	cvReleaseImage(&leftImgLab);
	cvReleaseImage(&rightImgLab);
	cvReleaseImage(&disparityImg);
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonAnccBpLab() {
	
	// ----- init -----
	getInputParam();

	disparityImg = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 1);
	leftImgLab = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	rightImgLab = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	cvZero(disparityImg);


	// ----- run -----
	myTimer.opencvStart();

	cvCvtColor(leftImg, leftImgLab, CV_RGB2Lab);		cvShowImage(WIN_LEFT_IMG, leftImgLab);
	cvCvtColor(rightImg, rightImgLab, CV_RGB2Lab);		cvShowImage(WIN_RIGHT_IMG, rightImgLab);

	anccWithBP.match(leftImgLab, rightImgLab, disparityImg, 
					disparityRangeFrom, disparityRangeTo, 
					matchingWinSize, sigmaDistance, sigmaColor,
					bpNPyramidLevels, bpNIter, bpDiscCostTruncate, bpDataCostTruncate, bpDataCostWeighting);

	processingTime = myTimer.opencvStop();


	// ----- display -----
	displayResult();
	cvShowImage(WIN_DISPARITY_IMG_ANCC_BP_LAB, disparityImg);


	// ----- release -----
	cvReleaseImage(&leftImgLab);
	cvReleaseImage(&rightImgLab);
	cvReleaseImage(&disparityImg);
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonAnccOnlyDummyChannels() {
	
	// we have to load gray images first


	// ----- init -----
	//getInputParam();

	disparityImg = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 1);
	leftImgWith2DummyChannels = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	rightImgWith2DummyChannels = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	cvZero(disparityImg);


	// ----- run -----
	myTimer.opencvStart();

	Util::addDummyValuesToGrayLeftAndRightImages(leftImg, rightImg, leftImgWith2DummyChannels, rightImgWith2DummyChannels, DUMMY_VALUE);
	anccOnly.match(leftImgWith2DummyChannels, rightImgWith2DummyChannels, disparityImg, 
					disparityRangeFrom, disparityRangeTo, 
					matchingWinSize, sigmaDistance, sigmaColor);
	
	processingTime = myTimer.opencvStop();


	// ----- display -----
	displayResult();
	cvShowImage(WIN_LEFT_IMG, leftImgWith2DummyChannels);
	cvShowImage(WIN_RIGHT_IMG, rightImgWith2DummyChannels);
	cvShowImage(WIN_DISPARITY_IMG_ANCC_ONLY_DUMMY_CHANNELS, disparityImg);


	// ----- release -----
	cvReleaseImage(&leftImgWith2DummyChannels);
	cvReleaseImage(&rightImgWith2DummyChannels);
	cvReleaseImage(&disparityImg);
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonAnccBpDummyChannels() {

	// we have to load gray images first


	// ----- init -----
	//getInputParam();

	disparityImg = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 1);
	leftImgWith2DummyChannels = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	rightImgWith2DummyChannels = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 3);
	cvZero(disparityImg);


	// ----- run -----
	myTimer.opencvStart();

	Util::addDummyValuesToGrayLeftAndRightImages(leftImg, rightImg, leftImgWith2DummyChannels, rightImgWith2DummyChannels, DUMMY_VALUE);
	anccWithBP.match(leftImgWith2DummyChannels, rightImgWith2DummyChannels, disparityImg, 
					disparityRangeFrom, disparityRangeTo, 
					matchingWinSize, sigmaDistance, sigmaColor,
					bpNPyramidLevels, bpNIter, bpDiscCostTruncate, bpDataCostTruncate, bpDataCostWeighting);
	
	processingTime = myTimer.opencvStop();


	// ----- display -----
	displayResult();
	cvShowImage(WIN_LEFT_IMG, leftImgWith2DummyChannels);
	cvShowImage(WIN_RIGHT_IMG, rightImgWith2DummyChannels);
	cvShowImage(WIN_DISPARITY_IMG_ANCC_BP_DUMMY_CHANNELS, disparityImg);


	// ----- release -----
	cvReleaseImage(&leftImgWith2DummyChannels);
	cvReleaseImage(&rightImgWith2DummyChannels);
	cvReleaseImage(&disparityImg);
}


void CMy111004_stereo_anccDlg::OnBnClickedButtonAnccCsbp() {

	// ----- init -----
	getInputParam();

	disparityImg = cvCreateImage(cvSize(leftImg->width, leftImg->height), IPL_DEPTH_8U, 1);
	cvZero(disparityImg);


	// ----- run -----
	myTimer.opencvStart();

	anccWithCSBP.match(leftImg, rightImg, disparityImg, 
					disparityRangeFrom, disparityRangeTo, 
					matchingWinSize, sigmaDistance, sigmaColor,
					bpNPyramidLevels, bpNIter, bpDiscCostTruncate, bpDataCostTruncate, bpDataCostWeighting);

	processingTime = myTimer.opencvStop();


	// ----- display -----
	displayResult();
	cvShowImage(WIN_DISPARITY_IMG_ANCC_CSBP, disparityImg);


	// ----- release -----
	cvReleaseImage(&disparityImg);
}


// ==========================================================================================


void CMy111004_stereo_anccDlg::initVar() {
	isLoadImg = false;

	disparityRangeFrom	= DISPARITY_RANGE_FROM;
	disparityRangeTo	= DISPARITY_RANGE_TO;

	matchingWinSize		= MATCHING_WIN_SIZE;
	sigmaDistance		= SIGMA_DISTANCE;
	sigmaColor			= SIGMA_COLOR;

	bpNPyramidLevels	= BP_N_PYRAMID_LEVELS;
	bpNIter				= BP_N_ITER;
	bpDiscCostTruncate	= BP_DISC_COST_TRUNCATE;
	bpDataCostTruncate	= BP_DATA_COST_TRUNCATE;
	bpDataCostWeighting	= BP_DATA_COST_WEIGHTING;
}


void CMy111004_stereo_anccDlg::initGUI() {
	CString s;	

	s.Format(L"%d", disparityRangeFrom);		this->GetDlgItem(IDC_EDIT_DISPARITY_RANGE_FROM)->SetWindowTextW((LPCTSTR)s);
	s.Format(L"%d", disparityRangeTo);			this->GetDlgItem(IDC_EDIT_DISPARITY_RANGE_TO)->SetWindowTextW((LPCTSTR)s);

	s.Format(L"%d", matchingWinSize);			this->GetDlgItem(IDC_EDIT_MATCHING_WIN_SIZE)->SetWindowTextW((LPCTSTR)s);
	s.Format(L"%2.2f", sigmaDistance);			this->GetDlgItem(IDC_EDIT_SIGMA_DISTANCE)->SetWindowTextW((LPCTSTR)s);
	s.Format(L"%2.2f", sigmaColor);				this->GetDlgItem(IDC_EDIT_SIGMA_COLOR)->SetWindowTextW((LPCTSTR)s);

	s.Format(L"%d", bpNPyramidLevels);			this->GetDlgItem(IDC_EDIT_BP_N_PYRAMID_LEVELS)->SetWindowTextW((LPCTSTR)s);
	s.Format(L"%d", bpNIter);					this->GetDlgItem(IDC_EDIT_BP_N_ITER)->SetWindowTextW((LPCTSTR)s);
	s.Format(L"%2.2f", bpDiscCostTruncate);		this->GetDlgItem(IDC_EDIT_BP_DISC_COST_TRUNCATE)->SetWindowTextW((LPCTSTR)s);
	s.Format(L"%2.2f", bpDataCostTruncate);		this->GetDlgItem(IDC_EDIT_BP_DATA_COST_TRUNCATE)->SetWindowTextW((LPCTSTR)s);
	s.Format(L"%2.2f", bpDataCostWeighting);	this->GetDlgItem(IDC_EDIT_BP_DATA_COST_WEIGHTING)->SetWindowTextW((LPCTSTR)s);
}


void CMy111004_stereo_anccDlg::getInputParam() {
	CString s;

	this->GetDlgItem(IDC_EDIT_DISPARITY_RANGE_FROM)->GetWindowTextW(s);			if (!s.IsEmpty())	disparityRangeFrom = (int)wcstod(s, NULL);
	this->GetDlgItem(IDC_EDIT_DISPARITY_RANGE_TO)->GetWindowTextW(s);			if (!s.IsEmpty())	disparityRangeTo = (int)wcstod(s, NULL);

	this->GetDlgItem(IDC_EDIT_MATCHING_WIN_SIZE)->GetWindowTextW(s);			if (!s.IsEmpty())	matchingWinSize = (int)wcstod(s, NULL);
	this->GetDlgItem(IDC_EDIT_SIGMA_DISTANCE)->GetWindowTextW(s);				if (!s.IsEmpty())	sigmaDistance = wcstod(s, NULL);
	this->GetDlgItem(IDC_EDIT_SIGMA_COLOR)->GetWindowTextW(s);					if (!s.IsEmpty())	sigmaColor = wcstod(s, NULL);

	this->GetDlgItem(IDC_EDIT_BP_N_PYRAMID_LEVELS)->GetWindowTextW(s);			if (!s.IsEmpty())	bpNPyramidLevels = (int)wcstod(s, NULL);
	this->GetDlgItem(IDC_EDIT_BP_N_ITER)->GetWindowTextW(s);					if (!s.IsEmpty())	bpNIter = (int)wcstod(s, NULL);
	this->GetDlgItem(IDC_EDIT_BP_DISC_COST_TRUNCATE)->GetWindowTextW(s);		if (!s.IsEmpty())	bpDiscCostTruncate = wcstod(s, NULL);
	this->GetDlgItem(IDC_EDIT_BP_DATA_COST_TRUNCATE)->GetWindowTextW(s);		if (!s.IsEmpty())	bpDataCostTruncate = wcstod(s, NULL);
	this->GetDlgItem(IDC_EDIT_BP_DATA_COST_WEIGHTING)->GetWindowTextW(s);		if (!s.IsEmpty())	bpDataCostWeighting = wcstod(s, NULL);
}


void CMy111004_stereo_anccDlg::displayResult() {
	CString s;
	s.Format(L"%d", processingTime);		GetDlgItem(IDC_STATIC_TIME)->SetWindowTextW((LPCTSTR)s);
}
