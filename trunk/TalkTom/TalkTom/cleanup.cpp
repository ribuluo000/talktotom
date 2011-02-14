#include "stdafx.h"
#include "cleanup.h"

/* cleanup function called when program exits */

void   cleanup(void)
{
	arVideoCapStop();
	arVideoClose();
	argCleanup();
}
