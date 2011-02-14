#include "Stdafx.h"
#include "SpeechToText.h"



CSpeechToText::CSpeechToText(WCHAR * szGrammarFileName)
{
	m_bSuccess = true;

	BEGIN_ID = 0x1000;

	//initialize com
	CoInitialize( NULL );

	while(1)
	{
		HRESULT hr = E_FAIL;
		// create a recognition engine
		hr = m_cpEngine.CoCreateInstance(CLSID_SpSharedRecognizer);
		if ( FAILED( hr ) )
		{
			m_bSuccess = false;
			break;
		}

		// create the command recognition context
		hr = m_cpEngine->CreateRecoContext( &m_cpRecoCtxt );
		if ( FAILED( hr ) )
		{
			m_bSuccess = false;
			break;
		}

		// create the grammar and load the xml file.
		hr = m_cpRecoCtxt->CreateGrammar(161, &m_cpCmdGrammar);
		if (FAILED(hr))
		{
			m_bSuccess = false;
			break;
		}

		hr = m_cpCmdGrammar->LoadCmdFromFile(szGrammarFileName, SPLO_DYNAMIC);
		if (FAILED(hr))
		{
			m_bSuccess = false;
			break;
		}

		// set all xml-specified-active rules active
		hr = m_cpCmdGrammar->SetRuleState( NULL, NULL, SPRS_ACTIVE );
		if (FAILED(hr))
		{
			m_bSuccess = false;
			break;
		}

		break;
	}
	if ( m_bSuccess == false )
	{
		_CleanupSAPI();
	}
}

CSpeechToText::~CSpeechToText()
{
	this->_CleanupSAPI();
}

void CSpeechToText::_CleanupSAPI()
{
	// Release grammar, if loaded
	if ( m_cpCmdGrammar )
	{
		m_cpCmdGrammar.Release();
	}
	// Release recognition context, if created
	if ( m_cpRecoCtxt )
	{
		m_cpRecoCtxt->SetNotifySink(NULL);
		m_cpRecoCtxt.Release();
	}
	// Release recognition engine instance, if created
	if ( m_cpEngine )
	{
		m_cpEngine.Release();
	}

	// Unload COM
	CoUninitialize();
}



bool CSpeechToText::_EnableRule(int RULE_ID)
{
	HRESULT hr = S_OK;
	hr = m_cpCmdGrammar->SetRuleIdState( RULE_ID, SPRS_ACTIVE );
	if ( FAILED( hr ) )
	{
		hr = E_FAIL;
	}
	return SUCCEEDED(hr);
}

bool CSpeechToText::_DisableRule(int RULE_ID)
{
	HRESULT hr = S_OK;
	hr = m_cpCmdGrammar->SetRuleIdState( RULE_ID, SPRS_INACTIVE );
	if ( FAILED( hr ) )
	{
		hr = E_FAIL;
	}
	return SUCCEEDED(hr);
}

bool CSpeechToText::_NotifyWindowMessage( HWND hWnd, UINT message )
{
	if (hWnd == NULL)
	{
		return false;
	}

	m_message = message;

	HRESULT hr = S_OK;
	if ( !m_cpRecoCtxt )
	{
		hr = E_FAIL;
	}

	// Set the notification message for the voice
	if ( SUCCEEDED( hr ) )
	{
		m_cpRecoCtxt->SetNotifyWindowMessage( hWnd, message, 0, 0 );
	}

	// We're interested in all SR events
	if( SUCCEEDED( hr ) )
	{
		ULONGLONG l =    SPFEI(SPEI_END_SR_STREAM)			|
						 SPFEI(SPEI_SOUND_START)			|
						 SPFEI(SPEI_SOUND_END)				|
						 SPFEI(SPEI_PHRASE_START)			|
						 SPFEI(SPEI_RECOGNITION)			|
						 SPFEI(SPEI_HYPOTHESIS)				|
						 SPFEI(SPEI_SR_BOOKMARK)			|
						 SPFEI(SPEI_PROPERTY_NUM_CHANGE)	|
						 SPFEI(SPEI_PROPERTY_STRING_CHANGE) |
						 SPFEI(SPEI_FALSE_RECOGNITION)		|
						 SPFEI(SPEI_INTERFERENCE)			|
						 SPFEI(SPEI_REQUEST_UI)				|
						 SPFEI(SPEI_RECO_STATE_CHANGE)		|
						 SPFEI(SPEI_ADAPTATION)				|
						 SPFEI(SPEI_START_SR_STREAM)		|
						 SPFEI(SPEI_RECO_OTHER_CONTEXT)		|
						 SPFEI(SPEI_SR_AUDIO_LEVEL);

		hr = m_cpRecoCtxt->SetInterest( l, l );
	}

	return SUCCEEDED(hr);
}


void CSpeechToText::_RegisterMessageFunc( UINT message, bool (*pFunc)(void *, void*), DWORD * pParam )
{
	MESSAGE_MAP message_map(message, pFunc, pParam);

	m_MessageMap.push_back(message_map);
}

