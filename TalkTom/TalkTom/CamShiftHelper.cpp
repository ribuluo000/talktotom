#include "stdafx.h"
#include "CamshiftHelper.h"


CCamShiftHelper::CCamShiftHelper()
{
	m_MouseState = LBUTTON_UP;
	m_bSaveTarget = false;
	m_bBeginTrack = false;
	m_bInitial = false;

	m_hdims = 48;     // 划分HIST的个数，越高越精确
	m_hranges_arr[0] = 0;
	m_hranges_arr[1] = 180;
	m_hranges = m_hranges_arr;

	m_vmin = 10;
	m_vmax = 256;
	m_smin = 30;

	m_image = NULL;
	m_hsv = NULL;
	m_hue = NULL;
	m_mask = NULL;
	m_backproject = NULL;
	m_hist = NULL;  // 计算直方图

	m_track_box.center.x = 0;
	m_track_box.center.y = 0;
	m_track_box.angle = 0.0;

	formerX = formerY = 0;
}



CCamShiftHelper::~CCamShiftHelper()
{
	if (m_image)
	{
		cvReleaseImage(&m_image);
	}

	if (m_hsv)
	{
		cvReleaseImage(&m_hsv);
	}

	if (m_hue)
	{
		cvReleaseImage(&m_hue);
	}

	if (m_mask)
	{
		cvReleaseImage(&m_mask);
	}

	if (m_backproject)
	{
		cvReleaseImage(&m_backproject);
	}

	if (m_hist)
	{
		cvReleaseHist(&m_hist);
	}

	// in case some window is not been closed
	cvDestroyAllWindows();
}


void CCamShiftHelper::_SetOrigin( int origin )
{
	m_image->origin = origin; 
}

void CCamShiftHelper::_ShowTargetArea( IplImage *pOriginFrame )
{

	// if the user has chosen the target area, then show it out
	if (   m_MouseState == LBUTTON_DOWN 
		&& m_selection.width > 0 
		&& m_selection.height > 0)
	{
		cvSetImageROI( pOriginFrame, m_selection );
		cvXorS( pOriginFrame, cvScalarAll(255), pOriginFrame, 0 );
		cvResetImageROI( pOriginFrame );
	}

}

void CCamShiftHelper::_SaveTarget(IplImage *pOriginImage)
{

	if ( m_bSaveTarget == true )
	{
		cvCopy( pOriginImage, m_image, 0 );
		cvCvtColor( pOriginImage, m_hsv, CV_BGR2HSV );  // 彩色空间转换 BGR to HSV 

		cvInRangeS( m_hsv, cvScalar(0,m_smin,MIN(m_vmin,m_vmax),0),
			cvScalar(180,256,MAX(m_vmin, m_vmax),0), m_mask );  // 得到二值的MASK
		cvSplit( m_hsv, m_hue, 0, 0, 0 );  // 只提取 HUE 分量


		float max_val = 0.f;
		cvSetImageROI( m_hue, m_selection );  // 得到选择区域 for ROI
		cvSetImageROI( m_mask, m_selection ); // 得到选择区域 for mask
		cvCalcHist( &m_hue, m_hist, 0, m_mask ); // 计算直方图
		cvGetMinMaxHistValue( m_hist, 0, &max_val, 0, 0 );  // 只找最大值
		cvConvertScale( m_hist->bins, m_hist->bins, max_val ? 255. / max_val : 0., 0 ); // 缩放 bin 到区间 [0,255] 
		cvResetImageROI( m_hue );  // remove ROI
		cvResetImageROI( m_mask );
		m_track_window = m_selection;

		// thus we can begin detect
		m_bBeginTrack = true;

		m_bSaveTarget = false;
	}

}

bool CCamShiftHelper::_Detect( IplImage *pOriginframe, bool bDrawOut )
{
	if (!m_bInitial)
	{
		return false;
	}

	if (m_bBeginTrack == true)
	{
		cvCopy( pOriginframe, m_image, 0 );
		cvCvtColor( m_image, m_hsv, CV_BGR2HSV );  // 彩色空间转换 BGR to HSV 

		cvInRangeS( m_hsv, cvScalar(0,m_smin,MIN(m_vmin,m_vmax),0),
					cvScalar(180,256,MAX(m_vmin,m_vmax),0), m_mask );  // 得到二值的MASK
		cvSplit( m_hsv, m_hue, 0, 0, 0 );  // 只提取 HUE 分量
		cvCalcBackProject( &m_hue, m_backproject, m_hist );  // 使用 back project 方法
		cvAnd( m_backproject, m_mask, m_backproject, 0 );

		// calling CAMSHIFT
		cvCamShift( m_backproject, m_track_window,
			cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
			&m_track_comp, &m_track_box );
		m_track_window = m_track_comp.rect;

		//// test whether the center is near (within 5 pixels) the former point
		//int tempx = m_track_box.center.x;
		//int tempy = m_track_box.center.y;
		//if (abs(m_track_box.center.x - formerX) < 3 && abs(m_track_box.center.y - formerY) < 3 )
		//{

		//	m_track_box.center.x = formerX;
		//	m_track_box.center.x = formerY;
		//}
		//formerX = tempx;
		//formerY = tempy;


		if( m_image->origin )
			m_track_box.angle = -m_track_box.angle;
		
		if (bDrawOut == true)
		{
			cvEllipseBox( m_image, m_track_box, CV_RGB(255,0,0), 3, CV_AA, 0 );
		}
	}

	return true;
}

