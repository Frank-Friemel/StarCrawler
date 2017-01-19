#pragma once

#include "resource.h"

class C3DObject;
class C3DGlyph;
class C3DWord;
class C3DStar;

#include "3DProjector.h"
#include "AudioPlayer.h"

class CProjectorDlg : public CDialogImpl<CProjectorDlg>, public C3DProjector, protected CMyThread, protected CAudioPlayer
{
public:
	CProjectorDlg();
	~CProjectorDlg();

	enum { IDD = IDD_PREVIEW };

	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnClose)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnClose)
		MSG_WM_TIMER(OnTimer)
		CHAIN_MSG_MAP(C3DProjector)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND, LPARAM);
	void	OnClose(UINT, int, HWND);
	void	OnTimer(UINT_PTR);

protected:
	virtual void PolyDraw(const PIXEL2D* lppt, const BYTE* lpbTypes, size_t n, double r, double g, double b, double alpha);
	virtual void OnEvent();
	virtual void OnPlayAudio(BYTE* pStream, ULONG dwLen);

private:
	void	StopAudio();
	bool	CreateScene();
	void	MoveScene(int nSteps = 1);
	void	PlayPause();
	void	AddTextBlock(std::wstring strTextBlock, bool bCenter, glm::dvec3& posLeftBorder, const glm::dvec3& posRightBorder, const glm::dvec3& boundsSpace, double lfVOffsetFactor = 1.5);

	void	RenderImGui();
	int		GetProgress() const
	{
		auto nFrameCount = m_nFrameCounter.get();

		if (nFrameCount >= m_nFramesTotal)
			return 100;
		return (nFrameCount * 100) / m_nFramesTotal;
	}
public:
	UINT	GetScrollSpeed() const
	{
		return static_cast<UINT>(m_lfScrollSpeed*100.0);
	}
	void	SetScrollSpeed(UINT v)
	{
		m_lfScrollSpeed = v / 100.0;
		m_matScroll		= glm::translate(glm::dvec3(0, 0, m_lfScrollSpeed));
	}
	UINT	GetFlightSpeed() const
	{
		return static_cast<UINT>(m_lfFlightSpeed*(-100.0));
	}
	void	SetFlightSpeed(UINT v)
	{
		m_lfFlightSpeed = v / (-100.0);
		m_matFlight		= glm::translate(glm::dvec3(0, 0, m_lfFlightSpeed));
	}
public:
	int													m_nWidth;
	int													m_nHeight;
	std::wstring										m_strHeadline;
	std::wstring										m_strMessage;
	float												m_lfViewAngle;
	LOGFONT												m_LogFont;
	double												m_lfFontScale;
	int													m_nStarCount;
	bool												m_bExportMode;
	bool												m_bHeadupDisplay; // ImGui Controls (yes/no)
	std::wstring										m_strExportFile;
	std::wstring										m_strAudioFile;
	bool												m_bWithAudio;
	bool												m_bSuccess;
	double												m_lfVideoLenInSeconds;
	double												m_lfFramesPerSecond;
	std::wstring										m_strVCodec;

private:
	double												m_lfScrollSpeed;
	double												m_lfFlightSpeed;
	CProgressBarCtrl									m_ctlProgress;

	int													m_nBitPlaneCount;
	interlocked_t										m_nFrameCounter;
	int													m_nFramesTotal;

	__declspec(property(get=GetProgress))		int		m_nProgress;

	glm::dvec4											m_posCam;
	glm::dvec4											m_posView;

	glm::dmat4											m_matScroll;
	glm::dmat4											m_matFlight;
	glm::dvec3											m_vecUp;

	std::list< shared_ptr<C3DWord> >					m_listWords;
	std::list< shared_ptr<C3DStar> >					m_listStars;

	class frame_buffer_t
	{
	public:
		frame_buffer_t(int nWidth, int nHeight, int nBitplaneCount);
		~frame_buffer_t();

		operator unsigned char *()
		{
			return m_pFrameBuffer;
		}
	public:
		SIZE		sz;
		int			bits;
		HBITMAP		hbmp;
		BYTE*		m_pFrameBuffer;
	};

	CMyQueue< std::shared_ptr< frame_buffer_t > >		m_qInput;
	CMyQueue< std::shared_ptr< frame_buffer_t > >		m_qOutput;

	CHandle												m_hVideoEncoder;
	CHandle												m_hVideoData;

	CHandle												m_hAudioDecoder;

	CTemporaryFile										m_fileTemp;
	interlocked_t										m_bPaused;
	UINT												m_nRandSeed;
};

