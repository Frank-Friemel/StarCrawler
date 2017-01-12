#include "stdafx.h"
#include "SDLPreviewDlg.h"

#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_rgb.h"
#include "agg_renderer_base.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_path_storage.h"
#include "agg_conv_curve.h"
#include "agg_conv_contour.h"
#include "agg_conv_bspline.h"

#include "3DObject.h"
#include "3DGlyph.h"
#include "3DWord.h"
#include "3DStar.h"

#define TIMER_ID 9999

CSDLPreviewDlg::CSDLPreviewDlg()
{
	m_pSDLWindow			= NULL;
	m_pSDLRenderer			= NULL;
	m_pSDLTexture			= NULL;

	m_nWidth				= 1280;
	m_nHeight				= 720;
	m_lfViewAngle			= 20;
	m_strHeadline			= L"StarCrawler Demo";
	m_strMessage			= L"Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";
	m_lfFontScale			= 0.5;
	m_lfScrollSpeed			= 0.02;
	m_lfFlightSpeed			= -0.1;
	m_lfVideoLenInSeconds	= 45.0;
	m_lfFramesPerSecond		= 25;
	m_lfSceneProgress		= 0;
	m_lfSceneStep			= 0;
	m_strVCodec				= L"libopenh264";

	memset(&m_LogFont, 0, sizeof(m_LogFont));

	m_LogFont.lfHeight		= 48;
	m_LogFont.lfWeight		= FW_NORMAL;
	m_LogFont.lfOutPrecision= OUT_TT_ONLY_PRECIS;
	m_LogFont.lfQuality		= ANTIALIASED_QUALITY;

	wcscpy_s(m_LogFont.lfFaceName, _countof(m_LogFont.lfFaceName), L"Arial Narrow");

	// begin to fade out scene at 90%
	m_lfFadeOutMarker		= 0.9; 

	m_lfStarFieldDistance	= -80.0;
	m_nStarCount			= 300;
	m_bExportMode			= false;
	m_nFrameCounter			= 0;
	m_nFramesTotal			= (int)(m_lfVideoLenInSeconds * m_lfFramesPerSecond);
	m_nBitPlaneCount		= m_bExportMode ? 3 : 4;

	m_strExportFile			= L"StarCrawler.mp4";
	m_strAudioFile			= L"TestTrack01.mp3";
	m_bWithAudio			= true;
	m_bSuccess				= false;
}


CSDLPreviewDlg::~CSDLPreviewDlg()
{
}

void CSDLPreviewDlg::CloseSDL()
{
	if (m_hAudioDecoder)
	{
		CAudioPlayer::Close();

		::WaitForSingleObject(m_hAudioDecoder, INFINITE);
		m_hAudioDecoder.Close();
	}
	if (m_pSDLWindow)
	{
		SDL_DestroyWindow(m_pSDLWindow);
		SDL_DestroyRenderer(m_pSDLRenderer);
		SDL_DestroyTexture(m_pSDLTexture);

		m_pSDLWindow	= NULL;
		m_pSDLRenderer	= NULL;
		m_pSDLTexture	= NULL;
	}
}

void CSDLPreviewDlg::OnPlayAudio(BYTE* pStream, DWORD dwLen)
{
	if (dwLen)
	{
		memset(pStream, 0, dwLen);

		int		nTry		= 5;
		DWORD	dwRead		= 0;
		DWORD	dwReadTotal	= 0;

		CTempBuffer<BYTE> buf(dwLen);

		while (nTry > 0 && dwLen && ::ReadFile(m_hAudioData, buf + dwReadTotal, dwLen, &dwRead, NULL))
		{
			if (dwRead == 0)
			{ 
				nTry--;
				Sleep(5);
			}
			else
			{
				// the input handle may be a pipe ... give it some time to deliver the data (over all 25ms)
				nTry		= 5;
				dwLen		-= dwRead;
				dwReadTotal += dwRead;
			}
		}
		if (dwReadTotal)
		{
			if (m_lfSceneProgress > m_lfFadeOutMarker)
			{
				// fade sound after we've reached 90%
				int nVolume = (int)(((double)SDL_MIX_MAXVOLUME) * ((1.0 - m_lfSceneProgress) * 10.0));

				if (nVolume > SDL_MIX_MAXVOLUME)
					nVolume = SDL_MIX_MAXVOLUME;
				if (nVolume < 0)
					nVolume = 0;

				SDL_MixAudio(pStream, buf, dwReadTotal, nVolume);
			}
			else
			{
				memcpy(pStream, buf, dwReadTotal);
			}
		}
	}
}

