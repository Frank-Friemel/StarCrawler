// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"
#include "ProjectorDlg.h"

CMainDlg::CMainDlg()
{
	m_pProjectorDlg	= new CProjectorDlg;
}

CMainDlg::~CMainDlg()
{
	if (m_pProjectorDlg)
		delete m_pProjectorDlg;
}

void CMainDlg::BringToFront()
{
	SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	MSG	msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	ParametersToControls();

	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnCancel(wNotifyCode, wID, hWndCtl, bHandled);
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

void CMainDlg::ParametersToControls()
{
	CAVCoder coder;

	GetDlgItem(IDC_EXPORT).EnableWindow(!coder.GetCoderPath().empty());

	SetDlgItemText(IDC_HEADLINE, m_pProjectorDlg->m_strHeadline.c_str());
	SetDlgItemText(IDC_MESSAGE, m_pProjectorDlg->m_strMessage.c_str());

	SetDlgItemInt(IDC_WIDTH			, m_pProjectorDlg->m_nWidth);
	SetDlgItemInt(IDC_HEIGHT		, m_pProjectorDlg->m_nHeight);
	SetDlgItemInt(IDC_HEIGHT		, m_pProjectorDlg->m_nHeight);
	SetDlgItemInt(IDC_ANGLE			, (UINT)m_pProjectorDlg->m_lfViewAngle);
	SetDlgItemInt(IDC_VIDEO_LEN		, (UINT)m_pProjectorDlg->m_lfVideoLenInSeconds);
	SetDlgItemInt(IDC_FPS			, (UINT)m_pProjectorDlg->m_lfFramesPerSecond);
	SetDlgItemInt(IDC_SCROLL_SPEED	, m_pProjectorDlg->GetScrollSpeed());
	SetDlgItemInt(IDC_FLIGHT_SPEED	, m_pProjectorDlg->GetFlightSpeed());
	SetDlgItemInt(IDC_STAR_COUNT	, (UINT)m_pProjectorDlg->m_nStarCount);
	SetDlgItemText(IDC_FONT			, m_pProjectorDlg->m_LogFont.lfFaceName);
	SetDlgItemText(IDC_AUDIO_FILE	, m_pProjectorDlg->m_strAudioFile.c_str());
	CheckDlgButton(IDC_AUDIO		, m_pProjectorDlg->m_bWithAudio ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_HEADUP_DISPLAY, m_pProjectorDlg->m_bHeadupDisplay ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_AUDIO_FILE).EnableWindow(m_pProjectorDlg->m_bWithAudio);

	// h264_qsv;libx264;libopenh264;mpeg4;
	CComboBox ctlCodecs = GetDlgItem(IDC_CODEC);

	ctlCodecs.ResetContent();
	ctlCodecs.AddString(L"h264_qsv");
	ctlCodecs.AddString(L"libx264");
	ctlCodecs.AddString(L"libopenh264");
	ctlCodecs.AddString(L"mpeg4");

	SetDlgItemText(IDC_CODEC		, m_pProjectorDlg->m_strVCodec.c_str());

	CComboBox ctlAudioTracks = GetDlgItem(IDC_AUDIO_FILE);
	
	if (ctlAudioTracks.GetCount() == 0)
		ctlAudioTracks.Dir(0, L"*.mp3");
}

void CMainDlg::ParametersFromControls()
{
	CSimpleString strValue(CAtlStringMgr::GetInstance());

	GetDlgItemText(IDC_HEADLINE, strValue);
	m_pProjectorDlg->m_strHeadline			= (PCWSTR) strValue;

	GetDlgItemText(IDC_MESSAGE, strValue);
	m_pProjectorDlg->m_strMessage				= (PCWSTR)strValue;

	m_pProjectorDlg->m_nWidth					= GetDlgItemInt(IDC_WIDTH);
	m_pProjectorDlg->m_nHeight				= GetDlgItemInt(IDC_HEIGHT);
	m_pProjectorDlg->m_lfViewAngle			= (float)GetDlgItemInt(IDC_ANGLE);
	m_pProjectorDlg->m_lfVideoLenInSeconds	= (double)GetDlgItemInt(IDC_VIDEO_LEN);
	m_pProjectorDlg->m_lfFramesPerSecond		= (double)GetDlgItemInt(IDC_FPS);
	m_pProjectorDlg->SetScrollSpeed(GetDlgItemInt(IDC_SCROLL_SPEED));
	m_pProjectorDlg->SetFlightSpeed(GetDlgItemInt(IDC_FLIGHT_SPEED));
	m_pProjectorDlg->m_nStarCount				= (int)GetDlgItemInt(IDC_STAR_COUNT);
	m_pProjectorDlg->m_bWithAudio				= IsDlgButtonChecked(IDC_AUDIO) == BST_CHECKED ? true : false;
	m_pProjectorDlg->m_bHeadupDisplay			= IsDlgButtonChecked(IDC_HEADUP_DISPLAY) == BST_CHECKED ? true : false;

	GetDlgItemText(IDC_AUDIO_FILE, strValue);
	m_pProjectorDlg->m_strAudioFile			= (PCWSTR)strValue;

	GetDlgItemText(IDC_CODEC, strValue);
	m_pProjectorDlg->m_strVCodec				= (PCWSTR)strValue;
}

void CMainDlg::OnClickedAudio(UINT, int, HWND)
{
	m_pProjectorDlg->m_bWithAudio				= IsDlgButtonChecked(IDC_AUDIO) == BST_CHECKED ? true : false;
	GetDlgItem(IDC_AUDIO_FILE).EnableWindow(m_pProjectorDlg->m_bWithAudio);
}

void CMainDlg::OnFontSel(UINT, int, HWND)
{
	CFontDialog dlg(&m_pProjectorDlg->m_LogFont, CF_TTONLY | CF_USESTYLE | CF_SCREENFONTS);

	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		dlg.GetCurrentFont(&m_pProjectorDlg->m_LogFont);
		SetDlgItemText(IDC_FONT, m_pProjectorDlg->m_LogFont.lfFaceName);
	}
}

