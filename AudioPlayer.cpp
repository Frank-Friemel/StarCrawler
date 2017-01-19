#include "stdafx.h"
#include "AudioPlayer.h"


#pragma comment(lib, "Winmm.lib")


/////////////////////////////////////////////////////////////////////////////////
// CWaveQueueItem

bool CWaveQueueItem::Prepare(HWAVEOUT hWaveOut)
{
	bool bResult = true;

	if ((m_WaveHeader.dwFlags & WHDR_PREPARED) == 0)
	{
		m_WaveHeader.dwBufferLength	= m_nLen;
		m_WaveHeader.lpData			= (LPSTR)(BYTE*) m_Blob;

		if (MMSYSERR_NOERROR != waveOutPrepareHeader(hWaveOut, &m_WaveHeader, sizeof(m_WaveHeader)))
		{
			bResult = false;
			memset(&m_WaveHeader, 0, sizeof(m_WaveHeader));
		}
	}
	else
	{
		m_WaveHeader.dwBufferLength	= m_nLen;
	}
	m_bPlayed	= false;

	return bResult;
}


void CWaveQueueItem::UnPrepare(HWAVEOUT hWaveOut)
{
	if (m_WaveHeader.dwFlags & WHDR_PREPARED)
	{
		while (waveOutUnprepareHeader(hWaveOut, &m_WaveHeader, sizeof(m_WaveHeader)) != MMSYSERR_NOERROR)
			Sleep(10);
		memset(&m_WaveHeader, 0, sizeof(m_WaveHeader));
	}
}

bool CWaveQueueItem::Play(HWAVEOUT hWaveOut)
{
	m_WaveHeader.dwFlags &= ~WHDR_DONE;

	if (waveOutWrite(hWaveOut, &m_WaveHeader, sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
	{
		m_bPlayed = true;
		return true;
	}
	return false;
}

void  CWaveQueueItem::Mute()
{
	memset(m_Blob, 0, m_nLen);
}

/////////////////////////////////////////////////////////////////////////////////
// CWavePlayThread

CWavePlayThread::CWavePlayThread(UINT nDeviceID /*= WAVE_MAPPER*/)
{
	m_hWaveOut		= NULL;
	m_nDeviceID		= nDeviceID;
	m_nRealDeviceID	= 0;
	m_nDropSize		= -1;
	Init();
	m_bPaused		= false;
	m_bMuted		= false;
}

CWavePlayThread::~CWavePlayThread()
{
	Stop();
}

void CWavePlayThread::Init(ULONG nFreq /*= 44100*/, ULONG nChannels /*= 2*/, ULONG nSampleSizeBits /*= 16*/)
{
	memset(&m_format, 0, sizeof(m_format));

	m_format.cbSize				= 0;
	m_format.wFormatTag			= WAVE_FORMAT_PCM;
	m_format.nSamplesPerSec		= nFreq;
	m_format.nChannels			= (WORD) nChannels;
	m_format.wBitsPerSample		= (WORD) nSampleSizeBits;
	m_format.nBlockAlign		= (WORD) (nChannels * (nSampleSizeBits >> 3));
	m_format.nAvgBytesPerSec	= nFreq * m_format.nBlockAlign;

	m_Pool.clear();
}

bool CWavePlayThread::Alloc(std::shared_ptr<CWaveQueueItem>& pItem, ULONG nSize)
{
	if (pItem)
		pItem.reset();

	m_Pool.unqueue(pItem);

	if (!pItem)
	{
		pItem = std::make_shared<queue_item_type>();
	}
	if (pItem)
	{
		if (pItem->m_Blob.Reallocate(nSize))
		{
			pItem->m_nLen = nSize;
		}
		else
		{
			pItem.reset();
		}
	}
	return pItem ? true : false;
}

void CWavePlayThread::Free(std::shared_ptr<CWaveQueueItem>& pItem)
{
	ATLASSERT(pItem);
	m_Pool.queue(pItem);
	pItem.reset();
}

bool CWavePlayThread::Start()
{
	if (IsRunning())
		return true;
	bool bResult = false;

	ATLASSERT(!m_hStarted);
	m_hStarted.Attach(::CreateEvent(NULL, TRUE, FALSE, NULL));
	
	HANDLE hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	if (__super::Start(hEvent))
	{
		::WaitForSingleObject(m_hStarted, INFINITE);

		bResult = m_hWaveOut != NULL ? true : false;

		if (!bResult)
			Stop();
	}
	m_hStarted.Close();
	CloseHandle(hEvent);

	return bResult;
}

void CWavePlayThread::Stop()
{
	if (IsPaused())
		Pause();
	__super::Stop();
}

bool CWavePlayThread::OnStart()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);

	bool bResult = false;

	ATLASSERT(m_hWaveOut == NULL);
	m_bPaused	= false;
	m_bMuted	= false;

	if (MMSYSERR_NOERROR != waveOutOpen(&m_hWaveOut, m_nDeviceID, &m_format, (DWORD_PTR)(HANDLE)m_hEvent, NULL, CALLBACK_EVENT))
	{
		m_hWaveOut = NULL;
	}
	else
	{
		bResult = true;

		if (MMSYSERR_NOERROR != waveOutGetID(m_hWaveOut, &m_nRealDeviceID)) 
		{
			m_nRealDeviceID = m_nDeviceID;
		}
	}
	::SetEvent(m_hStarted);
	return bResult;
}