LRESULT CSDLPreviewDlg::OnInitDialog(HWND, LPARAM)
{
	USES_CONVERSION;

	m_lfSceneProgress		= 0;
	m_nFrameCounter			= 0;
	m_bSuccess				= false;
	m_nBitPlaneCount		= m_bExportMode ? 3 : 4;
	m_nFramesTotal			= (int)(m_lfVideoLenInSeconds * m_lfFramesPerSecond);
	m_lfSceneStep			= 1.0 / (m_lfFramesPerSecond * m_lfVideoLenInSeconds);
	m_pCurrentFrameBuffer	= NULL;

	Prepare(CSize(m_nWidth, m_nHeight), m_nBitPlaneCount);

	m_ctlProgress.Attach(GetDlgItem(IDC_PROGRESS));

	if (!m_bExportMode)
	{
		OutputDebugStringW(L"Starting in Preview Mode\n");

		// no progress bar in preview mode
		m_ctlProgress.ShowWindow(SW_HIDE);

		CSimpleString strTitle(CAtlStringMgr::GetInstance());
		this->GetWindowText(strTitle);

		m_pSDLWindow = SDL_CreateWindowFrom(m_hWnd);

		SDL_SetWindowTitle(m_pSDLWindow, T2CA(strTitle));
		SDL_SetWindowSize(m_pSDLWindow, m_nWidth, m_nHeight);
		SDL_ShowWindow(m_pSDLWindow);

		InitImGui(m_hWnd);

		// center the dialog on the screen
		CenterWindow(GetParent());

		m_pSDLRenderer	= SDL_CreateRenderer(m_pSDLWindow, -1, SDL_RENDERER_ACCELERATED);
		m_pSDLTexture	= SDL_CreateTexture(m_pSDLRenderer, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STREAMING, m_nWidth, m_nHeight);

		if (m_bWithAudio)
		{
			CHandle		hPipe;
			CAVCoder	decoder;

			m_hAudioDecoder.Attach(decoder.DecodeAudioToPipe(m_strAudioFile, &hPipe.m_h));

			if (m_hAudioDecoder)
			{
				ATLASSERT(hPipe);

				if (!CAudioPlayer::Init(hPipe))
				{
					MessageBox(L"Failed to open audio device\n", L"Error", MB_ICONEXCLAMATION | MB_OK);
					hPipe.Close();
					m_hAudioDecoder.Close();
				}
			}
			else
			{
				MessageBox(L"Failed to start audio decoder\n", L"Error", MB_ICONEXCLAMATION | MB_OK);
				m_bWithAudio = false;
			}
		}
	}
	else
	{
		OutputDebugStringW(L"Starting in Export Mode\n");

		// center the dialog on the screen
		CenterWindow(GetParent());

		SetWindowText(L"Export");

		m_ctlProgress.SetRange32(0, m_nFramesTotal);
		m_ctlProgress.SetPos(m_nFrameCounter);

		ATLASSERT(!m_strExportFile.empty());

		if (m_bWithAudio)
		{ 
			// we'll have to decode the audio file to .wav before we're merging audio and video together
			CWaitCursor wc;
			
			CTemporaryFile fileTemp;

			// organize a temporary file
			if (FAILED(fileTemp.Create()))
			{
				MessageBox(L"Failed to create temporary PCM file", L"Error", MB_OK | MB_ICONEXCLAMATION);
				PostMessage(WM_COMMAND, IDOK);
				return FALSE;
			}
			// got it - but we need the file name only
			fileTemp.Close();

			CAVCoder	decoder;

			// decode whatever format to raw PCM temporary file
			CHandle hTempAudioDecoder(decoder.DecodeAudioToFile(m_strAudioFile, fileTemp.m_strTempFileName, true));

			if (!hTempAudioDecoder)
			{
				MessageBox(L"Failed to create temporary audio decoder", L"Error", MB_OK | MB_ICONEXCLAMATION);
				PostMessage(WM_COMMAND, IDOK);
				return FALSE;
			}
			// wait for the decoder to finish
			if (::WaitForSingleObject(hTempAudioDecoder, 60000) == WAIT_TIMEOUT)
			{
				MessageBox(L"Failed to decode audio file", L"Error", MB_OK | MB_ICONEXCLAMATION);
				PostMessage(WM_COMMAND, IDOK);
				return FALSE;
			}
			// open the raw PCM stream
			if (FAILED(fileTemp.Open()))
			{
				MessageBox(L"Failed to open temporary PCM file", L"Error", MB_OK | MB_ICONEXCLAMATION);
				PostMessage(WM_COMMAND, IDOK);
				return FALSE;
			}
			CWaveHeader wh;

			ULONGLONG sizePCM = 0;
			ATLVERIFY(SUCCEEDED(fileTemp.m_file.GetSize(sizePCM)));

			if (sizePCM)
			{ 
				wh.update((uint32_t)sizePCM);
				ATLASSERT(wh.isValid());

				// calc the size we need
				sizePCM = (ULONGLONG)(BYTES_PER_SEC * (m_lfVideoLenInSeconds + 0.1));
					
				// align size to sample border
				ALIGN_DOWN_TO_SAMPLE(sizePCM);

				// calc offset where we're fading out
				ULONGLONG offFade = (ULONGLONG)(m_lfFadeOutMarker * sizePCM);

				// align offset to sample border
				ALIGN_DOWN_TO_SAMPLE(offFade);

				ATLTRACE(L"the wave contains %lu PCM data -> %.02lf seconds\n", wh.getPCMBytes(), wh.getPCMSeconds());

				m_fileTemp.m_bDeleteOnClose = false;

				if (FAILED(m_fileTemp.Create()))
				{
					MessageBox(L"Failed to create temporary WAV file", L"Error", MB_OK | MB_ICONEXCLAMATION);
					PostMessage(WM_COMMAND, IDOK);
					return FALSE;
				}
				if (wh.getPCMBytes() < sizePCM)
				{
					// update wave header to the new stream size
					wh.update((uint32_t)sizePCM);

					// write the wav header
					ATLVERIFY(SUCCEEDED(m_fileTemp.m_file.Write(&wh, wh.mySize())));

					double lfFadeTotal = 0;

					// enlarge and fade the audio stream
					while (sizePCM)
					{
						if (offFade > BYTES_PER_SEC)
						{
							ULONGLONG nShoveled = Shovel(fileTemp, m_fileTemp, (ULONGLONG) BYTES_PER_SEC);

							sizePCM -= nShoveled;
							offFade -= nShoveled;

							if (nShoveled < BYTES_PER_SEC)
								fileTemp.SeekToBegin();

							lfFadeTotal = (double) sizePCM;
						}
						else
						{
							double	lfVolume	= ((double)sizePCM) / lfFadeTotal;
							short	sample		= 0;

							for (auto i = 0; i < NUM_CHANNELS; ++i)
							{
								DWORD dwDummy = 0;

								// channel x
								ATLVERIFY(::ReadFile(fileTemp, &sample, sizeof(short), &dwDummy, NULL));
								sample = (short)(((double)sample) * lfVolume);
								ATLVERIFY(::WriteFile(m_fileTemp, &sample, sizeof(short), &dwDummy, NULL));
								
								sizePCM -= sizeof(short);
							}
						}
					}
				}
				else
				{
					// update wave header to the new stream size
					wh.update((uint32_t)sizePCM);
					
					// write the wav header
					ATLVERIFY(SUCCEEDED(m_fileTemp.m_file.Write(&wh, wh.mySize())));
					
					// copy 90% of the PCM stream over
					ATLVERIFY(Shovel(fileTemp, m_fileTemp, offFade) == offFade);

					// fade the remaining samples
					sizePCM -= offFade;

					ATLASSERT(sizePCM % (NUM_CHANNELS*sizeof(short)) == 0);
					sizePCM /= (NUM_CHANNELS*sizeof(short));

					// shorten and fade the audio stream
					short		sample			= 0;
					double		lfFadeTotal		= (double)sizePCM;

					while (sizePCM)
					{
						--sizePCM;

						double lfVolume = ((double)sizePCM) / lfFadeTotal;

						for (auto i = 0; i < NUM_CHANNELS; ++i)
						{
							DWORD dwDummy = 0;

							// channel x
							ATLVERIFY(::ReadFile(fileTemp, &sample, sizeof(short), &dwDummy, NULL));
							sample = (short) (((double)sample) * lfVolume);
							ATLVERIFY(::WriteFile(m_fileTemp, &sample, sizeof(short), &dwDummy, NULL));
						}
					}
				}
				m_fileTemp.Close();
			}
		}
		CAVCoder encoder;

		// start MP4 encoder
		m_hVideoEncoder.Attach(encoder.EncodeSlideShow(m_strExportFile, &m_hVideoData.m_h, (int)m_lfFramesPerSecond, m_fileTemp.m_strTempFileName, m_strVCodec.c_str()));

		if (!m_hVideoEncoder)
		{
			MessageBox(L"Failed to start Video Encoder", L"Error", MB_ICONEXCLAMATION | MB_OK);
			PostMessage(WM_COMMAND, IDOK);
			return FALSE;
		}
		ATLASSERT(m_hVideoData);
	}
	m_posCam	= glm::dvec4(0, 0, 10, 1);
	m_posView	= glm::dvec4(0, 0, 0, 1);

	glm::dvec3 myRotationAxis(1, 0, 0);
	glm::dmat4 matRot = glm::rotate((glm::dvec3::value_type)glm::radians(m_lfViewAngle), myRotationAxis);

	m_posCam = matRot * m_posCam;

	glm::dmat4 matTransStartOffset = glm::translate(glm::dvec3(0, 0, -10));

	m_posCam	= matTransStartOffset * m_posCam;
	m_posView	= matTransStartOffset * m_posView;
	m_matScroll = glm::translate(glm::dvec3(0, 0, m_lfScrollSpeed));
	m_matFlight = glm::translate(glm::dvec3(0, 0, m_lfFlightSpeed));
	m_vecUp		= glm::dvec3(0, 1, 0);

	m_matCamView = glm::lookAt(glm::dvec3(m_posCam.x, m_posCam.y, m_posCam.z), glm::dvec3(m_posView.x, m_posView.y, m_posView.z), m_vecUp);

	// Create 3D Objects ....
	if (CreateScene())
	{
		std::shared_ptr<frame_buffer_t> pFrameBuffer;
		
		// pre load input queue with 2 frame buffers
		for (int i = 0; i < 2; ++i)
		{
			pFrameBuffer = std::make_shared<frame_buffer_t>(m_nWidth * m_nHeight * m_nBitPlaneCount);
			m_qInput.queue(pFrameBuffer);
		}
		// build frame buffers in worker thread
		ATLVERIFY(CMyThread::Start(m_qInput.m_hEvent));

		// render output with x fps
		SetTimer(TIMER_ID, m_bExportMode ? 4 : (UINT)(1000.0 / m_lfFramesPerSecond));
	}
	else
	{
		PostMessage(WM_COMMAND, IDOK);
		MessageBox(L"Failed to create Scene\n", L"Error", MB_ICONEXCLAMATION|MB_OK);
	}
	return TRUE;
}

