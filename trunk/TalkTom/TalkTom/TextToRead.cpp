#include "stdafx.h"
#include "TextToRead.h"
/*******************************************************
//auxiliary string function
*******************************************************/
int isequal(WCHAR *s, WCHAR *t);
//************************************
// Description:
// to test whether target is contained in source, the function
// returns the first index of the substring, or -1 indicating 
// no match
//************************************
int SubStringIndex(WCHAR *source, WCHAR *target) 
{ 
	int i=0,j=0; 
	WCHAR *s2; 
	while(source[i]!='\0') 
	{ 
		s2=&source[i]; 
		if(isequal(s2,target)) 
			return i; 
		i++; 
	} 
	return -1; 
} 

int isequal(WCHAR *s,WCHAR *t) 
{ 
	int i=0; 
	while(t[i]!='\0') 
	{ 
		if(CharLower((PTSTR)s[i]) != CharLower((PTSTR)t[i])) 
			break; 
		i++; 
	} 
	if(t[i]=='\0') 
		return 1; 
	else 
		return 0; 
}




CTextToRead::CTextToRead()
{
	//initialize com
	CoInitialize( NULL );

	HRESULT hr = _InitSapi();


	 // Set the default output format
	if( SUCCEEDED( hr ) )
	{
		CComPtr<ISpStreamFormat> cpStream;
		HRESULT hrOutputStream = m_cpVoice->GetOutputStream(&cpStream);
		if (hrOutputStream == S_OK)
		{
			CSpStreamFormat Fmt;
			hr = Fmt.AssignFormat(cpStream);
			if (SUCCEEDED(hr))
			{
				m_CurrentStreamFormat = Fmt.ComputeFormatEnum();
			}
		}
	}


	// Set default voice data 
	if( SUCCEEDED(hr))
	{
		hr = m_cpVoice->GetVoice(&m_currentVoiceToken);
	}

	// Get default rate
	if( SUCCEEDED( hr ) )
	{
		hr = m_cpVoice->GetRate( &m_Rate );
	}

	// Get default volume
	if( SUCCEEDED( hr ) )
	{
		hr = m_cpVoice->GetVolume( &m_Volume );
	}

	if ( SUCCEEDED( hr ) )
	{
		SpCreateDefaultObjectFromCategoryId( SPCAT_AUDIOOUT, &m_cpOutAudio );
	}

	m_bStop = m_bPause = false;

	m_pszwFileText = NULL;

	// we make the relation between SPSTREAMFORMAT(only for some formats) and the index
	this->_makeRelation();


	//If any SAPI initialization failed, shut down!
	if (!SUCCEEDED(hr))
	{
		//TRACE(TEXT("Error in initial SAPI\n"));
		return;
	}
}

HRESULT CTextToRead::_InitSapi()
{
	HRESULT	hr = m_cpVoice.CoCreateInstance( CLSID_SpVoice );

	return hr;
}



//************************************
// Method:    _ChangeVoice
// FullName:  CTextToRead::_ChangeVoice
// Access:    public 
// Returns:   bool
// param:	  pToken

// Description: 
// If the new voice is different from the one that's currently 
// selected, it first stops any synthesis that is going on and
// sets the new voice on the global voice object. 
//************************************
bool CTextToRead::_ChangeVoice(ISpObjectToken* pToken)
{
	HRESULT         hr = S_OK;
	GUID*           pguidAudioFormat = NULL;
	int             iFormat = 0;

	// Get the token associated with the selected voice

	//Determine if it is the current voice
	CComPtr<ISpObjectToken> pOldToken;
	hr = m_cpVoice->GetVoice( &pOldToken );

	if (SUCCEEDED(hr))
	{
		if (pOldToken != pToken)
		{        
			// Stop speaking. This is not necessary, for the next call to work,
			// but just to show that we are changing voices.
			hr = m_cpVoice->Speak( NULL, SPF_PURGEBEFORESPEAK, 0);

			//// Get the id of the VOICE
			//WCHAR*  pszTokenIds;
			//hr = pToken->GetId(&pszTokenIds);
			//CoTaskMemFree(pszTokenIds);

			// And set the new voice on the global voice object
			if (SUCCEEDED (hr) )
			{
				hr = m_cpVoice->SetVoice( pToken );
			}
		}
	}

	return SUCCEEDED(hr);
}