void CMainDlg::OnPreview(UINT, int, HWND)
{
	ParametersFromControls();

	m_pProjectorDlg->m_bExportMode = false;
	m_pProjectorDlg->DoModal(m_hWnd);

	ParametersToControls();

	BringToFront();
}

void CMainDlg::OnExport(UINT, int, HWND)
{
	CFilePicker file_picker(OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT);

	file_picker.SetDefExt(L".mp4");

	WTL::CString strLabel(L"Export File");

	WTL::CString strPoject(L"MPEG4 File");
	WTL::CString strMask = WTL::CString(L"*") + WTL::CString(L".mp4");

	file_picker.SetPathName(m_pProjectorDlg->m_strExportFile.c_str());

	if (file_picker.Save(strLabel, 1, strPoject, strMask))
	{
		ParametersFromControls();

		m_pProjectorDlg->m_bExportMode	= true;
		m_pProjectorDlg->m_strExportFile	= file_picker.GetPathName();

		if (			 m_pProjectorDlg->m_strExportFile.length() < 4
			||	_wcsicmp(m_pProjectorDlg->m_strExportFile.substr(m_pProjectorDlg->m_strExportFile.length()-4).c_str(), L".mp4") != 0)
		{
			m_pProjectorDlg->m_strExportFile += L".mp4";

			if (PathFileExists(m_pProjectorDlg->m_strExportFile.c_str()))
			{
				if (MessageBox(L"File exists. Overwrite?", L"Question", MB_YESNO | MB_ICONQUESTION) != IDYES)
					return;
			}
		}
		m_pProjectorDlg->DoModal(m_hWnd);

		if (m_pProjectorDlg->m_bSuccess)
		{ 
			if (MessageBox(L"Export succeeded. Navigate to file?", L"Success", MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{
				std::wstring strExec(L"explorer.exe /select,\"");

				strExec += m_pProjectorDlg->m_strExportFile;
				strExec += L"\"";

				Exec(strExec.c_str(), 0, false, NULL, SW_SHOWNORMAL);
			}
		}
		else
			::DeleteFile(m_pProjectorDlg->m_strExportFile.c_str());
	}
	BringToFront();
}
