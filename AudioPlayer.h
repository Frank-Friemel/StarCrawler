/////////////////////////////////////////////////////////////////////////////////
// CAudioPlayer

#pragma once

#include "Mmsystem.h"
#include "utils.h"

class 	CWavePlayThread;

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

	void		Mute(bool bMute);
	bool		IsMuted() const;
		
	static void FadeSamples(void* pDest, const BYTE* pSrc, ULONG dwBytes, double lfVolume);

public:
	// the default implementation just copies the data over
	virtual void OnPlayAudio(BYTE* pStream, ULONG dwLen);

protected:
	CHandle								m_hAudioData;
	std::shared_ptr<CWavePlayThread>	m_threadWavePlay;
};

