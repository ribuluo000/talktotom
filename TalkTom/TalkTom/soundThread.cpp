#include "stdafx.h"
#include "soundThread.h"
#include "SpeechToText.h"
#include "TextToRead.h"

#define MS_MYMESSAGE 0x400

extern CGlobal g_global;

unsigned __stdcall soundThread(void * p)
{
	CSpeechToText SpeechToText(L"..\\data\\command_chinese.xml");

	if (!SpeechToText._IsSuccess())
	{
		MessageBox(NULL, L"Sound system initialization fail!", L"ERROR", MB_OK);
		return 0;
	}

	asdffsda

	// create message queue
	MSG msg;
	PeekMessage(&msg, NULL, MS_MYMESSAGE, MS_MYMESSAGE, PM_NOREMOVE);

	// wait until we get the info of the marker
	WaitForSingleObject(g_global.hWaitForMarker, INFINITE);

	HWND hWnd = FindWindow(NULL, L"CamShift Adjust Window");
	SetWindowText(hWnd, L"asdf");

	bool b = false;
	b = SpeechToText._NotifyWindowMessage(hWnd, MS_MYMESSAGE);
	SpeechToText._RegisterMessageFunc(SPEI_SOUND_START, FuncSoundStart, NULL);
	SpeechToText._RegisterMessageFunc(SPEI_RECOGNITION, FuncFind , NULL);

	ResetEvent(g_global.hWaitForMarker);

	while (1)
	{
		// wait until we get the info of the marker
		WaitForSingleObject(g_global.hWaitForMarker, INFINITE);

		// we can mix the sound system now
		GetMessage(&msg, hWnd, 0, 0x1000);

		printf("deal message!\n");

		SpeechToText._DealMessage(msg.message);

		ResetEvent(g_global.hWaitForMarker);
	}


	return 0;
}