void CSDLPreviewDlg::OnClose(UINT, int wID, HWND)
{
	KillTimer(TIMER_ID);
	CMyThread::Stop();

	ShutdownImGui();
	CloseSDL();

	if (m_hVideoEncoder)
	{
		m_hVideoData.Close();
		::WaitForSingleObject(m_hVideoEncoder, INFINITE);
		m_hVideoEncoder.Close();

		m_fileTemp.Delete();
		m_fileTemp.Clear();
	}
	m_ctlProgress.Detach();

	m_qInput.clear();
	m_qOutput.clear();

	if (wID == IDCANCEL && m_bExportMode)
		MessageBox(L"The task has been canceled", L"Information", MB_OK | MB_ICONINFORMATION);
	EndDialog(wID);
}

void CSDLPreviewDlg::RenderImGui()
{
	ATLASSERT(!m_bExportMode);

	POINT ptMouse;
	GetCursorPos(&ptMouse);

	::MapWindowPoints(NULL, m_hWnd, &ptMouse, 1);

	NewFrameImGui(ptMouse);

	static bool show_test_window = true;
	static bool show_another_window = false;
	
	ImVec4 clear_color = ImColor(114, 144, 154);

	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		
		if (ImGui::Button("Test Window")) 
			show_test_window ^= 1;
		
		if (ImGui::Button("Another Window")) 
			show_another_window ^= 1;
		
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window)
	{
		ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}

	// Rendering
	C3DProjector::RenderImGui();
}

