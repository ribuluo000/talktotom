#ifndef	__SOUNDTOTEXT
#define __SOUNDTOTEXT

#pragma warning(disable: 4995)

#include "stdafx.h"
#include <windows.h>
#include <sapi.h>           // SAPI includes
#include <sphelper.h>
#include <spuihelp.h>
#include "StructForSRandTTS.h"
#include <vector>



class CSpeechToText
{
public:
	CSpeechToText(WCHAR * szGrammarFileName);
	~CSpeechToText();

public:

	////////////////////////////////////////////////////////////////
	// command and control mode
	bool _EnableRule(int RULE_ID);
	bool _DisableRule(int RULE_ID);

	////////////////////////////////////////////////////////////////
	// dynamic rule

	// ruleID must be defined higher than 0x0400
	// either ruleName or ruleID should be given
	int _CreateNewRule( int ruleID, 
						bool bClearRule = true);
	bool _DeleteCreatedRule(int ruleID);

	bool _AddWordsIntoNewRule(int ruleID, WCHAR * words);
	// not realized yet
	bool _DeleteWordsFromRule(int ruleID, WCHAR * words);



	/////////////////////////////////////////////////////////////////
	// message-related methods
	bool	_NotifyWindowMessage(HWND hWnd, UINT Message);
	// messages passed to the following three methods are 
	// those that used in SAPI, beginning with SPEI_
	void	_RegisterMessageFunc(UINT message, bool (*pFunc)(void*, void*), DWORD * param);
	bool	_UnRegisterMessage(UINT message);
	// here we pass the ISpRecoResult* to the first parameter to MESSAGE_MAP::m_pFunc
	int		_DealMessage(UINT message);


	bool _IsSuccess(){return m_bSuccess; }

private:
	void _CleanupSAPI();
	bool _Recognize( ISpRecoResult * pRSResult);
	// recursive traverse grammar trees
	void _TraverseLeaves(const SPPHRASEPROPERTY * pPro, 
					     const SPPHRASERULE ** ppRule,
						 ISpRecoResult** ppSpRecoResult);
	void _GetLeaveMessage(const SPPHRASEPROPERTY * pProp, 
						  const SPPHRASERULE ** ppRule,
						  ISpRecoResult** ppSpRecoResult);

private:
	CSpeechToText(){}
	CSpeechToText(CSpeechToText &){}
	CSpeechToText& operator=(CSpeechToText &){}

private:
	CComPtr<ISpRecoGrammar>         m_cpCmdGrammar; // Pointer to our grammar object
	CComPtr<ISpRecoContext>         m_cpRecoCtxt;   // Pointer to our recognition context
	CComPtr<ISpRecognizer>		    m_cpEngine;		// Pointer to our recognition engine instance

private:
	bool						m_bSuccess; // indicate whether the struct method succeeds
	std::vector<MESSAGE_MAP>	m_MessageMap;
	UINT						m_message;
	std::vector<STREAMFORMAT>	m_vStreamFormat;
	RECORESULT					m_recoResult;
	UINT						BEGIN_ID;
	std::vector<int>			m_vecCreatedRuleID; // vector for created rule id
	std::vector<NEWRULEINFO>	m_vecRuleInfo;
};


#endif