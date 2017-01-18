#pragma once

#include <atlfile.h>
#include <atlsync.h>

#define NUM_CHANNELS	2
#define	SAMPLE_SIZE		16
#define	SAMPLE_FACTOR	(NUM_CHANNELS * (SAMPLE_SIZE >> 3))
#define	SAMPLE_FREQ		(44100.0)
#define BYTES_PER_SEC	(SAMPLE_FREQ * SAMPLE_FACTOR)

// aligns to the nearest sample (previous or next)
#define	ALIGN_TO_SAMPLE(__ToAlign)		{ auto __mod = __ToAlign  % SAMPLE_FACTOR; __ToAlign += (__mod > SAMPLE_FACTOR / 2) ? (SAMPLE_FACTOR - __mod) : (__mod * (-1)); }

// aligns to the previous sample
#define	ALIGN_DOWN_TO_SAMPLE(__ToAlign)	{ __ToAlign -= (__ToAlign  % SAMPLE_FACTOR); }

namespace local_utils
{ 

////////////////////////////////////////////////////////////////////
// global functions

HANDLE			DuplicateHandle(HANDLE h);
ULONGLONG		Shovel(HANDLE hFrom, HANDLE hTo, ULONGLONG nBytesToShovel, bool bInputHandleIsPipe = false);
HANDLE			Exec(LPCTSTR strCmd, DWORD dwWait = INFINITE, bool bStdInput = false, HANDLE* phWrite = NULL, WORD wShowWindow = SW_HIDE, PCWSTR strPath = NULL, HANDLE* phStdOut = NULL, HANDLE* phStdErr = NULL, DWORD nPipeBufferSize = 0);
std::wstring	GetModulePath(HMODULE hModule = NULL, PCWSTR strModuleName = NULL);
std::wstring	LocateModulePath(PCWSTR strModule, PCWSTR strDefaultExtension = L".exe");
bool			WritePPMData(HANDLE hTo, const BYTE* pFrameBuffer, int nWidth, int nHeight);
void			InitBitmapInfo(__out_bcount(cbInfo) BITMAPINFO *pbmi, ULONG cbInfo, LONG cx, LONG cy, WORD bpp, bool bFlip = true);
HRESULT			CreateDibmap(HDC hdc, const SIZE *psize, int nBits, __deref_opt_out void **ppvBits, __out HBITMAP* phBmp, bool bFlip = true);

///////////////////////////////////////////////////////////////////////
// CAutoSync

template<class T>
class CAutoSyncT
{
public:
	CAutoSyncT(T* pObjectToLock)
	{
		m_pObjectToLock = NULL;

		if (pObjectToLock)
			Lock(pObjectToLock);
	}
	CAutoSyncT(T& ObjectToLock)
	{
		m_pObjectToLock = NULL;
		Lock(&ObjectToLock);
	}
	~CAutoSyncT()
	{
		Unlock();
	}
public:
	inline void Lock(T* pObjectToLock)
	{
		ATLASSERT(!m_pObjectToLock);
		m_pObjectToLock = pObjectToLock;
		ATLASSERT(m_pObjectToLock);
		m_pObjectToLock->Enter();
	}
	inline void Lock(T& ObjectToLock)
	{
		Lock(&ObjectToLock)
	}
	inline void Unlock()
	{
		if (m_pObjectToLock)
		{
			m_pObjectToLock->Leave();
			m_pObjectToLock = NULL;
		}
	}
public:
	T*	m_pObjectToLock;
};

typedef CAutoSyncT<CCriticalSection> CAutoSync;

///////////////////////////////////////////////////////////////////////
// CMyQueue

template<class T>
class CMyQueue : protected std::list<T>
{
public:
	CMyQueue()
	{
		m_bCanQueue = true;
		ATLVERIFY(m_hEvent.Create(NULL, FALSE, FALSE, NULL));
	}
public:
	inline bool queue(const T& item)
	{
		CAutoSync sync(m_mtx);

		if (m_bCanQueue)
		{
			push_back(item);
			sync.Unlock();

			m_hEvent.Set();
			return true;
		}
		return false;
	}
	inline bool unqueue(T& item)
	{
		CAutoSync sync(m_mtx);
		
		if (!__super::empty())
		{
			item = front();
			pop_front();
			return true;
		}
		return false;
	}
	inline size_t size(void) const
	{
		CAutoSync sync(m_mtx);
		return __super::size();
	}
	inline size_t count(void) const
	{
		return size();
	}
	inline bool is_flushed() const
	{
		CAutoSync sync(m_mtx);
		return (!m_bCanQueue && _super::empty()) ? true : false;
	}
	inline void flush()
	{
		CAutoSync sync(m_mtx);
		m_bCanQueue = false;
		sync.Unlock();

		m_hEvent.Set();
	}
	inline bool empty() const
	{
		CAutoSync sync(m_mtx);
		return __super::empty() ? true : false;
	}
	inline void clear()
	{
		CAutoSync sync(m_mtx);
		m_hEvent.Reset();
		__super::clear();
		m_bCanQueue = true;
	}
public:
	CEvent					m_hEvent;
protected:
	CCriticalSection		m_mtx;
	bool					m_bCanQueue;
};


/////////////////////////////////////////////////////////////////////////
// CMyThread

class CMyThread
{
public:
	CMyThread()
	{
		m_dwTimeout = INFINITE;
		ATLVERIFY(m_hStopEvent.Create(NULL, TRUE, FALSE, NULL));
	}
	virtual ~CMyThread()
	{
		if (IsRunning())
			Stop();
	}
public:
	bool Start(HANDLE hEvent)
	{
		if (hEvent)
		{
			Stop();

			m_hEvent.Attach(DuplicateHandle(hEvent));

			if (m_hEvent)
			{
				m_hThread.Attach((HANDLE)_beginthreadex(NULL, 0, _Run, this, CREATE_SUSPENDED, NULL));

				if (m_hThread)
				{
					::ResumeThread(m_hThread);
					return true;
				}
			}
			Stop();
		}
		return false;
	}
	void Stop()
	{
		if (m_hThread)
		{
			m_hStopEvent.Set();
			::WaitForSingleObject(m_hThread, INFINITE);
			m_hThread.Close();
		}
		m_hEvent.Close();
		m_hStopEvent.Reset();
	}
	inline bool IsRunning() const
	{
		return m_hThread && ::WaitForSingleObject(m_hThread, 0) != WAIT_OBJECT_0;
	}
	inline bool IsStopping() const
	{
		return ::WaitForSingleObject(m_hStopEvent, 0) == WAIT_OBJECT_0;
	}
protected:
	virtual bool OnStart()
	{
		return true;
	}
	virtual void OnStop()
	{
	}
	virtual void OnTimeout()
	{
	}
	virtual void OnEvent()
	{
	}
private:
	static unsigned int __stdcall _Run(LPVOID pParam)
	{
		CMyThread* pThis = (CMyThread*)pParam;
		return pThis->Run();
	}
	unsigned int Run()
	{
		if (OnStart())
		{
			HANDLE h[2];

			h[0] = m_hStopEvent;
			h[1] = m_hEvent;

			bool bRun = true;

			do
			{
				switch (::WaitForMultipleObjects(m_hEvent != NULL ? 2 : 1, h, FALSE, m_dwTimeout))
				{
					case WAIT_OBJECT_0:
					{
						OnStop();
						bRun = false;
					}
					break;

					case WAIT_TIMEOUT:
					{
						OnTimeout();
					}
					break;

					default:
					{
						OnEvent();
					}
					break;
				}
			} 
			while (bRun);
		}
		_endthreadex(0);
		return 0;
	}
public:
	CHandle					m_hThread;
	CEvent					m_hStopEvent;
	CHandle					m_hEvent;
	DWORD					m_dwTimeout;
};


/////////////////////////////////////////////////////////////////////////////
// CFilePicker

class CFilePicker
{
public:
	CFilePicker(DWORD dwFlags = MAXDWORD)
	{
		m_dwFlags = dwFlags;
	}
	/* "Alle Dateien", "*.*", .... */
	BOOL	Load(LPCTSTR Title, int AnzFilter, ...);
	BOOL	Save(LPCTSTR Title, int AnzFilter, ...);
	LPCTSTR	GetPathName()
	{
		return m_strPathName;
	}
	void	SetPathName(LPCTSTR lpszPathName)
	{
		if (lpszPathName != NULL)
			m_strPathName = lpszPathName;
	}
	void	SetInitialDir(LPCTSTR);
	void	SetDefExt(LPCWSTR lpstrExt);
	void	SetDefExt(LPCSTR lpstrExt);
	long	GetSelectedType();

protected:
	WTL::CString	m_strPathName;
	WTL::CString	m_strInitialDir;
	WTL::CString	m_strDefExt;
	long			m_nSelectedType;
private:
	TCHAR			m_Buf[MAX_PATH];
	DWORD			m_dwFlags;
};


/////////////////////////////////////////////////////////
// CTemporaryFile

class CTemporaryFile
{
public:
	CTemporaryFile(PCWSTR strExt = NULL, bool bDeleteOnClose = true, bool bOverlappedIO = false)
	{
		if (strExt != NULL)
			m_strExt = strExt;

		m_bDeleteOnClose = bDeleteOnClose;
		memset((void*)&m_Operlapped, 0, sizeof(OVERLAPPED));

		if (bOverlappedIO)
		{
			m_Operlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
			ATLASSERT(m_Operlapped.hEvent);
		}
	}
	~CTemporaryFile()
	{
		Close();

		if (m_Operlapped.hEvent)
		{
			CloseHandle(m_Operlapped.hEvent);
			memset((void*)&m_Operlapped, 0, sizeof(OVERLAPPED));
		}
	}
	void Close()
	{
		if (m_file.m_h && m_file.m_h != INVALID_HANDLE_VALUE)
		{
			m_file.Close();
		}
	}
	bool IsOpen() const
	{
		if (m_file.m_h && m_file.m_h != INVALID_HANDLE_VALUE)
			return true;
		return false;
	}
	HRESULT Create(LPCTSTR pszDir = NULL, DWORD dwDesiredAccess = GENERIC_READ|GENERIC_WRITE, DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)
	{
		if (m_strTempFileName.empty())
		{
			TCHAR szPath[_MAX_PATH];
			TCHAR tmpFileName[_MAX_PATH];

			ATLASSERT(m_file.m_h == NULL);

			if (pszDir == NULL)
			{
				DWORD dwRet = GetTempPath(_MAX_DIR, szPath);

				if (dwRet == 0)
				{
					return AtlHresultFromLastError();
				}
				else if (dwRet > _MAX_DIR)
				{
					return DISP_E_BUFFERTOOSMALL;
				}
			}
			else
			{
				if (Checked::tcsncpy_s(szPath, _countof(szPath), pszDir, _TRUNCATE) == STRUNCATE)
				{
					return DISP_E_BUFFERTOOSMALL;
				}
			}
			if (!GetTempFileName(szPath, _T("UTL"), 0, tmpFileName))
			{
				return AtlHresultFromLastError();
			}
			tmpFileName[_countof(tmpFileName) - 1] = '\0';

			m_strTempFileName = tmpFileName;

			if (!m_strExt.empty())
			{
				::DeleteFile(m_strTempFileName.c_str());
				m_strTempFileName += m_strExt;
			}
		}
		return Open(false, dwDesiredAccess, dwShareMode);
	}
	inline HRESULT Open(bool bOpenExisting = true, DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE, DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)
	{
		SECURITY_ATTRIBUTES secatt;

		secatt.nLength = sizeof(secatt);
		secatt.lpSecurityDescriptor = NULL;
		secatt.bInheritHandle = TRUE;

		return m_file.Create(m_strTempFileName.c_str(), dwDesiredAccess, dwShareMode, bOpenExisting ? OPEN_EXISTING : CREATE_ALWAYS, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_TEMPORARY | (m_bDeleteOnClose ? FILE_FLAG_DELETE_ON_CLOSE : 0), &secatt);
	}
	operator HANDLE()
	{
		return m_file;
	}
	inline HANDLE Detach()
	{
		return m_file.Detach();
	}
	inline bool SeekToBegin()
	{
		return SUCCEEDED(m_file.Seek(0, FILE_BEGIN)) ? true : false;
	}
	inline bool Delete()
	{
		if (IsOpen())
			Close();

		::DeleteFile(m_strTempFileName.c_str());
		return !Exists();
	}
	inline bool Exists()
	{
		return PathFileExists(m_strTempFileName.c_str()) ? true : false;
	}
	inline void Clear()
	{
		if (IsOpen())
			Close();
		m_strTempFileName.clear();
	}
public:
	CAtlFile			m_file;
	std::wstring		m_strTempFileName;
	std::wstring		m_strExt;
	bool				m_bDeleteOnClose;
	OVERLAPPED			m_Operlapped;
};


////////////////////////////////////////////////////////////////////////////
// CWaveHeader

#include "pshpack1.h"

class CWaveHeader
{
public:
	struct
	{
		uint8_t		chunkID[4];
		uint32_t	chunkSize;
		uint8_t		format[4];
		uint8_t		subchunk1ID[4];
		uint32_t	subchunk1Size;
		uint16_t	audioFormat;
		uint16_t	numChannels;
		uint32_t	sampleRate;
		uint32_t	byteRate;
		uint16_t	blockAlign;
		uint16_t	bitsPerSample;
		uint8_t		subchunk2ID[4];
		uint32_t	subchunk2Size;
	} myData;
public:
	CWaveHeader()
	{
		init();
	}
	void init(uint32_t sampleRate = (uint32_t)SAMPLE_FREQ, uint8_t bitDepth = SAMPLE_SIZE, uint8_t numChans = NUM_CHANNELS)
	{
		// set all the ASCII literals
		memcpy(myData.chunkID		, "RIFF", 4);
		memcpy(myData.format		, "WAVE", 4);
		memcpy(myData.subchunk1ID	, "fmt ", 4);
		memcpy(myData.subchunk2ID	, "data", 4);

		// set all the known numeric values
		myData.audioFormat		= 1;
		myData.chunkSize		= 36;
		myData.subchunk1Size	= 16;
		myData.subchunk2Size	= 0;

		// set the passed-in numeric values
		myData.sampleRate		= sampleRate;
		myData.bitsPerSample	= bitDepth;
		myData.numChannels		= numChans;

		// calculate the remaining dependent values
		myData.blockAlign		= myData.numChannels * myData.bitsPerSample / 8;
		myData.byteRate			= myData.sampleRate * myData.blockAlign;
	}
	inline bool isValid() const
	{
		return memcmp(myData.chunkID, "RIFF", 4) == 0 ? true : false;
	}
	inline void update(uint32_t numBytes)
	{
		myData.subchunk2Size	= numBytes;
		myData.chunkSize		= myData.subchunk2Size + 36;
	}
	inline uint8_t	mySize()
	{
		return sizeof(myData);
	}
	inline uint32_t getPCMBytes() const
	{
		return myData.subchunk2Size;
	}
	inline double getPCMSeconds() const
	{
		return ((double)myData.subchunk2Size) / ((double)(myData.sampleRate * (myData.numChannels * (myData.bitsPerSample >> 3))));
	}
};

#include "poppack.h"

////////////////////////////////////////////////////////////////////////////
// CAVCoder

class CAVCoder
{
public:
	CAVCoder(PCWSTR strCoderName = L"ffmpeg");