void CSDLPreviewDlg::OnEvent()
{
	// this worker thread constructs the frames
	std::shared_ptr<frame_buffer_t> pFrameBuffer;

	// whenever a frame buffer is available -> fill it
	while (m_qInput.unqueue(pFrameBuffer))
	{
		m_lfSceneProgress += m_lfSceneStep;

		// draw 3D scene to frame buffer
		m_pCurrentFrameBuffer = *pFrameBuffer;

		// the universe is "black"
		memset(m_pCurrentFrameBuffer, 0, m_nWidth * m_nHeight * m_nBitPlaneCount);

		// draw stars in background
		for each (auto& star in m_listStars)
		{
			star->Draw(this);
		}
		// draw the message  
		for each (auto& word in m_listWords)
		{
			word->Draw(this);
		}
		if (!m_bExportMode)
			RenderImGui();

		m_pCurrentFrameBuffer = NULL;

		// the frame is done -> queue frame buffer to main thread
		m_qOutput.queue(pFrameBuffer);

		// update 3D data of scene
		m_posCam	= m_matScroll * m_posCam;
		m_posView	= m_matScroll * m_posView;

		m_posCam	= m_matFlight * m_posCam;
		m_posView	= m_matFlight * m_posView;

		for each (auto& word in m_listWords)
		{
			word->Move(m_matFlight);
		}
		m_matCamView = glm::lookAt(glm::dvec3(m_posCam.x, m_posCam.y, m_posCam.z), glm::dvec3(m_posView.x, m_posView.y, m_posView.z), m_vecUp);

		// star field update
		for each (auto& star in m_listStars)
		{
			if (!star->IsVisible(this))
			{
				// revisit flight passed stars which gives the illusion of an infinite star field
				star->Randomize(this, star->m_vecVertices[0].z + m_lfStarFieldDistance);
				ATLASSERT(star->IsVisible(this));
			}
		}
		// fade out text message, after a while
		if (m_lfSceneProgress >= 0.50)
		{
			size_t n = m_listWords.size() * 25 / 100;

			for each (auto& word in m_listWords)
			{
				word->SetAlpha(word->GetAlpha() - 0.001);

				if (--n == 0)
					break;
			}
		}
		if (m_lfSceneProgress >= 0.55)
		{
			size_t n = m_listWords.size() * 50 / 100;

			for each (auto& word in m_listWords)
			{
				word->SetAlpha(word->GetAlpha() - 0.001);

				if (--n == 0)
					break;
			}
		}
		if (m_lfSceneProgress >= 0.60)
		{
			size_t n = m_listWords.size() * 75 / 100;

			for each (auto& word in m_listWords)
			{
				word->SetAlpha(word->GetAlpha() - 0.001);

				if (--n == 0)
					break;
			}
		}
		if (m_lfSceneProgress >= 0.65)
		{
			for each (auto& word in m_listWords)
			{
				word->SetAlpha(word->GetAlpha() - 0.001);
			}
		}
	}
}