bool CTextToRead::_ChangeVoice(WCHAR * voice)
{
	HRESULT hr = S_OK;
	ISpObjectToken * pToken = NULL;
	this->_GetVoice(voice, &pToken);
	if (pToken != NULL)
	{
		hr = this->_ChangeVoice(pToken);
	}
	return SUCCEEDED(hr);
}



bool CTextToRead::_NotifyWindowMessage( HWND hWnd, UINT message )
{
	if (hWnd == NULL)
	{
		return false;
	}

	m_message = message;

	HRESULT hr = S_OK;
	if ( !m_cpVoice )
	{
		hr = E_FAIL;
	}

	// Set the notification message for the voice
	if ( SUCCEEDED( hr ) )
	{
		// note that the TTS's window message is the responsiblity of IspVoice
		m_cpVoice->SetNotifyWindowMessage( hWnd, message, 0, 0 );
	}

	// We're interested in all TTS events
	if( SUCCEEDED( hr ) )
	{
		hr = m_cpVoice->SetInterest( SPFEI_ALL_TTS_EVENTS, SPFEI_ALL_TTS_EVENTS );
	}

	return SUCCEEDED(hr);
}


//************************************
// Method:    ReadTheFile
// FullName:  CTextToRead::ReadTheFile
// Access:    private 
// Returns:   HRESULT
// Qualifier:
// Parameter: TCHAR * szFileName
// Parameter: BOOL * bIsUnicode
// Parameter: WCHAR * * ppszwBuff

// Description:
// This file opens and reads the contents of a file. It
// returns a pointer to the string.
// Warning, this function allocates memory for the string on 
// the heap so the caller must free it with 'delete'.

//************************************
HRESULT CTextToRead::_ReadTheFile( TCHAR* szFileName, BOOL* bIsUnicode, WCHAR** ppszwBuff )
{
	// Open up the file and copy it's contents into a buffer to return
	HRESULT		hr = 0;
	HANDLE		hFile;
	DWORD		dwSize = 0;
	DWORD		dwBytesRead = 0;

	// First delete any memory previously allocated by this function
	if( m_pszwFileText )
	{
		delete [] m_pszwFileText;
	}

	hFile = CreateFile( szFileName, GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		*ppszwBuff = NULL;
		hr = E_FAIL;
	}

	if( SUCCEEDED( hr ) )
	{
		dwSize = GetFileSize( hFile, NULL );
		if( dwSize == 0xffffffff )
		{
			*ppszwBuff = NULL;
			hr = E_FAIL;        
		}
	}

	if( SUCCEEDED( hr ) )
	{
		// Read the file contents into a wide buffer and then determine
		// if it's a unicode or ascii file
		WCHAR	Signature = 0;

		ReadFile( hFile, &Signature, 2, &dwBytesRead, NULL );

		// Check to see if its a unicode file by looking at the signature of the first character.
		if( 0xFEFF == Signature )
		{
			*ppszwBuff = new WCHAR [dwSize/2];

			*bIsUnicode = TRUE;
			ReadFile( hFile, *ppszwBuff, dwSize-2, &dwBytesRead, NULL );
			(*ppszwBuff)[dwSize/2-1] = NULL;

			CloseHandle( hFile );
		}			
		else  // MBCS source
		{
			char*	pszABuff = new char [dwSize+1];
			*ppszwBuff = new WCHAR [dwSize+1];

			*bIsUnicode = FALSE;
			SetFilePointer( hFile, NULL, NULL, FILE_BEGIN );
			ReadFile( hFile, pszABuff, dwSize, &dwBytesRead, NULL );
			pszABuff[dwSize] = NULL;
			::MultiByteToWideChar( CP_ACP, 0, pszABuff, -1, *ppszwBuff, dwSize + 1 );

			delete( pszABuff );
			CloseHandle( hFile );
		}
	}

	return hr;
}

void CTextToRead::_ReadFromFile( TCHAR* szFileName, BOOL* bIsUnicode)
{
	this->_ReadTheFile(szFileName, bIsUnicode, &m_pszwFileText);
	
	this->_ReadText(m_pszwFileText);

	//we allocate it in ReadTheFile
	delete [] m_pszwFileText;

	m_pszwFileText = NULL;
}