	const std::wstring&	GetCoderPath()
	{
		return c_strPathToCoder;
	}
	HANDLE	StartCoder(PHANDLE phStdIn = NULL, PHANDLE phStdOut = NULL);
	
	HANDLE	DecodeAudioToPipe(std::wstring strAudioSourceFile, PHANDLE phPipe, bool bRawPCM = true);
	HANDLE	DecodeAudioToFile(std::wstring strAudioSourceFile, std::wstring strAudioDestFile, bool bRawPCM = false);
	
	HANDLE	EncodeSlideShow(std::wstring strMP4DestFile, PHANDLE phPipe, int nFramesPerSecond = 25, std::wstring strAudioSourceFile = std::wstring(), PCWSTR strOutputCodec = L"libx264", PCWSTR strInputCodec = L"ppm");

protected:
	static	std::wstring	c_strPathToCoder;
			std::wstring	m_strParamaters;
};

////////////////////////////////////////////////////////////////////////////
// CTokenizer

template<class T>
class CTokenizerT
{
public:
	typedef	T									value_type;
	typedef	typename value_type::value_type		char_type;
	typedef typename value_type::const_iterator	const_iterator;
	typedef enum
	{
		invalid,
		token,
		sep,
		quoted
	} token_type;

	typedef struct
	{
		inline value_type ToString() const
		{
			ATLASSERT(m_tt != token_type::invalid);
			return value_type(&(*m_iToken), m_nLen);
		}
		inline char_type ToChar() const
		{
			ATLASSERT(m_tt != token_type::invalid);
			return *m_iToken;
		}
		const_iterator	m_iToken;
		size_t			m_nLen;
		token_type		m_tt;
	}  CToken;
public:
	CTokenizerT(const value_type& strToParse, const value_type& strSeps, char_type QuotationMark = '\"')
	{
		m_strToParse			= strToParse;
		m_strSeps				= strSeps;
		m_QuotationMark			= m_QuotationMark;
		m_pCurrent				= m_strToParse.begin();
	}
	inline void Restart(const value_type& strToParse, const value_type& strSeps, char_type QuotationMark)
	{
		m_strToParse			= strToParse;
		m_strSeps				= strSeps;
		m_QuotationMark			= m_QuotationMark;
		m_pCurrent				= m_strToParse.begin();
	}
	inline void Restart(const value_type& strToParse, const value_type& strSeps)
	{
		m_strToParse			= strToParse;
		m_strSeps				= strSeps;
		m_pCurrent				= m_strToParse.begin();
	}
	inline void Restart(const value_type& strToParse)
	{
		m_strToParse			= strToParse;
		m_pCurrent				= m_strToParse.begin();
	}
	inline bool GetNext(CToken& result)
	{
		result.m_tt = token_type::invalid;

		bool bFinished = false;

		while (!bFinished && m_pCurrent != m_strToParse.end())
		{
			switch (result.m_tt)
			{
				case token_type::invalid:
				{
					if (m_QuotationMark && *m_pCurrent == m_QuotationMark)
					{
						result.m_tt = token_type::quoted;
						++m_pCurrent;
						result.m_iToken = m_pCurrent;
					}
					else if (m_strSeps.find(*m_pCurrent) != value_type::npos)
					{
						result.m_tt = token_type::sep;
						result.m_iToken = m_pCurrent++;
					}
					else
					{
						result.m_tt = token_type::token;
						result.m_iToken = m_pCurrent++;
					}
				}
				break;

				case token_type::quoted:
				{
					if (*m_pCurrent == m_QuotationMark)
						bFinished = true;
					else
						++m_pCurrent;
				}
				break;

				case token_type::sep:
				{
					if (m_strSeps.find(*m_pCurrent) == value_type::npos || (m_QuotationMark && *m_pCurrent == m_QuotationMark))
						bFinished = true;
					else
						++m_pCurrent;
				}
				break;

				case token_type::token:
				{
					if (m_strSeps.find(*m_pCurrent) != value_type::npos || (m_QuotationMark && *m_pCurrent == m_QuotationMark))
						bFinished = true;
					else
						++m_pCurrent;
				}
				break;

				default:
				{
					ATLASSERT(FALSE);
				}
				break;
			}
		}
		if (result.m_tt != token_type::invalid)
		{
			result.m_nLen = m_pCurrent - result.m_iToken;
			
			if (result.m_tt == token_type::quoted)
				--result.m_nLen;

			return true;
		}
		return false;
	}
public:
	value_type		m_strToParse;
	value_type		m_strSeps;
	char_type		m_QuotationMark;
protected:
	const_iterator	m_pCurrent;
};

typedef CTokenizerT<std::wstring>		CTokenizer;
typedef CTokenizerT<std::string>		CTokenizerA;

////////////////////////////////////////////////////////////////////////////
// interlocked_t

class interlocked_t
{
public:
	interlocked_t(LONG nInitValue = 0)
	{ 
		_value = nInitValue;
	}
	inline LONG increment()
	{
		// result is the incremented value
		return InterlockedIncrement(&_value);
	}
	inline LONG decrement()
	{ 
		// result is the decremented value
		return InterlockedDecrement(&_value);
	}
	// pre-increment 
	inline LONG operator ++()
	{
		return increment(); 
	}
	// pre-decrement 
	inline LONG operator --()
	{ 
		return decrement();
	}
	inline LONG clear()
	{
		// result is the cleared value
		return InterlockedExchange(&_value, 0); 
	}
	inline LONG reset() 
	{ 
		return clear();
	}
	template<class T>
	inline T set(T nNewValue)
	{ 
		// result is the original value
		return static_cast<T>(InterlockedExchange(&_value, static_cast<LONG>(nNewValue)));
	}
	inline bool set(bool bNewValue)
	{
		return set(bNewValue ? 1l : 0l) ? true : false;
	}
	inline LONG get() const
	{ 
		return InterlockedCompareExchange((volatile LONG *)&_value, 0, 0);
	}
	inline operator bool () const
	{
		return get() ? true : false;
	}
	template<class T>
	inline operator T () const
	{
		return static_cast<T>(get());
	}
	template<class T>
	inline T operator=(T i)
	{
		return set(i);
	}
	template<class T>
	inline bool operator==(T i) const
	{
		return (static_cast<T>(get())) == i ? true : false;
	}
	template<class T>
	inline bool operator!=(T i) const
	{
		return (static_cast<T>(get())) != i ? true : false;
	}
	template<class T>
	inline bool operator<=(T i) const
	{
		return (static_cast<T>(get())) <= i ? true : false;
	}
	template<class T>
	inline bool operator>=(T i) const
	{
		return (static_cast<T>(get())) >= i ? true : false;
	}
	template<class T>
	inline bool operator<(T i) const
	{
		return (static_cast<T>(get())) < i ? true : false;
	}
	template<class T>
	inline bool operator>(T i) const
	{
		return (static_cast<T>(get())) > i ? true : false;
	}
	inline bool operator!() const
	{
		return get() ? false : true;
	}
	template<class T>
	inline T add(T i)
	{
		// result is value + i
		return static_cast<T>(InterlockedAdd(&_value, static_cast<LONG>(i)));
	}
	template<class T>
	inline T sub(T i)
	{
		// result is value - i
		return static_cast<T>(InterlockedAdd(&_value, static_cast<LONG>(i) * (-1)));
	}
	template<class T>
	inline T operator+=(T i)
	{
		return add(i);
	}
	template<class T>
	inline T operator-=(T i)
	{
		return sub(i);
	}
protected:
	volatile LONG _value;
};

}