bool CCamShiftHelper::_ShowAdjustWindow( IplImage *pOriginImage , bool bShowTarget)
{
	if (!m_bInitial)
	{
		return false;
	}

	this->_SetOrigin(pOriginImage->origin);
	cvCopy(pOriginImage, m_image);

	this->_SaveTarget(m_image);
	this->_ShowTargetArea(m_image);
	this->_Detect(m_image, bShowTarget);

	cvShowImage("CamShift Adjust Window", m_image);
	cvWaitKey(2);

	return true;
}

void CCamShiftHelper::_CloseAdjustWindow()
{
	cvDestroyWindow("CamShift Adjust Window");
}

void CCamShiftHelper::_Initialize( int width, int height )
{
	/* allocate all the buffers */
	if (!m_bInitial)
	{
		m_image = cvCreateImage( cvSize(width, height), IPL_DEPTH_8U, 3 );
		m_hsv = cvCreateImage( cvSize(width, height), 8, 3 );
		m_hue = cvCreateImage( cvSize(width, height), 8, 1 );
		m_mask = cvCreateImage( cvSize(width, height), 8, 1 );
		m_backproject = cvCreateImage( cvSize(width, height), 8, 1 );
		m_hist = cvCreateHist( 1, &m_hdims, CV_HIST_ARRAY, &m_hranges, 1 );  // 计算直方图

		m_bInitial = true;
	}
}

// we have to do so, because cvSetMouseCallback function must take global 
// function as its second parameter
void on_mouse( int state, int x, int y, int flags, void* param)
{
	CCamShiftHelper *pHelper = (CCamShiftHelper*)param;

	if (!pHelper->m_bInitial)
	{
		return;
	}

	if( pHelper->m_image->origin )
		y = pHelper->m_image->height - y;

	if( pHelper->m_MouseState )
	{
		pHelper->m_selection.x = MIN(x,pHelper->m_origin.x);
		pHelper->m_selection.y = MIN(y,pHelper->m_origin.y);
		pHelper->m_selection.width = pHelper->m_selection.x + CV_IABS(x - pHelper->m_origin.x);
		pHelper->m_selection.height = pHelper->m_selection.y + CV_IABS(y - pHelper->m_origin.y);

		pHelper->m_selection.x = MAX( pHelper->m_selection.x, 0 );
		pHelper->m_selection.y = MAX( pHelper->m_selection.y, 0 );
		pHelper->m_selection.width = MIN( pHelper->m_selection.width, pHelper->m_image->width );
		pHelper->m_selection.height = MIN( pHelper->m_selection.height, pHelper->m_image->height );
		pHelper->m_selection.width -= pHelper->m_selection.x;
		pHelper->m_selection.height -= pHelper->m_selection.y;

	}

	switch( state )
	{
	case CV_EVENT_LBUTTONDOWN:
		pHelper->m_MouseState = CCamShiftHelper::LBUTTON_DOWN;
		pHelper->m_origin = cvPoint(x,y);
		pHelper->m_selection = cvRect(x,y,0,0);
		break;
	case CV_EVENT_LBUTTONUP:
		pHelper->m_MouseState = CCamShiftHelper::LBUTTON_UP;
		if( pHelper->m_selection.width > 0 && pHelper->m_selection.height > 0 )
		{
			// we have chosen the target then we need to analyze it.
			pHelper->m_bSaveTarget = true;
		}
		break;
	}
}


void CCamShiftHelper::_OpenAdjustWindow()
{
	cvNamedWindow( "CamShift Adjust Window", 1 );
	cvCreateTrackbar( "Vmin", "CamShift Adjust Window", &m_vmin, 256, 0 );
	cvCreateTrackbar( "Vmax", "CamShift Adjust Window", &m_vmax, 256, 0 );
	cvCreateTrackbar( "Smin", "CamShift Adjust Window", &m_smin, 256, 0 );

	// register mouse event
	cvSetMouseCallback( "CamShift Adjust Window", 
		on_mouse,
		this); 
}