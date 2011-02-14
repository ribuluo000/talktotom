// TalkTom.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


CGlobal g_global;

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	init();


	arVideoCapStart();
	argMainLoop(mouseEvent, keyEvent, mainLoop );


	return (0);
}