void CTextToRead::_ReadText( WCHAR* pszwBuff )
{
	HRESULT hr = S_OK;

	m_bStop = FALSE;
	
	// only get the string if we're not paused
	if( !m_bPause )
	{
		hr = m_cpVoice->Speak( pszwBuff, SPF_ASYNC | SPF_IS_NOT_XML , NULL );
	}

	m_bPause = FALSE;
	// Set state to run
	hr = m_cpVoice->Resume();            
}

void CTextToRead::_PauseRestartRead()
{
	if( !m_bStop )
	{
		if( !m_bPause )
		{
			// Pause the voice...
			m_cpVoice->Pause();
			m_bPause = TRUE;
		}
		else
		{
			// Restart the voice...
			m_cpVoice->Resume();
			m_bPause = FALSE;
		}
	}
}

//************************************
// Method:    StopRead
// FullName:  CTextToRead::StopRead
// Access:    public 
// Returns:   void
// Qualifier:

// Description:
// Stop reading, and resets global audio state to stopped
//************************************
void CTextToRead::_StopRead()
{
	// Stop current rendering with a PURGEBEFORESPEAK...
	HRESULT hr = m_cpVoice->Speak( NULL, SPF_PURGEBEFORESPEAK, 0 );

	m_bPause = FALSE;
	m_bStop = TRUE;             
}

bool CTextToRead::_PlayWave( TCHAR* szFileName )
{
	//not checking the accuracy of the file type
	if (szFileName == NULL)
	{
		return false;
	}

	CComPtr<ISpStream> cpWavStream;

	// User helper function found in sphelper.h to open the wav file and
	// get back an IStream pointer to pass to SpeakStream
	HRESULT hr = SPBindToFile( szFileName, SPFM_OPEN_READONLY, &cpWavStream );

	if( SUCCEEDED( hr ) )
	{
		hr = m_cpVoice->SpeakStream( cpWavStream, SPF_ASYNC, NULL );
	}

	return SUCCEEDED(hr);
}

bool CTextToRead::_SaveToWavFile( WCHAR* szFileName, WCHAR* wszTextToSave )
{
	if (szFileName == NULL)
	{
		return false;
	}

	//check weather the last 3 character of szFileName is 'wav'
	int length = wcslen(szFileName);
	WCHAR * temp = szFileName + (length - 3);
	if (wcscmp(temp, TEXT("wav")) != 0)
	{
		// temp is not 'wav'
		return false;
	}
	

	USES_CONVERSION;

	CComPtr<ISpStreamFormat>    cpOldStream;
	CComPtr<ISpStream>  cpWavStream;
	CSpStreamFormat OriginalFmt;

	HRESULT hr = m_cpVoice->GetOutputStream( &cpOldStream );
	if (hr == S_OK)
	{
		hr = OriginalFmt.AssignFormat(cpOldStream);
	}
	else
	{
		hr = E_FAIL;
	}
	// User SAPI helper function in sphelper.h to create a wav file
	if (SUCCEEDED(hr))
	{
		hr = SPBindToFile( szFileName, SPFM_CREATE_ALWAYS,\
						   &cpWavStream, &OriginalFmt.FormatId(), \
						   OriginalFmt.WaveFormatExPtr() ); 
	}
	if( SUCCEEDED( hr ) )
	{
		// Set the voice's output to the wav file instead of the speakers
		hr = m_cpVoice->SetOutput(cpWavStream, TRUE);
	}

	if ( SUCCEEDED( hr ) )
	{
		// Do the Speak, now the target is wav file rather than speakers
		this->_ReadText(wszTextToSave);
	}

	// Set output back to original stream
	// Wait until the speak is finished if saving to a wav file so that
	// the smart pointer cpWavStream doesn't get released before its
	// finished writing to the wav.
	m_cpVoice->WaitUntilDone( INFINITE );
	cpWavStream.Release();

	// Reset output
	m_cpVoice->SetOutput( cpOldStream, FALSE );

	if ( SUCCEEDED( hr ) )
	{
		return true;
	}
	else
	{
		return false;	
	}
}

CTextToRead::~CTextToRead()
{
	// delete any allocated memory
	if( m_pszwFileText )
	{
		delete [] m_pszwFileText;
	}


	// Release voice, if created
	if ( m_cpVoice )
	{
		m_cpVoice->SetNotifySink(NULL);
		m_cpVoice.Release();
	}

	// Release outaudio, if created
	if(m_cpOutAudio)
	{
		m_cpOutAudio.Release();
	}

	// Unload COM
	CoUninitialize();
}

