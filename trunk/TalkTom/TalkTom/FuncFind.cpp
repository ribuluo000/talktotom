#include "stdafx.h"
#include "FuncFind.h"

bool FuncFind(void* p1,void* p2)
{
	RECORESULT *pRecoResult = (RECORESULT*)p2;


	for (int i = 0;i < pRecoResult->m_number;i++)
	{
		WCHAR temp[100];
		swprintf(temp, L"Word ID = %d\nWord Des = %s\nRule ID = %d\nWords Num = %d\nWord Position = %d\n\0",
			pRecoResult->m_recoWordsIDArr[i],
			pRecoResult->m_recoWordsDesArr[i].c_str(),
			pRecoResult->m_ruleID,
			pRecoResult->m_WordsNum[i],
			pRecoResult->m_WordsPosition[i]);

		MessageBox(NULL, temp, NULL, MB_OK);
	}

	return true;
}
