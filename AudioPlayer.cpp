#include "stdafx.h"
#include "AudioPlayer.h"


#pragma comment(lib, "Winmm.lib")

////////////////////////////////////////////////////////////////
// CWaveQueueItem

class CWaveQueueItem
{
public:
	CWaveQueueItem()
	{
		m_nLen		= 0;
		InitNew();
	}
	inline bool IsPlayed() const
	{
		return m_bPlayed;
	}
	inline bool IsDone() const
	{
		return m_WaveHeader.dwFlags & WHDR_DONE ? true : false;
	}
	inline void InitNew()
	{
		memset(&m_WaveHeader, 0, sizeof(m_WaveHeader));
		m_bPlayed	= false;
	}
	bool Prepare(HWAVEOUT hWaveOut);
	void UnPrepare(HWAVEOUT hWaveOut);
	bool Play(HWAVEOUT hWaveOut);
	void WaitPlayed();
	void Mute();

public:
	CTempBuffer<BYTE>			m_Blob;
	ULONG						m_nLen;

private:
	WAVEHDR						m_WaveHeader;
	bool						m_bPlayed;
};

///////////////////////////////////////////////////////////
// CWavePlayThread

class CWavePlayThread : public CMyThread
{
public:
	typedef std::function<void(BYTE* pStream, ULONG dwLen)>	callback_t;

	CWavePlayThread(UINT nDeviceID = WAVE_MAPPER);
	~CWavePlayThread();

	void Init(ULONG nFreq = (ULONG)SAMPLE_FREQ, ULONG nChannels = NUM_CHANNELS, ULONG nSampleSizeBits = SAMPLE_SIZE);
	bool Start(callback_t callback, size_t nThreshold = 16);
	void Stop();

	void		Pause();
	inline bool IsPaused() const
	{
		return m_bPaused;
	}
	void		Mute();
	inline bool IsMuted() const
	{
		return m_bMuted;
	}
	void SetVolume(WORD wVolume);
	WORD GetVolume();

private:
	bool Alloc(std::shared_ptr<CWaveQueueItem>& pItem);
	void Free(std::shared_ptr<CWaveQueueItem>& pItem);

protected:
	virtual bool OnStart();
	virtual void OnStop();
	virtual void OnEvent();

protected:
	HWAVEOUT				m_hWaveOut;

	typedef CMyQueue< std::shared_ptr<CWaveQueueItem>, CDummyLock >	CWaveQueueItemList;

	CWaveQueueItemList		m_Queue;
	CWaveQueueItemList		m_Pool;

	CHandle					m_hStarted;
	UINT					m_nDeviceID;
	size_t					m_nThreshold;
	volatile bool			m_bPaused;
	volatile bool			m_bMuted;
	callback_t				m_callback;

public:
	WAVEFORMATEX			m_format;
	UINT					m_nRealDeviceID;
};


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
		InitNew();
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

void CWaveQueueItem::WaitPlayed()
{
	if (m_bPlayed)
	{
		while (!(m_WaveHeader.dwFlags & WHDR_DONE))
		{
			Sleep(10);
		}
	}
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
	m_nThreshold	= 16;
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

bool CWavePlayThread::Alloc(std::shared_ptr<CWaveQueueItem>& pItem)
{
	if (pItem)
		pItem.reset();

	m_Pool.unqueue(pItem);

	if (!pItem)
	{
		pItem = std::make_shared<CWaveQueueItem>();

		if (!pItem)
			return false;

		pItem->m_nLen = 4096;

		if (!pItem->m_Blob.Allocate(pItem->m_nLen))
			return false;
	}
	memset(pItem->m_Blob, 0, pItem->m_nLen);
	return true;
}

void CWavePlayThread::Free(std::shared_ptr<CWaveQueueItem>& pItem)
{
	ATLASSERT(pItem);
	pItem->InitNew();
	m_Pool.queue(pItem);
	pItem.reset();
}

bool CWavePlayThread::Start(callback_t callback, size_t nThreshold /*= 16*/)
{
	if (IsRunning())
		return true;
	bool bResult = false;

	m_nThreshold	= nThreshold;
	m_callback		= callback;

	ATLASSERT(m_callback);

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
	__super::Stop();
}

bool CWavePlayThread::OnStart()
{
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
		if (waveOutPause(m_hWaveOut) == MMSYSERR_NOERROR)
		{
			::ResetEvent(m_hEvent);
			m_bPaused = true;
		}
		else
		{
			ATLASSERT(FALSE);
		}
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
		ATLVERIFY(waveOutReset(m_hWaveOut) == MMSYSERR_NOERROR);

		std::shared_ptr<CWaveQueueItem> pItem;

		while(m_Queue.unqueue(pItem))
		{
			pItem->WaitPlayed();
			pItem->UnPrepare(m_hWaveOut);
			pItem.reset();
		}
		m_Pool.clear();
		ATLVERIFY(waveOutClose(m_hWaveOut) == MMSYSERR_NOERROR);
		
		m_hWaveOut	= NULL;
		m_bPaused	= false;
	}
}