HRESULT CTextToRead::_GetVoice( WCHAR * voice , ISpObjectToken ** ppGotToken)
{
	*ppGotToken = NULL;

	HRESULT hr;
	ISpObjectToken * pToken;        // NOTE:  Not a CComPtr!  Be Careful.
	CComPtr<IEnumSpObjectTokens> cpEnum;
	hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);
	if (hr == S_OK)
	{
		bool fSetDefault = false;
		while (cpEnum->Next(1, &pToken, NULL) == S_OK)
		{
			CSpDynamicString dstrDesc;
			hr = SpGetDescription(pToken, &dstrDesc);
			if (SUCCEEDED(hr))
			{
				WCHAR * temp = dstrDesc.Copy();
				if ( -1 != SubStringIndex(temp, voice) )
				{
					//the voice is what we need. Set this voice
					*ppGotToken = pToken;

					break;
				}
			}
			if (FAILED(hr))
			{
				pToken->Release();
			}
		}
	}
	else
	{
		hr = SPERR_NO_MORE_ITEMS;
	}

	return hr;
}

HRESULT CTextToRead::_SetOutStreamFormat( SPSTREAMFORMAT format )
{
	HRESULT hr = E_FAIL;
	SPSTREAMFORMAT eFmt = format;
	
	CSpStreamFormat Fmt;
	Fmt.AssignFormat(eFmt);

	if ( m_cpOutAudio )
	{
		hr = m_cpOutAudio->SetFormat( Fmt.FormatId(), Fmt.WaveFormatExPtr() );
		m_CurrentStreamFormat = format;
	}
	else
	{
		hr = E_FAIL;
	}

	if( SUCCEEDED( hr ) )
	{
		hr = m_cpVoice->SetOutput( m_cpOutAudio, FALSE );
	}

	return hr;
}

HRESULT CTextToRead::_SetOutStreamFormat( int khz, int bit, bool bMono )
{
	// combine the three parameter into a SPSTREAMFORMAT form phrase
	//WCHAR swTempFormat[25];
	//swprintf(swTempFormat, L"SPSF_%dkHz%dBit%s", khz, bit, bMono?L"Mono":L"Stereo");

	HRESULT hr = E_FAIL;

	int length = m_vStreamFormat.size();
	for (int i = 0;i < length;i++)
	{
		if(     m_vStreamFormat[i].m_khz == khz
		   &&	m_vStreamFormat[i].m_bit == bit
		   &&   m_vStreamFormat[i].m_bMono == bMono)
		{
			hr = this->_SetOutStreamFormat((SPSTREAMFORMAT)m_vStreamFormat[i].m_index);
			break;
		}
	}

	return hr;
}


//************************************
// Method:    _RegisterMessageFunc
// FullName:  CTextToRead::_RegisterMessageFunc
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: UINT message		message
// Parameter: * func			callback function
// Parameter: DWORD * param		parameter passed into func

// Description:
//************************************
void CTextToRead::_RegisterMessageFunc( UINT message, bool (*pFunc)(void *, void*), DWORD * pParam )
{
	MESSAGE_MAP message_map(message, pFunc, pParam);

	m_MessageMap.push_back(message_map);
}

bool CTextToRead::_UnRegisterMessage( UINT message )
{
	bool bFind = false;
	for (std::vector<MESSAGE_MAP>::iterator it = m_MessageMap.begin(); 
		 it != m_MessageMap.end(); 
		 it++)
	{
		if (message == it->m_message)
		{
			m_MessageMap.erase(it);
			bFind = true;
			break;
		}
	}
	return bFind;
}

//************************************
// Method:    _DealMessage
// FullName:  CTextToRead::_DealMessage
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: UINT message

// Description:
// if _DealMessage returns -1, it means that we do not have the corresponding 
// message dealing function. if it returns 0 or 1, it means the dealing function
// returns it.
//************************************
int CTextToRead::_DealMessage(UINT message)
{
	int iDeal = -1;

	if (message == m_message)
	{
		//it is the message that we want
		
		CSpEvent        event;  // helper class in sphelper.h for events that releases any 
								// allocated memory in it's destructor - SAFER than SPEVENT

		while( event.GetFrom(m_cpVoice) == S_OK )
		{
			for (std::vector<MESSAGE_MAP>::iterator it = m_MessageMap.begin(); 
				it != m_MessageMap.end(); 
				it++)
			{
				if (event.eEventId == it->m_message)
				{
					iDeal = (int) (it->m_pFunc(it->m_pParam, NULL));
					break;
				}
			}
		}
	}

	return iDeal;
}

