/////////////////////////////////////////////////////////////////////////////////
// CAudioPlayer

#pragma once


class CAudioPlayer
{
public:
	CAudioPlayer();
	~CAudioPlayer();

	bool	Init(HANDLE hDataSource);
	void	Close();

	void	Play();
	void	Pause();

public:
	virtual void OnPlayAudio(BYTE* pStream, DWORD dwLen);

protected:
	CHandle	m_hAudioData;

};