void CWavePlayThread::OnStop()
{
	if (m_hWaveOut)
	{
		std::shared_ptr<CWaveQueueItem> pItem;

		m_Queue.unqueue(pItem);

		while(pItem)
		{
			pItem->UnPrepare(m_hWaveOut);
			pItem.reset();

			m_Queue.unqueue(pItem);
		}

		m_Pool.unqueue(pItem);

		while(pItem)
		{
			pItem->UnPrepare(m_hWaveOut);
			pItem.reset();

			m_Pool.unqueue(pItem);
		}

		ATLVERIFY(waveOutClose(m_hWaveOut) == MMSYSERR_NOERROR);
		m_hWaveOut = NULL;
	}
}

bool CWavePlayThread::Play(std::shared_ptr<CWaveQueueItem>& pItem)
{
	ATLASSERT(pItem);

	if (m_hWaveOut != NULL)
	{
		if (!IsStopping())
		{
			if (pItem->Prepare(m_hWaveOut))
			{
				m_Queue.Lock(true);
				::ResetEvent(m_hEvent);
				m_Queue.push_back(pItem);

				if (!m_bPaused && m_nDropSize >= 0 && m_hDropEvent && m_nDropSize >= (LONG) m_Queue.size())
				{
					::SetEvent(m_hDropEvent);
					m_Queue.Unlock();
					return true;
				}
				if (m_bPaused)
				{
					m_Queue.Unlock();
					return true;
				}
				for (auto i = m_Queue.begin(); i != m_Queue.end(); ++i)
				{
					if (!(*i)->IsPlayed())
					{
						if (m_bMuted)
						{
							(*i)->Mute();
						}
						(*i)->Play(m_hWaveOut);
					}
				}		
				m_Queue.Unlock();
				return true;
			}
		}
	}
	return false;
}

void CWavePlayThread::OnEvent()
{
	DWORD dwSleep = 0;

	m_Queue.Lock(true);

	while (!m_Queue.empty())
	{
		std::shared_ptr<CWaveQueueItem>& pItem = m_Queue.front();

		if (pItem->m_WaveHeader.dwFlags & WHDR_DONE)
		{
			std::shared_ptr<CWaveQueueItem> _pItem = pItem;
			m_Queue.pop_front();
			m_Queue.Unlock();

			Free(_pItem);

			m_Queue.Lock(true);
		}
		else
			break;
	}
	if (!m_bPaused && m_nDropSize >= 0 && m_hDropEvent && m_nDropSize >= (LONG) m_Queue.size())
	{
		::SetEvent(m_hDropEvent);

		if (m_Queue.size() == 0)
			dwSleep = 5;
	}
	m_Queue.Unlock();		

	// allow thread switch doing this wait
	::WaitForSingleObject(m_hStopEvent, dwSleep);
}