void CTextToRead::_makeRelation()
{
	STREAMFORMAT streamFormat;
	streamFormat.Generate(SPSF_8kHz8BitMono, 8, 8, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_8kHz16BitMono, 8, 16, true);
	m_vStreamFormat.push_back(streamFormat);
	
	streamFormat.Generate(SPSF_8kHz16BitStereo, 8, 16, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_11kHz8BitMono, 11, 8, true);
	m_vStreamFormat.push_back(streamFormat);
	
	streamFormat.Generate(SPSF_11kHz8BitStereo, 11, 8, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_11kHz16BitMono, 11, 16, true);
	m_vStreamFormat.push_back(streamFormat);
		
	streamFormat.Generate(SPSF_11kHz16BitStereo, 11, 16, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_12kHz8BitMono, 12, 8, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_12kHz8BitStereo, 12, 8, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_12kHz16BitMono, 12, 16, true);
	m_vStreamFormat.push_back(streamFormat);
	
	streamFormat.Generate(SPSF_12kHz16BitStereo, 12, 16, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_16kHz8BitMono, 16, 8, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_16kHz8BitStereo, 16, 8, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_16kHz16BitMono, 16, 16, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_16kHz16BitStereo, 16, 16, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_22kHz8BitMono, 22, 8, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_22kHz8BitStereo, 22, 8, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_22kHz16BitMono, 22, 16, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_22kHz16BitStereo, 22, 16, false);
	m_vStreamFormat.push_back(streamFormat);
		
	streamFormat.Generate(SPSF_24kHz8BitMono, 24, 8, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_24kHz8BitStereo, 24, 8, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_24kHz16BitMono, 24, 16, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_24kHz16BitStereo, 24, 16, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_32kHz8BitMono, 32, 8, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_32kHz8BitStereo, 32, 8, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_32kHz16BitMono, 32, 16, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_32kHz16BitStereo, 32, 16, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_44kHz8BitMono, 44, 8, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_44kHz8BitStereo, 44, 8, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_44kHz16BitMono, 44, 16, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_44kHz16BitStereo, 44, 16, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_48kHz8BitMono, 48, 8, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_48kHz8BitStereo, 48, 8, false);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_48kHz16BitMono, 48, 16, true);
	m_vStreamFormat.push_back(streamFormat);

	streamFormat.Generate(SPSF_48kHz16BitStereo, 48, 16, false);
	m_vStreamFormat.push_back(streamFormat);
}

bool CTextToRead::_GetVoiceList(WCHAR ** ppVoiceDesArr, int* pWordsLengthArr, ULONG& numOfVoice)
{
	HRESULT hr = E_FAIL;
	ISpObjectToken * pToken;        // NOTE:  Not a CComPtr!  Be Careful.
	CComPtr<IEnumSpObjectTokens> cpEnum;
	hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);
	if (hr == S_OK)
	{
		cpEnum->GetCount(&numOfVoice);
		if (ppVoiceDesArr == NULL && pWordsLengthArr == NULL)
		{
			// we only focus on the number of available voices
			return true;
		}
		
		// then get descriptions and indexes
		int uIndex = 0;
		while (cpEnum->Next(1, &pToken, NULL) == S_OK)
		{
			CSpDynamicString dstrDesc;
			hr = SpGetDescription(pToken, &dstrDesc);
			
			if ( SUCCEEDED(hr))
			{
				WCHAR * temp = dstrDesc.Copy();
				if (pWordsLengthArr != NULL)
				{
					pWordsLengthArr[uIndex] = wcslen(temp) + 1;
				}
				if (ppVoiceDesArr != NULL)
				{
					wcscpy(ppVoiceDesArr[uIndex], dstrDesc.Copy());
				}				
			}
			if ( FAILED(hr) )
			{
				if (pWordsLengthArr != NULL)
				{
					pWordsLengthArr[uIndex] = 0;
				}
				if (ppVoiceDesArr != NULL)
				{
					ppVoiceDesArr[uIndex] = NULL;
				}
				pToken->Release();
			}
			uIndex++;
		}
	}
	else
		return false;

	return true;
}

