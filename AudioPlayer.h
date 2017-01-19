/////////////////////////////////////////////////////////////////////////////////
// CAudioPlayer

#pragma once

#include "Mmsystem.h"
#include "utils.h"

class CWavePlayThread;

////////////////////////////////////////////////////////////////
// CWaveQueueItem

class CWaveQueueItem
{
	friend class CWavePlayThread;
public:
	CWaveQueueItem()
	{
		m_nLen		= 0;
		memset(&m_WaveHeader, 0, sizeof(m_WaveHeader));
		m_bPlayed	= false;
	}
	inline bool IsPlayed() const
	{
		return m_bPlayed;
	}
protected:
	bool Prepare(HWAVEOUT hWaveOut);
	void UnPrepare(HWAVEOUT hWaveOut);
	bool Play(HWAVEOUT hWaveOut);
	void Mute();

public:
	CTempBuffer<BYTE>			m_Blob;
	ULONG						m_nLen;

private:
	WAVEHDR						m_WaveHeader;
	bool						m_bPlayed;
};

////////////////////////////////////////////////////////////////
// CWaveQueueItemList

typedef CMyQueue< std::shared_ptr<CWaveQueueItem> >	CWaveQueueItemList;

///////////////////////////////////////////////////////////
// CWavePlayThread

class CWavePlayThread : public CMyThread
{
public:
	typedef CWaveQueueItem	queue_item_type;

	CWavePlayThread(UINT nDeviceID = WAVE_MAPPER);
	~CWavePlayThread();

	void Init(ULONG nFreq = (ULONG)SAMPLE_FREQ, ULONG nChannels = NUM_CHANNELS, ULONG nSampleSizeBits = SAMPLE_SIZE);
	bool Start();
	void Stop();

	bool Alloc(std::shared_ptr<CWaveQueueItem>& pItem, ULONG nSize);
	void Free(std::shared_ptr<CWaveQueueItem>& pItem);

	bool Play(std::shared_ptr<CWaveQueueItem>& pItem);

	inline ULONG Queued() const
	{
		return (ULONG) m_Queue.size();
	}
	inline void SetDropEvent(HANDLE hEvent = NULL, LONG nDropSize = -1)
	{
		if (hEvent)
		{
			m_hDropEvent.Attach(DuplicateHandle(hEvent));
			ATLASSERT(m_hDropEvent);
			m_nDropSize = nDropSize;
		}
		else
		{
			m_hDropEvent.Close();
			m_nDropSize = -1;
		}
	}
	inline ULONG GetDropSize() const
	{
		return (ULONG) m_nDropSize.get();
	}
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

protected:
	virtual bool OnStart();
	virtual void OnStop();
	virtual void OnEvent();

protected:
	HWAVEOUT				m_hWaveOut;
	CWaveQueueItemList		m_Queue;
	CWaveQueueItemList		m_Pool;

	CHandle					m_hStarted;
	UINT					m_nDeviceID;
	CHandle					m_hDropEvent;
	interlocked_t			m_nDropSize;
	bool					m_bPaused;
	bool					m_bMuted;

public:
	WAVEFORMATEX			m_format;
	UINT					m_nRealDeviceID;
};

///////////////////////////////////////////////////////////
// CAudioPlayer

class CAudioPlayer 
{
public:
	CAudioPlayer();
	~CAudioPlayer();

	// handle of a pipe or a file
	bool	Init(HANDLE hDataSource);
	void	Close();

	void	Play();
	void	Pause();

public:
	// the default implementation just copies the data over
	virtual void OnPlayAudio(BYTE* pStream, DWORD dwLen);

protected:
	CHandle			m_hAudioData;
	CWavePlayThread	m_threadWavePlay;
};

