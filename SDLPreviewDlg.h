#pragma once

#include "resource.h"

class C3DObject;
class C3DGlyph;
class C3DWord;
class C3DStar;

#include "3DProjector.h"
#include "AudioPlayer.h"

class CSDLPreviewDlg : public CDialogImpl<CSDLPreviewDlg>, public C3DProjector, protected CMyThread, protected CAudioPlayer
{
public:
	CSDLPreviewDlg();
	~CSDLPreviewDlg();

	enum { IDD = IDD_PREVIEW };

	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnClose)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnClose)
		MSG_WM_TIMER(OnTimer)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND, LPARAM);
	void	OnClose(UINT, int, HWND);
	void	OnTimer(UINT_PTR);

protected:
	virtual void PolyDraw(const PIXEL2D* lppt, const BYTE* lpbTypes, size_t n, double r, double g, double b, double alpha);
	virtual void OnEvent();
	virtual void OnPlayAudio(BYTE* pStream, DWORD dwLen);

private:
	void	CloseSDL();
	bool	CreateScene();
	void	AddTextBlock(std::wstring strTextBlock, bool bCenter, glm::dvec3& posLeftBorder, const glm::dvec3& posRightBorder, const glm::dvec3& boundsSpace, double lfVOffsetFactor = 1.5);

public:
	int													m_nWidth;
	int													m_nHeight;
	std::wstring										m_strHeadline;
	std::wstring										m_strMessage;
	double												m_lfViewAngle;
	LOGFONT												m_LogFont;
	double												m_lfFontScale;
	double												m_lfScrollSpeed;
	double												m_lfFlightSpeed;
	size_t												m_nStarCount;
	bool												m_bExportMode;
	std::wstring										m_strExportFile;
	std::wstring										m_strAudioFile;
	bool												m_bWithAudio;
	bool												m_bSuccess;
	double												m_lfVideoLenInSeconds;
	double												m_lfFramesPerSecond;
	std::wstring										m_strVCodec;

private:
	CProgressBarCtrl									m_ctlProgress;
	SDL_Window*											m_pSDLWindow;
	SDL_Renderer*										m_pSDLRenderer;
	SDL_Texture*										m_pSDLTexture;

	int													m_nBitPlaneCount;
	int													m_nFrameCounter;
	int													m_nFramesTotal;

	double												m_lfSceneProgress;
	double												m_lfSceneStep;
	double												m_lfFadeOutMarker;

	glm::dvec4											m_posCam;
	glm::dvec4											m_posView;

	glm::dmat4											m_matScroll;
	glm::dmat4											m_matFlight;
	glm::dvec3											m_vecUp;

	double												m_lfStarFieldDistance;

	std::list< shared_ptr<C3DWord> >					m_listWords;
	std::list< shared_ptr<C3DStar> >					m_listStars;

	typedef  CTempBuffer<uint8_t>  frame_buffer_t;

	CMyQueue< std::shared_ptr< frame_buffer_t > >		m_qInput;
	CMyQueue< std::shared_ptr< frame_buffer_t > >		m_qOutput;

	frame_buffer_t*										m_pCurrentFrameBuffer;

	CHandle												m_hVideoEncoder;
	CHandle												m_hVideoData;

	CHandle												m_hAudioDecoder;

	CTemporaryFile										m_fileTemp;
};