void CWavePlayThread::Pause()
{
	if (m_hWaveOut != NULL)
	{
		if (m_bPaused)
		{
			m_Queue.Lock(true);
			m_bPaused = false;

			if (m_nDropSize >= 0 && m_hDropEvent && m_nDropSize >= (LONG) m_Queue.size())
			{
				::SetEvent(m_hDropEvent);
				Sleep(0);
			}
			else
			{
				for (auto i = m_Queue.begin(); i != m_Queue.end(); ++i)
				{
					if (!(*i)->IsPlayed())
					{
						(*i)->Play(m_hWaveOut);
					}
				}
			}
			m_Queue.Unlock();

			ATLVERIFY(waveOutRestart(m_hWaveOut) == MMSYSERR_NOERROR);
		}
		else
		{
			m_Queue.Lock(true);

			if (waveOutPause(m_hWaveOut) == MMSYSERR_NOERROR)
			{
				::ResetEvent(m_hEvent);
				m_bPaused = true;
			}
			m_Queue.Unlock();
		}
	}
}

void CWavePlayThread::Mute()
{
	m_bMuted = !m_bMuted;
}

void CWavePlayThread::SetVolume(WORD wVolume)
{
	ATLASSERT(m_hWaveOut != NULL);

	DWORD dwVolume = ((DWORD) wVolume) | (((DWORD) wVolume) << 16);
	ATLVERIFY(waveOutSetVolume(m_hWaveOut, dwVolume) == MMSYSERR_NOERROR);
}

WORD CWavePlayThread::GetVolume()
{
	ATLASSERT(m_hWaveOut != NULL);

	DWORD dwVolume = 0;
	ATLVERIFY(waveOutGetVolume(m_hWaveOut, &dwVolume) == MMSYSERR_NOERROR);
	return (WORD) (dwVolume & 0x0000ffff);
}


CAudioPlayer::CAudioPlayer()
{
}

CAudioPlayer::~CAudioPlayer()
{
	Close();
}

bool CAudioPlayer::Init(HANDLE hDataSource)
{
	if (!hDataSource)
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	Close();

	m_hAudioData.Attach(DuplicateHandle(hDataSource));

	if (!m_hAudioData)
		return false;

	SDL_AudioSpec wav_spec;

	memset(&wav_spec, 0, sizeof(wav_spec));

	// WAV parameters as usual
	wav_spec.callback	= my_sdl_audio_callback;
	wav_spec.userdata	= this;
	wav_spec.freq		= 44100;
	wav_spec.channels	= 2;
	wav_spec.samples	= 4096;
	wav_spec.format		= AUDIO_S16LSB;

	// thanks to SDL we don't have much todo
	if (SDL_OpenAudio(&wav_spec, NULL) < 0)
	{
		m_hAudioData.Close();
		return false;
	}
	return true;
}

void CAudioPlayer::Play()
{
	if (m_hAudioData)
		SDL_PauseAudio(0);
}

void CAudioPlayer::Pause()
{
	if (m_hAudioData)
		SDL_PauseAudio(1);
}

void CAudioPlayer::Close()
{
	if (m_hAudioData)
	{ 
		SDL_CloseAudio();
		m_hAudioData.Close();
	}
}

void CAudioPlayer::OnPlayAudio(BYTE* pStream, DWORD dwLen)
{
	if (dwLen)
	{
		memset(pStream, 0, dwLen);

		int		nTry		= 5;
		DWORD	dwRead		= 0;
		DWORD	dwReadTotal	= 0;

		// read PCM data from handle directly to the sound buffer in mem
		while (nTry > 0 && dwLen && ::ReadFile(m_hAudioData, pStream + dwReadTotal, dwLen, &dwRead, NULL))
		{
			if (dwRead == 0)
			{
				// the input handle may be a pipe ... give it some time to deliver the data (over all 25ms)
				nTry--;
				Sleep(5);
			}
			else
			{
				nTry		= 5;
				dwLen		-= dwRead;
				dwReadTotal += dwRead;
			}
		}
	}
}