void CSDLPreviewDlg::OnTimer(UINT_PTR)
{
	// the main thread displays the constructed frames
	if (m_nFrameCounter < m_nFramesTotal)
	{
		std::shared_ptr<frame_buffer_t> pFrameBuffer;

		// any frame available ...?
		if (::WaitForSingleObject(m_qOutput.m_hEvent, 5) != WAIT_TIMEOUT && m_qOutput.unqueue(pFrameBuffer))
		{
			// ...yes!
			++m_nFrameCounter;

			if (m_bExportMode)
			{
				// PPM format expects 3 color planes!
				ATLASSERT(m_nBitPlaneCount == 3);

				// export a single frame in PPM format
				if (!WritePPMData(m_hVideoData, *pFrameBuffer, m_nWidth, m_nHeight))
				{
					KillTimer(TIMER_ID);
					MessageBox(L"Failed to encode video\n", L"Error", MB_ICONEXCLAMATION | MB_OK);
					PostMessage(WM_COMMAND, IDOK);
					return;
				}
				// update the progress bar control
				m_ctlProgress.SetPos(m_nFrameCounter);
			}
			else
			{
				// we're in preview mode
				// start playing audio on the first frame
				if (m_nFrameCounter == 1 && m_hAudioDecoder)
				{
					CAudioPlayer::Play();
				}
				// let SDL render the frame buffer
				SDL_UpdateTexture(m_pSDLTexture, nullptr, *pFrameBuffer, m_nStride);

				SDL_RenderClear(m_pSDLRenderer);
				SDL_RenderCopy(m_pSDLRenderer, m_pSDLTexture, nullptr, nullptr);
				SDL_RenderPresent(m_pSDLRenderer);
			}
			if (m_nFrameCounter >= m_nFramesTotal)
			{
				// we're done - 100% reached
				KillTimer(TIMER_ID);
				m_bSuccess = true;
				PostMessage(WM_COMMAND, IDOK);
			}
			else
			{
				// queue frame buffer to be re-filled with pixels
				m_qInput.queue(pFrameBuffer);
			}
		}
		else if (!m_bExportMode)
		{
			WCHAR buf[256];
		
			swprintf_s(buf, 256, L"Missing frame at #%d / #%d\n", m_nFrameCounter, m_nFramesTotal);
			OutputDebugStringW(buf);
		}	
	}
}