bool CSpeechToText::_UnRegisterMessage( UINT message )
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
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: UINT message

// Description:
// if _DealMessage returns -1, it means that we do not have the corresponding 
// message dealing function. if it returns 0 or 1, it means the dealing function
// returns it.
//************************************
int CSpeechToText::_DealMessage(UINT message)
{
	int iDeal = -1;

	USES_CONVERSION;

	if (message == m_message)
	{
		//it is the message that we want
		
		CSpEvent        event;  // helper class in sphelper.h for events that releases any 
								// allocated memory in it's destructor - SAFER than SPEVENT
		while( event.GetFrom(m_cpRecoCtxt) == S_OK )
		{
			// assign the specific event to the specific function
			for (std::vector<MESSAGE_MAP>::iterator it = m_MessageMap.begin(); 
				it != m_MessageMap.end(); 
				it++)
			{
				if (event.eEventId == it->m_message)
				{
					if (it->m_message == SPEI_RECOGNITION)
					{
						// we have recognized some words
						// get the reco words
						this->_Recognize(event.RecoResult());
					}

					// invoke user defined function
					iDeal = (int) (it->m_pFunc(it->m_pParam, &m_recoResult));

					if (it->m_message == SPEI_RECOGNITION)
					{
						m_recoResult.Clear();
					}

					break;
				}
			}
		}
	}

	return iDeal;
}

//************************************
// Method:    _Recognize
// FullName:  CSpeechToText::_Recognize
// Access:    private 
// Returns:   bool

// Description:
// this method specially deal with the specific grammar written format, which is 
// given as the affiliated file with the class source code. We must design the grammar
// according to the sample, or there may be some errors in using the class

//************************************
bool CSpeechToText::_Recognize(ISpRecoResult * pSRResult)
{
	bool bSuccess = true;

    USES_CONVERSION;
    
    SPPHRASE *pElements;

    // Get the phrase elements, one of which is the rule id we specified in
    // the grammar.  Switch on it to figure out which command was recognized.
    if (SUCCEEDED(pSRResult->GetPhrase(&pElements)))
    {
		// get the rule id
		m_recoResult.m_ruleID = pElements->Rule.ulId;


		const SPPHRASEPROPERTY *pProp = NULL;
		const SPPHRASERULE *pRule = NULL;

		// get the head of the tree
		pProp = pElements->pProperties;
		pRule = pElements->Rule.pFirstChild;

		// take advantage of non-terminal symbols are in the leaves position
		this->_TraverseLeaves(pProp, &pRule, &pSRResult);	

		// Free the pElements memory which was allocated for us
		::CoTaskMemFree(pElements);	
	}

	return bSuccess;
}

void CSpeechToText::_TraverseLeaves(const SPPHRASEPROPERTY * pPro, 
									const SPPHRASERULE ** ppRule,
									ISpRecoResult** ppSpRecoResult)
{
	if (pPro != NULL)
	{
		this->_TraverseLeaves(pPro->pFirstChild, ppRule, ppSpRecoResult);
		if (pPro->pFirstChild == NULL)
		{
			// deal leaves only
			this->_GetLeaveMessage(pPro, ppRule, ppSpRecoResult);
		}
		this->_TraverseLeaves(pPro->pNextSibling, ppRule, ppSpRecoResult);
	}
}

void CSpeechToText::_GetLeaveMessage( const SPPHRASEPROPERTY * pProp, 
									  const SPPHRASERULE ** ppRule,
									  ISpRecoResult** ppSpRecoResult)
{
		ULONG ulFirstElement, ulCountOfElements;

		m_recoResult.m_recoWordsIDArr.push_back(static_cast< ULONG >(pProp->vValue.ulVal) );
		// Get the count of elements from the rule ref, not the actual leaf
		// property
		if ( *ppRule )
		{
			ulFirstElement = (*ppRule)->ulFirstElement;
			ulCountOfElements = (*ppRule)->ulCountOfElements;
		}
		else
		{
			// in this case, the only possible is the speaker is speaking 
			// the new words inserted into the new rule
			ulFirstElement = 0;
			ulCountOfElements = 0;
		}

		m_recoResult.m_WordsPosition.push_back(ulFirstElement);
		m_recoResult.m_WordsNum.push_back(ulCountOfElements);


		// This is the text corresponding to property iCnt - it must be
		// released when we are done with it
		WCHAR* temp = NULL;
		(*ppSpRecoResult)->GetText( ulFirstElement, ulCountOfElements,FALSE, &temp, NULL);
		
		if (temp != NULL)
		{
			m_recoResult.m_recoWordsDesArr.push_back(temp);
			CoTaskMemFree( temp );
		}
		else
		{
			// in this case, the only possible is the speaker is speaking 
			// the new words inserted into the new rule

			// traverse the registered words' property value and return the right info
			int k = m_vecRuleInfo.size();
			for (int j = 0; j < k; j++)
			{
				if (m_vecRuleInfo[j].m_propID == pProp->vValue.ulVal)
				{
					// it is what is spoken
					m_recoResult.m_recoWordsDesArr.push_back(m_vecRuleInfo[j].m_des);
					m_recoResult.m_recoWordsIDArr.push_back(m_vecRuleInfo[j].m_propID);
				}
			}
		}

		m_recoResult.m_number++;

		// traverse the rule
		if (*ppRule != NULL)
		{
			*ppRule = (*ppRule)->pNextSibling;
		}
}

