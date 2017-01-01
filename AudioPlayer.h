/////////////////////////////////////////////////////////////////////////////////
// CAudioPlayer

#pragma once


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
	CHandle	m_hAudioData;

};

