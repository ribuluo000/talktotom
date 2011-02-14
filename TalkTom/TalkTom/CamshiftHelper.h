#include "stdafx.h"

#ifndef __CCAMSHIFTHELPER__
#define __CCAMSHIFTHELPER__

#include "cv.h"
#include "highgui.h"


class CCamShiftHelper
{
public:
	CCamShiftHelper();

	~CCamShiftHelper();

public:
	void _Initialize(int width, int height);
	
	void _OpenAdjustWindow();

	bool _ShowAdjustWindow(IplImage *pOriginImage , bool bShowTarget = false);

	void _CloseAdjustWindow();

	bool _Detect(IplImage *pOriginframe, bool bDrawOut = false);


public:
	void _GetImage(IplImage **img){*img = m_image; }

	void _GetDetectedAngle(float *pAngle){*pAngle = m_track_box.angle; }
	
	void _GetDetectedBoxSize(float *pWidth, float *pHeight)
		{*pWidth = m_track_box.size.width; 
		*pHeight = m_track_box.size.height; }
	
	void _GetDetectedCenter(int *pX, int *pY)
		{*pX = (int)m_track_box.center.x; 
		 *pY = (int)m_track_box.center.y; }

private:
	CCamShiftHelper(CCamShiftHelper&){}
	CCamShiftHelper& operator=(CCamShiftHelper&){}

private:
	void _ShowTargetArea(IplImage *pOriginFrame);

	void _SaveTarget(IplImage *pOriginImage);

	void _SetOrigin(int origin);

	//void _ChooseTargetByMouse( int state, int x, int y, int flags, void* param);

public:
	static enum MOUSE_STATE {LBUTTON_UP, LBUTTON_DOWN};

	// we have to do so, because cvSetMouseCallback function must take global 
	// function as its second parameter, and we use "this" pointer to access the 
	// member data
public:
	CvPoint m_origin;
	CvRect m_selection;
	int m_MouseState;
	IplImage *m_image;
	bool m_bInitial;
	bool m_bSaveTarget;


private:

	IplImage  *m_hsv, *m_hue, *m_mask, *m_backproject;
	CvHistogram *m_hist;

	int m_vmin, m_vmax, m_smin;

	float m_hranges_arr[2];
	float* m_hranges;
	bool m_bBeginTrack;
	CvRect m_track_window;
	CvBox2D m_track_box;  // tracking 返回的区域 box，带角度
	CvConnectedComp m_track_comp;
	int m_hdims;     // 划分HIST的个数，越高越精确

private:
	int formerX, formerY;
};

#endif
