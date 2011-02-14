#include "stdafx.h"
#include "init.h"

extern CGlobal g_global;

void init()
{
	ARParam  wparam;

	/* open the video path */
	if( arVideoOpen( g_global.vconf ) < 0 ) exit(0);
	/* find the size of the window */
	if( arVideoInqSize(&g_global.imgWidth, &g_global.imgHeight) < 0 ) exit(0);
	printf("Image size (x,y) = (%d,%d)\n", g_global.imgWidth, g_global.imgHeight);

	/* set the initial camera parameters */
	if( arParamLoad(g_global.cparam_name, 1, &wparam) < 0 ) {
		printf("Camera parameter load error !!\n");
		exit(0);
	}
	arParamChangeSize( &wparam, g_global.imgWidth, g_global.imgHeight, &g_global.cparam );
	arInitCparam( &g_global.cparam );
	printf("*** Camera Parameter ***\n");
	arParamDisp( &g_global.cparam );

	if( (g_global.patt_id=arLoadPatt(g_global.patt_name)) < 0 ) {
		printf("pattern load error !!\n");
		exit(0);
	}

	/* open the graphics window */
	argInit( &g_global.cparam, 1.0, 0, 0, 0, 0 );
	
	glutReshapeWindow(500,500);
}