void CSDLPreviewDlg::PolyDraw(const PIXEL2D* lppt, const BYTE* lpbTypes, size_t n, double r, double g, double b, double alpha)
{
	// beautiful agg polygon rendering ....
	agg::path_storage ps;
	agg::conv_curve<agg::path_storage> curve(ps);

	ps.remove_all();

	while (n--)
	{
		switch (*lpbTypes++)
		{
			case PT_MOVETO:
			{
				ps.vertices().add_vertex(lppt->x, lppt->y, agg::path_cmd_move_to);
			}
			break;

			case PT_LINETO:
			{
				ps.vertices().add_vertex(lppt->x, lppt->y, agg::path_cmd_line_to);
			}
			break;

			case PT_BEZIERTO:
			{
				ps.vertices().add_vertex(lppt->x, lppt->y, agg::path_cmd_curve3);
			}
			break;

			case PT_CCW_EX:
			{
				ps.vertices().add_vertex(lppt->x, lppt->y, agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw);
			}
			break;

			case PT_STOP:
			{
				ps.vertices().add_vertex(lppt->x, lppt->y, agg::path_cmd_stop);
			}
			break;

			default:
			{
				ATLASSERT(FALSE);
			}
			break;
		}
		++lppt;
	}
	ps.close_polygon();

	agg::scanline_u8				sl;
	agg::rasterizer_scanline_aa<>	ras;

	ras.gamma(agg::gamma_linear());
	ras.add_path(curve);

	agg::rendering_buffer rbuf(m_pCurrentFrameBuffer, m_nWidth, m_nHeight, m_nStride);

	// do some fading effect after we've reached 90% 
	if (m_lfSceneProgress >= m_lfFadeOutMarker)
		alpha *= ((1.0-m_lfSceneProgress) * 10.0);

	if (m_nBitPlaneCount == 4)
	{
		agg::pixfmt_rgba32						pixf(rbuf);
		agg::renderer_base<agg::pixfmt_rgba32>	ren(pixf);

		agg::render_scanlines_aa_solid(ras, sl, ren, agg::rgba(r, g, b, alpha));
	}
	else
	{
		agg::pixfmt_rgb24						pixf(rbuf);
		agg::renderer_base<agg::pixfmt_rgb24>	ren(pixf);

		agg::render_scanlines_aa_solid(ras, sl, ren, agg::rgba(r, g, b, alpha));
	}
}

