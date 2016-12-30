// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "3DProjector.h"
#include "3DObject.h"

class CSDLPreviewDlg;


class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
	CMainDlg();
	~CMainDlg();

	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP_EX(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(IDC_PREVIEW, OnPreview)
		COMMAND_ID_HANDLER_EX(IDC_EXPORT, OnExport)
		COMMAND_ID_HANDLER_EX(IDC_FONT_SEL, OnFontSel)
		COMMAND_ID_HANDLER_EX(IDC_AUDIO, OnClickedAudio)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void	OnPreview(UINT, int, HWND);
	void	OnExport(UINT, int, HWND);
	void	OnFontSel(UINT, int, HWND);
	void	OnClickedAudio(UINT, int, HWND);

private:
	void BringToFront();
	void ParametersFromControls();
	void ParametersToControls();

private:
	CSDLPreviewDlg* m_pPreviewDlg;
};
