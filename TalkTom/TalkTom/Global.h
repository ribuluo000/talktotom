#include "stdafx.h"


#ifndef __CGLOBAL__
#define __CGLOBAL__

class CGlobal
{
public:
	char			*vconf;
	int             imgWidth, imgHeight;
	int             thresh;

	char           *cparam_name;
	ARParam         cparam;

	char           *patt_name;
	int             patt_id;
	double          patt_width;
	double          patt_center[2];
	double          patt_trans[3][4];

	CGlobal()
	{
		char strVC[] = "..\\data\\WDM_camera_flipV.xml";
		int i = sizeof(strVC);
		vconf = new char [sizeof(strVC) + 1];
		memcpy(vconf, strVC, sizeof(strVC) + 1);

		imgWidth = imgHeight = 0;
		thresh = 100;

		char strCN[] = "..\\data\\camera_para.dat";
		cparam_name = new char [sizeof(strCN) + 1];
		memcpy(cparam_name, strCN, sizeof(strCN) + 1);

		char strPN[] = "..\\data\\patt.kanji";
		patt_name = new char [sizeof(strPN) + 1];
		memcpy(patt_name, strPN, sizeof(strPN) + 1);

		patt_id = 0;

		patt_width = 80.0;
		patt_center[0] = 0.0;
		patt_center[1] = 0.0;
	}

	~CGlobal()
	{
		if (vconf != NULL)
		{
			delete [] vconf;
		}

		if (cparam_name != NULL)
		{
			delete [] cparam_name;
		}

		if (patt_name != NULL)
		{
			delete [] patt_name;
		}
	}
};

#endif