void CWavePlayThread::OnEvent()
{
	::ResetEvent(m_hEvent);

	std::shared_ptr<CWaveQueueItem> pItem;

	while (m_Queue.peek(pItem))
	{
		if (pItem->IsDone())
		{
			m_Queue.unqueue();
			Free(pItem);
		}
		else
		{
			pItem.reset();
			break;
		}
	}
	if (!m_bPaused)
	{
		while (m_nThreshold >= m_Queue.size())
		{
			if (!Alloc(pItem))
				break;
			m_callback(pItem->m_Blob, pItem->m_nLen);

			if (pItem->Prepare(m_hWaveOut))
			{
				m_Queue.queue(pItem);
			}
			else
			{
				ATLASSERT(FALSE);
			}
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
	}
}

void CWavePlayThread::Pause()
{
	if (m_hWaveOut != NULL)
	{
		if (m_bPaused)
		{
			m_bPaused = false;

			::SetEvent(m_hEvent);
			Sleep(0);

			ATLVERIFY(waveOutRestart(m_hWaveOut) == MMSYSERR_NOERROR);
		}
		else
		{
			if (waveOutPause(m_hWaveOut) == MMSYSERR_NOERROR)
			{
				::ResetEvent(m_hEvent);
				m_bPaused = true;
			}
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

//////////////////////////////////////////////////////////
// CAudioPlayer

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
	
	m_threadWavePlay = std::make_shared<CWavePlayThread>();

	if (!m_threadWavePlay)
	{
		Close();
		return false;
	}
	m_threadWavePlay->Init();
	return m_threadWavePlay->Start([this](BYTE* pStream, ULONG dwLen) -> void { OnPlayAudio(pStream, dwLen); });
}

void CAudioPlayer::Play()
{
	if (m_threadWavePlay && m_threadWavePlay->IsPaused())
		m_threadWavePlay->Pause();
}

void CAudioPlayer::Pause()
{
	if (m_threadWavePlay && !m_threadWavePlay->IsPaused())
		m_threadWavePlay->Pause();
}

void CAudioPlayer::Close()
{
	if (m_threadWavePlay)
	{
		m_threadWavePlay->Stop();
		m_hAudioData.Close();
		m_threadWavePlay.reset();
	}
}

void  CAudioPlayer::FadeSamples(void* pDest, const BYTE* pSrc, ULONG dwBytes, double lfVolume)
{
	ATLASSERT(pDest && pSrc && dwBytes % SAMPLE_FACTOR == 0 && lfVolume >= 0);

	if (dwBytes)
	{
		const short*	sample_in			= (const short*)pSrc;
		short*			sample_out			= (short*)pDest;

		dwBytes /= SAMPLE_FACTOR;

		do
		{
			for (auto i = 0; i < NUM_CHANNELS; ++i)
			{
				*sample_out++ = (short) (((double)*sample_in++) * lfVolume);
			}
		}				
		while (--dwBytes);
	}
}

void CAudioPlayer::OnPlayAudio(BYTE* pStream, ULONG dwLen)
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