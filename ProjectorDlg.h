#pragma once

#include "resource.h"

class C3DObject;
class C3DGlyph;
class C3DWord;
class C3DStar;

#include "3DProjectorimpl.h"
#include "AudioPlayer.h"

class CProjectorDlg
	: public CDialogImpl<CProjectorDlg>
	, public C3DProjectorImpl
	, protected local_utils::CMyThread
	, protected CAudioPlayer
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
		CHAIN_MSG_MAP(C3DProjectorImpl)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND, LPARAM);
	void	OnClose(UINT, int, HWND);
	void	OnTimer(UINT_PTR);

protected:
	void	PolyDraw(uint8_t* frameBuffer, const PolyPoint* polyPoints, size_t count, double r, double g, double b, double alpha) override;
	void	OnEvent() override;
	void	OnPlayAudio(uint8_t* pStream, ULONG dwLen) override;

private:
	void	StopAudio();
	bool	CreateScene();
	void	MoveScene(int nSteps = 1);
	void	PlayPause();
	void	AddTextBlock(std::wstring strTextBlock, bool bCenter, glm::dvec3& posLeftBorder, const glm::dvec3& posRightBorder, const glm::dvec3& boundsSpace, double lfVOffsetFactor = 1.5);

	void	RenderImGui();
	int		GetProgress() const;

	void	RandomizeStar(std::shared_ptr<C3DStar>& star);

public:
	UINT	GetScrollSpeed() const;
	void	SetScrollSpeed(UINT v);
	UINT	GetFlightSpeed() const;
	void	SetFlightSpeed(UINT v);

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
	std::atomic_int										m_nFrameCounter{ 0 };
	int													m_nFramesTotal;

	__declspec(property(get=GetProgress))		int		m_nProgress;

	const glm::dvec3									m_vecUp;
	glm::dvec4											m_posCam;
	glm::dvec4											m_posView;

	glm::dmat4											m_matScroll;
	glm::dmat4											m_matFlight;

	std::list< std::shared_ptr<C3DWord> >				m_listWords;
	std::list< std::shared_ptr<C3DStar> >				m_listStars;

	class frame_buffer_t
	{
	public:
		frame_buffer_t(int nWidth, int nHeight, int nBitplaneCount);
		~frame_buffer_t();

		operator uint8_t *()
		{
			return m_pFrameBuffer;
		}

	public:
		const SIZE		sz;
		const int		bits;
		HBITMAP			hbmp;
		uint8_t*		m_pFrameBuffer;
	};

	local_utils::CMyQueue< std::shared_ptr< frame_buffer_t > >		m_qInput;
	local_utils::CMyQueue< std::shared_ptr< frame_buffer_t > >		m_qOutput;

	CHandle															m_hVideoEncoder;
	CHandle															m_hVideoData;

	CHandle															m_hAudioDecoder;

	local_utils::CTemporaryFile										m_fileTemp;
	std::atomic_bool												m_bPaused{ false };
	UINT															m_nRandSeed;
};

