#include "stdafx.h"
#include "draw.h"

extern CGlobal g_global;

void   draw(double xOri, double yOri, double zOri,\
			double xDir, double yDir, double zDir)
{
	double    gl_para[16];
	GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
	GLfloat   mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
	GLfloat   mat_flash_shiny[] = {50.0};
	GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
	GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
	GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

	argDrawMode3D();
	argDraw3dCamera( 0, 0 );
	glClearDepth( 1.0 );
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	/* load the camera transformation matrix */
	argConvGlpara(g_global.patt_trans, gl_para);


	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd( gl_para );

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMatrixMode(GL_MODELVIEW);
	
	// ----------------------
	// by aicro

	// draw what we like
	glTranslatef( 0.0, 0.0, 25.0 );
	glutSolidCube(50.0);


	// ----------------------
	// by aicro

	CCrashTest crashTest;
	crashTest.calculateAABB(-25, 0, -25, 25, 50, 25);
	double t = crashTest.rayIntersect(xOri, yOri, zOri, xDir, yDir, zDir);
 
	if (t != -1)
	{
		
	}

	//// draw out the ray
	//glBegin(GL_LINES);
	//	glVertex3d(xOri,yOri,zOri);
	//	glVertex3d(xDir,yDir,zDir);
	//glEnd();


	// ----------------------

	glDisable( GL_LIGHTING );

	glDisable( GL_DEPTH_TEST );
}