#ifndef __STRUCTFORSRANDTTS
#define __STRUCTFORSRANDTTS

#include "stdafx.h"
#include <vector>
#include <string>



typedef struct strMessageMap
{
	UINT m_message;
	bool (*m_pFunc)(void*, void*);
	DWORD * m_pParam;
	strMessageMap(UINT message, bool (*pFunc)(void*, void*), DWORD * pParam)
		:m_message( message ),
		m_pFunc(pFunc),
		m_pParam(pParam){}
}MESSAGE_MAP, * PMESSAGE_MAP;

typedef struct strStreamFormat 
{
	int m_index;
	int m_khz;
	int m_bit;
	bool m_bMono;
	void Generate(int index, int khz, int bit, bool bMono)
	{
		m_index = index;
		m_khz = khz;
		m_bit = bit;
		m_bMono = bMono;
	}
}STREAMFORMAT, *PSTREAMFORMAT;


typedef struct  strRecoResult
{
	int				     m_ruleID; // ruleId
	std::vector<ULONG>	 m_recoWordsIDArr; // array for recognized words id
	std::vector<std::wstring> m_recoWordsDesArr;// array for recognized words description string
	std::vector<int>     m_WordsPosition; // array for recognized words position in 
	                                     //the original spoken sentence 
	std::vector<int>     m_WordsNum;      // array for numbers of a single group recognized words.
										 // ie. how many words are there in the group

	int				m_number; // number of recognized words
	
	void Clear()
	{
		m_recoWordsDesArr.clear();
		m_recoWordsIDArr.clear();
		m_WordsNum.clear();
		m_WordsPosition.clear();
		m_number = 0;
	}

}RECORESULT, *PRECORESULT;


typedef struct strNewRuleInfo
{
	int		m_ruleID;
	int		m_propID;
	std::wstring	m_des;
	strNewRuleInfo(int ruleID, int propID, std::wstring des)
		: m_ruleID(ruleID),
		  m_propID(propID),
		  m_des(des){}
}NEWRULEINFO, *PNUEWRULEINFO;

#endif