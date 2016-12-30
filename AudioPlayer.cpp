#include "stdafx.h"
#include "AudioPlayer.h"


static void my_sdl_audio_callback(void* pUserdata, Uint8* stream, int len)
{
	ATLASSERT(pUserdata);

	CAudioPlayer* pThis = (CAudioPlayer*)pUserdata;

	pThis->OnPlayAudio((BYTE*)stream, len);
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

	wav_spec.callback	= my_sdl_audio_callback;
	wav_spec.userdata	= this;
	wav_spec.freq		= 44100;
	wav_spec.channels	= 2;
	wav_spec.samples	= 4096;
	wav_spec.format		= AUDIO_S16LSB;

	if (SDL_OpenAudio(&wav_spec, NULL) < 0)
	{
		m_hAudioData.Close();
		return false;
	}
	return true;
}

void CAudioPlayer::Play()
{
	SDL_PauseAudio(0);
}

void CAudioPlayer::Pause()
{
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