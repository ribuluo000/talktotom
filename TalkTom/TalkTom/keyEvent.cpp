#include "stdafx.h"
#include "keyEvent.h"

extern CGlobal g_global;

void   keyEvent( unsigned char key, int x, int y)
{
	/* quit if the ESC key is pressed */
	if( key == 0x1b ) {
		cleanup();
		exit(0);
	}

	if( key == 'a' ) {
		int i = 0;
	}

}