//************************************
// Method:    _CreateRule
// FullName:  CSpeechToText::_CreateRule
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: int ruleID
// Parameter: bool bCreateIfNotExist
// Parameter: bool bClearRule

// Description:
//	either ruleName or ruleID must be specified. the function will create a new	rule 
//  if return 1, then it is OK. if it returns 0, then some functions error.
//  if it returns -1, then No rule matching the specified criteria can be found and a new rule is not created.
//  if it returns -2, then One of the name and ID matches an existing rule but the other does not match the same rule.
//  if it returns -3, then At least one parameter is invalid. Also returned when both pszRuleName and dwRuleId are NULL.
//  if it returns -4, then Not enough memory to complete operation.
//************************************
int CSpeechToText::_CreateNewRule( int ruleID, bool bClearRule )
{
	HRESULT hr = E_FAIL;

	SPSTATEHANDLE	dynRuleHandle;
	// Create a dynamic rule containing the description strings of the voice tokens
	hr = m_cpCmdGrammar->GetRule(NULL, ruleID, 
								 SPRAF_TopLevel | SPRAF_Active | SPRAF_Dynamic, 
								 true, 
								 &dynRuleHandle);

	int iReturn = 1;
	if (hr == SPERR_RULE_NOT_FOUND)
	{
		iReturn = -1;
	}
	if (hr == SPERR_RULE_NAME_ID_CONFLICT)
	{
		iReturn = -2;
	}
	if (hr == E_INVALIDARG)
	{
		iReturn = -3;
	}
	if (hr == E_OUTOFMEMORY)
	{
		iReturn = -4;
	}

	m_vecCreatedRuleID.push_back(ruleID);

	if ( SUCCEEDED( hr ) )
	{
		// Clear the rule first
		if (bClearRule)
		{
			hr = m_cpCmdGrammar->ClearRule( dynRuleHandle );
		}

		if ( SUCCEEDED(hr) )
		{
			// Commit the changes
			hr = m_cpCmdGrammar->Commit(0);
		}
		else
			iReturn = 0;
	}
	else
		iReturn = 0;


	return iReturn;
}

bool CSpeechToText::_AddWordsIntoNewRule( int ruleID, WCHAR * words )
{
	HRESULT hr = E_FAIL;

	while (1)
	{
		SPSTATEHANDLE	dynRuleHandle;
		// find the specific rule
		hr = m_cpCmdGrammar->GetRule(NULL, ruleID, 
			SPRAF_TopLevel | SPRAF_Active | SPRAF_Dynamic, 
			false, 
			&dynRuleHandle);
		if (FAILED(hr) )
		{
			break; 
		}

		SPPROPERTYINFO prop;
		prop.pszName = L"Id";
		prop.pszValue = L"Property";
		prop.vValue.vt = VT_I4;
		prop.vValue.ulVal = BEGIN_ID++;
		hr = m_cpCmdGrammar->AddWordTransition( dynRuleHandle, NULL, words, L" ",
			SPWT_LEXICAL, 1.0, &prop);                  

		if (FAILED(hr) )
		{
			break; 
		}

		// store information(i.e words) of the new rule
		std::wstring strTemp(words);
		NEWRULEINFO ruleInfo(ruleID, prop.vValue.ulVal, strTemp);
		m_vecRuleInfo.push_back(ruleInfo);

		// Commit the changes
		hr = m_cpCmdGrammar->Commit(0);
		
		if (FAILED(hr) )
		{
			break; 
		}

		break;
	}

	return SUCCEEDED(hr);
}

bool CSpeechToText::_DeleteWordsFromRule( int ruleID, WCHAR * words )
{
	bool bSuccess = false;

	return bSuccess;
}

bool CSpeechToText::_DeleteCreatedRule( int ruleID )
{
	bool bSuccess = false;

	HRESULT hr = E_FAIL;

	hr = this->_DisableRule(ruleID);

	bSuccess = SUCCEEDED(hr);

	SPSTATEHANDLE	dynRuleHandle;
	
	// find the specific rule
	hr = m_cpCmdGrammar->GetRule(NULL, ruleID, 
		SPRAF_TopLevel | SPRAF_Active | SPRAF_Dynamic, 
		false, 
		&dynRuleHandle);

	bSuccess = SUCCEEDED(hr);

	m_cpCmdGrammar->ClearRule(dynRuleHandle);

	// clear the array
	if (bSuccess == true)
	{
		std::vector<int>::iterator it = m_vecCreatedRuleID.begin();
		for ( ; it != m_vecCreatedRuleID.end() ; it++)
		{
			if (ruleID == *it)
			{
				m_vecCreatedRuleID.erase(it);
				bSuccess = true;
				break;
			}
		}
	}

	return bSuccess;
}