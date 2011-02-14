#ifndef	__TEXTTOREAD
#define __TEXTTOREAD

#pragma warning (disable: 4995)

#include "stdafx.h"
#include <windows.h>
#include <sapi.h>           // SAPI includes
#include <sphelper.h>
#include <spuihelp.h>
#include <vector>
#include "StructForSRandTTS.h"

class CTextToRead
{
public:
	CTextToRead();
	~CTextToRead();


public:
	void	_ReadText(WCHAR* pszwBuff);
	void	_ReadFromFile(TCHAR* szFileName, BOOL* bIsUnicode);
	void	_PauseRestartRead();
	void	_StopRead();
	bool	_ChangeVoice(ISpObjectToken* pToken);
	bool	_ChangeVoice(WCHAR * voice);
	// to use this method, first get the number of voices and length of each 
	// voice description, and then allocate memory for WCHAR* VoiceDesArr[num],
	// and reuse the method with VoiceDesArr.
	bool	_GetVoiceList(WCHAR ** ppVoiceDesArr, int* pWordsLengthArr, ULONG& numOfVoice);


	bool	_NotifyWindowMessage(HWND hWnd, UINT Message);
	//messages passed to the following three methods are 
	//those that used in SAPI, beginning with SPEI_
	void	_RegisterMessageFunc(UINT message, bool (*pFunc)(void *, void*), DWORD * param);
	bool	_UnRegisterMessage(UINT message);
	int		_DealMessage(UINT message);
	
	bool	_PlayWave(TCHAR* szFileName);
	bool	_SaveToWavFile(WCHAR* szFileName, WCHAR* wszTextToSave);

public:
	void	_SetVolumn(USHORT volumn) {m_Volume = volumn; m_cpVoice->SetVolume((USHORT)m_Volume);}
	USHORT	_GetVolumn(){return m_Volume; }
	void	_SetRate(long rate){m_Rate = rate; m_cpVoice->SetRate(m_Rate);}
	long	_GetRate(){return m_Rate; }
	HRESULT	_SetOutStreamFormat(SPSTREAMFORMAT format);
	//only support Standard PCM wave formats
	HRESULT	_SetOutStreamFormat(int khz, int bit, bool bMono);


	SPSTREAMFORMAT	_GetOutStreamFormat(){return m_CurrentStreamFormat; }
	

private:
	HRESULT _InitSapi();
	HRESULT _ReadTheFile( TCHAR* szFileName, BOOL* bIsUnicode, WCHAR** ppszwBuff );
	//available param is 'Mary', 'Mike', and 'Chinese'
	HRESULT _GetVoice(WCHAR * voice , ISpObjectToken ** ppGotToken);
	void	_makeRelation();


private:
	CTextToRead(CTextToRead & ){}
	CTextToRead& operator=(CTextToRead &){}

//
//  Member data
// 
private:
	CComPtr<ISpVoice>   m_cpVoice;
	CComPtr<ISpAudio>   m_cpOutAudio;
	BOOL                m_bPause;
	BOOL                m_bStop;
	USHORT              m_Volume;
	long                m_Rate;
	SPSTREAMFORMAT      m_CurrentStreamFormat;
	WCHAR*              m_pszwFileText;
	ISpObjectToken*		m_currentVoiceToken;
	std::vector<MESSAGE_MAP>	m_MessageMap;
	UINT				m_message;
	std::vector<STREAMFORMAT>	m_vStreamFormat;
};



#endif