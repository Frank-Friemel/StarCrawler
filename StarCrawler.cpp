// StarCrawler.cpp : main source file for StarCrawler.exe
//

#include "stdafx.h"

#include "resource.h"

#include "MainDlg.h"

CAppModule _Module;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = 0;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) >= 0)
	{
		CMainDlg dlgMain;
		nRet = (int) dlgMain.DoModal();

		SDL_Quit();
	}
	else
	{
		::MessageBox(NULL, L"Failed to initialize SDL", L"Error", MB_OK|MB_ICONEXCLAMATION);
	}

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
