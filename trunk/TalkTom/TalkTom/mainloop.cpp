#include "stdafx.h"
#include "mainloop.h"
#include "draw.h"


extern CGlobal g_global;
extern CCamShiftHelper g_camshiftHelper;

void   mainLoop(void)
{
	ARUint8         *dataPtr;
	ARMarkerInfo    *marker_info;
	int             marker_num;
	int             j, k;

	/* grab a vide frame */
	if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
		arUtilSleep(2);
		return;
	}

	// ----------------------
	// by aicro
	g_camshiftHelper._Fill_CV_IplImage(g_global.imgWidth, g_global.imgHeight, (char*)dataPtr);

	g_camshiftHelper._ShowAdjustWindow(true);

	// here we take the center of the picture
	int x, y;
	g_camshiftHelper._GetDetectedCenter(&x, &y);

	// 使用opengl的逆拾取操作
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	GLdouble mvmatrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	GLdouble projmatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
	GLint realy = viewport[3] - (GLint)y;
	//	printf("detected: x = %d	real = %d\n\n", x, realy);

	double wxDir, wyDir, wzDir, wxOri, wyOri, wzOri;
	gluUnProject((GLdouble)x, (GLdouble)realy, 1.0,\
			      mvmatrix, projmatrix, viewport, &wxDir, &wyDir, &wzDir);
	gluUnProject((GLdouble)g_global.imgWidth/2, (GLdouble)g_global.imgHeight/2, 0.0,\
				  mvmatrix, projmatrix, viewport, &wxOri, &wyOri, &wzOri);

	//	printf("\n%f %f %f\n",wxDir, wyDir, wzDir);

	//////////////////////////////////


	argDrawMode2D();
	argDispImage( dataPtr, 0,0 );




	/* detect the markers in the video frame */
	if( arDetectMarker(dataPtr, g_global.thresh, &marker_info, &marker_num) < 0 ) {
		cleanup();
		exit(0);
	}

	arVideoCapNext();

	/* check for object visibility */
	k = -1;
	for( j = 0; j < marker_num; j++ ) {
		if( g_global.patt_id == marker_info[j].id ) {
			if( k == -1 ) k = j;
			else if( marker_info[k].cf < marker_info[j].cf ) k = j;
		}
	}
	if( k == -1 ) {
		argSwapBuffers();
		return;
	}

	/* get the transformation between the marker and the real camera */
	arGetTransMat(&marker_info[k], g_global.patt_center, g_global.patt_width, g_global.patt_trans);

	//printf("\n%f %f %f\n",patt_trans[0][3],patt_trans[1][3],patt_trans[2][3]);

	draw(wxOri, wyOri, wzOri, wxDir, wyDir, wzDir);

	argSwapBuffers();
}