bool CSDLPreviewDlg::CreateScene()
{
	m_listWords.clear();
	m_listStars.clear();

	C3DGlyph glyphSpace;

	if (!glyphSpace.Create(L'l', &m_LogFont))
		return false;

	glyphSpace.RotateX(90.0);
	glyphSpace.Scale(m_lfFontScale);

	glm::dvec3 boundsSpace = glyphSpace.GetBoundingBox();

	glm::dvec4	ptLowerLeft3D;
	PIXEL2D		ptLowerLeft2D;
	glm::dvec4	ptLowerRight3D;
	PIXEL2D		ptLowerRight2D;

	ptLowerLeft2D.x = 0;
	ptLowerLeft2D.y = m_nHeight  - 1;
	
	ptLowerRight2D.x = m_nWidth  - 1;
	ptLowerRight2D.y = m_nHeight - 1;

	PixelToVertexStaticY(0, ptLowerLeft3D, ptLowerLeft2D);
	PixelToVertexStaticY(0, ptLowerRight3D, ptLowerRight2D);

	glm::dvec3 posLeftBorder = ptLowerLeft3D;
	posLeftBorder.x += boundsSpace.x;

	glm::dvec3 posRightBorder = ptLowerRight3D;
	posRightBorder.x -= boundsSpace.x;

	// sprinkle some stars
	while (m_listStars.size() < m_nStarCount)
	{
		std::shared_ptr<C3DStar> pStar = std::make_shared<C3DStar>();

		pStar->Randomize(this, m_lfStarFieldDistance);

		m_listStars.push_back(pStar);
	}
	// calc start position for the scroll text
	posLeftBorder.z += (fabs(boundsSpace.z) * 1.5 + m_bExportMode ? 0.9 : 0.0);

	if (!m_strHeadline.empty())
	{
		// create text block for headline text
		AddTextBlock(m_strHeadline, true, posLeftBorder, posRightBorder, boundsSpace);

		// add some extra space after the headline
		posLeftBorder.z -= (boundsSpace.z * 0.5);
	}
	// create text block for message text
	AddTextBlock(m_strMessage, false, posLeftBorder, posRightBorder, boundsSpace);

	return true;
}

void CSDLPreviewDlg::AddTextBlock(std::wstring strTextBlock, bool bCenter, glm::dvec3& posLeftBorder, const glm::dvec3& posRightBorder, const glm::dvec3& boundsSpace, double lfVOffsetFactor /*= 1.5*/)
{
	strTextBlock += L"\n";

	// horizontal start offset
	double lfHOffset = posLeftBorder.x;

	// calculate space between the lines
	double lfNextLineOffset = fabs(boundsSpace.z) * lfVOffsetFactor;

	// set vertical start offset off scroll text
	double& lfVOffset = posLeftBorder.z;

	// prepare message tokenizing
	CTokenizer			tokenizer(strTextBlock, L" \r\n\t", 0);
	int					nWordCount = 0;
	CTokenizer::CToken	token;

	// tokenize the message
	while (tokenizer.GetNext(token))
	{
		if (token.m_tt == CTokenizer::token_type::sep)
		{
			if (token.ToChar() == L'\n')
			{
				if (bCenter)
				{
					auto ppWord = m_listWords.rbegin();
					double hoffset = (posRightBorder.x - lfHOffset) / 2;

					for (int i = nWordCount; i > 0; --i)
					{
						(*ppWord)->MoveX(hoffset);
						ppWord++;
					}
				}
				lfVOffset += lfNextLineOffset;
				lfHOffset = posLeftBorder.x;
				nWordCount = 0;
			}
		}
		else
		{
			std::wstring strToken = token.ToString();

			std::shared_ptr<C3DWord> pWord = std::make_shared<C3DWord>();

			if (pWord->Create(strToken.c_str(), &m_LogFont))
			{
				pWord->SetColor(1, 1, 0);

				// lay down, scale and pos word onto the current line
				pWord->RotateX(90.0);
				pWord->Scale(m_lfFontScale);

				glm::dvec3 bounds = pWord->GetBoundingBox();

				if (nWordCount && lfHOffset + bounds.x > posRightBorder.x)
				{
					if (bCenter || nWordCount > 3)
					{
						auto ppWord = m_listWords.rbegin();

						if (bCenter)
						{
							double hoffset = (posRightBorder.x - lfHOffset) / 2;

							for (int i = nWordCount; i > 0; --i)
							{
								(*ppWord)->MoveX(hoffset);
								ppWord++;
							}
						}
						else
						{
							double hoffset = (posRightBorder.x - (lfHOffset - boundsSpace.x)) / ((double)nWordCount - 1);

							for (int i = nWordCount - 1; i > 0; --i)
							{
								(*ppWord)->MoveX(hoffset*i);
								ppWord++;
							}
						}
					}
					lfVOffset += lfNextLineOffset;
					lfHOffset = posLeftBorder.x;
					nWordCount = 0;
				}
				pWord->MoveX(lfHOffset);
				pWord->MoveZ(lfVOffset);

				lfHOffset += bounds.x;
				lfHOffset += boundsSpace.x;

				m_listWords.push_back(pWord);
				nWordCount++;
			}
		}
	}
}