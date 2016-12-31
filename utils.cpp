#include "stdafx.h"
#include "utils.h"

namespace local_utils
{

class CLocalFileDialog : public CFileDialogImpl<CLocalFileDialog>
{
public:
	CLocalFileDialog(BOOL bFlag) : CFileDialogImpl<CLocalFileDialog>(bFlag)
	{
		m_nSelectedType = 0;
	}
	CLocalFileDialog(BOOL bFlag, DWORD dwFlags) : CFileDialogImpl<CLocalFileDialog>(bFlag, NULL, NULL, dwFlags)
	{
		m_nSelectedType = 0;
	}
	BOOL OnFileOK(LPOFNOTIFY /*lpon*/)
	{
		TCHAR buf[1024];

		GetFilePath(buf, sizeof(buf) / sizeof(TCHAR));

		m_strPathName = buf;
		return TRUE;
	}
public:
	WTL::CString m_strPathName;
	WTL::CString m_strDefExt;

	long	m_nSelectedType;

	void OnTypeChange(LPOFNOTIFY lpon)
	{
		m_nSelectedType = lpon->lpOFN->nFilterIndex;
	}

	void OnInitDone(LPOFNOTIFY /*lpon*/)
	{
		if (m_strDefExt.GetLength() > 0)
		{
			SetDefExt(m_strDefExt);
		}
	}
};

///////////////////////////////////////////////////////////////////////////
// CFilePicker

BOOL CFilePicker::Load(LPCTSTR Title, int AnzFilter, ...)
{
	const size_t BUF_SIZE = 128;
	TCHAR* 	buf = new TCHAR[BUF_SIZE];
	TCHAR*	filter = NULL;
	TCHAR*  s1;
	TCHAR*  s2;

	WTL::CString strFilter;

	va_list	marker;
	va_start(marker, AnzFilter);

	buf[0] = _T('\0');

	for (int lauf = 0; lauf < AnzFilter; lauf++)
	{
		s1 = va_arg(marker, TCHAR*);
		s2 = va_arg(marker, TCHAR*);

		_stprintf_s(buf, BUF_SIZE, _T("%s|%s|"), s1, s2);
		strFilter += buf;
	}
	strFilter += _T('|');

	size_t lenFilter = strFilter.GetLength() + 1;
	filter = new TCHAR[lenFilter];
	_tcscpy_s(filter, lenFilter, strFilter);

	for (int lauf = 0; lauf < strFilter.GetLength(); lauf++)
		if (filter[lauf] == _T('|'))
			filter[lauf] = _T('\0');

	CLocalFileDialog* pDlg = NULL;

	if (m_dwFlags != MAXDWORD)
		pDlg = new CLocalFileDialog(TRUE, m_dwFlags);
	else
		pDlg = new CLocalFileDialog(TRUE);

	pDlg->m_ofn.lpstrTitle = Title;
	pDlg->m_ofn.lpstrFilter = filter;

	if (!m_strInitialDir.IsEmpty())
		pDlg->m_ofn.lpstrInitialDir = m_strInitialDir;

	BOOL result = pDlg->DoModal() == IDOK;

	if (result)
	{
		m_strPathName = pDlg->m_strPathName;
	}
	else
		m_strPathName.Empty();

	delete[] buf;
	delete[] filter;

	m_nSelectedType = pDlg->m_nSelectedType;

	delete pDlg;

	return result;
}

BOOL CFilePicker::Save(LPCTSTR Title, int AnzFilter, ...)
{
	const size_t BUF_SIZE = 128;
	TCHAR* 	buf = new TCHAR[BUF_SIZE];
	TCHAR*	filter = NULL;
	TCHAR*  s1;
	TCHAR*  s2;
	WTL::CString strFilter;

	va_list	marker;
	va_start(marker, AnzFilter);

	buf[0] = _T('\0');

	for (int lauf = 0; lauf < AnzFilter; lauf++)
	{
		s1 = va_arg(marker, TCHAR*);
		s2 = va_arg(marker, TCHAR*);

		_stprintf_s(buf, BUF_SIZE, _T("%s|%s|"), s1, s2);
		strFilter += buf;
	}
	strFilter += _T('|');

	size_t lenFilter = strFilter.GetLength() + 1;
	filter = new TCHAR[lenFilter];
	_tcscpy_s(filter, lenFilter, strFilter);

	for (int lauf = 0; lauf < strFilter.GetLength(); lauf++)
		if (filter[lauf] == _T('|'))
			filter[lauf] = _T('\0');

	CLocalFileDialog* pDlg = NULL;

	if (m_dwFlags != MAXDWORD)
		pDlg = new CLocalFileDialog(FALSE, m_dwFlags);
	else
		pDlg = new CLocalFileDialog(FALSE);

	pDlg->m_ofn.lpstrTitle = Title;
	pDlg->m_ofn.lpstrFilter = filter;

	if (!m_strInitialDir.IsEmpty())
		pDlg->m_ofn.lpstrInitialDir = m_strInitialDir;

	if (!m_strPathName.IsEmpty())
	{
		int n = m_strPathName.ReverseFind(_T('\\'));

		if (n > 0)
			_tcscpy_s(m_Buf, _countof(m_Buf), m_strPathName.Mid(n + 1));
		else
			_tcscpy_s(m_Buf, _countof(m_Buf), m_strPathName);

		pDlg->m_ofn.lpstrFile = m_Buf;
	}

	pDlg->m_strDefExt = m_strDefExt;

	BOOL result = pDlg->DoModal() == IDOK;

	if (result)
	{
		m_strPathName = pDlg->m_strPathName;
	}
	else
		m_strPathName.Empty();

	delete[] buf;
	delete[] filter;

	m_nSelectedType = pDlg->m_nSelectedType;

	delete pDlg;

	return result;
}

void CFilePicker::SetInitialDir(LPCTSTR strDir)
{
	m_strInitialDir = strDir;
}

void CFilePicker::SetDefExt(LPCWSTR lpstrExt)
{
	USES_CONVERSION;
	m_strDefExt = W2CT(lpstrExt);
}

void CFilePicker::SetDefExt(LPCSTR lpstrExt)
{
	USES_CONVERSION;
	m_strDefExt = A2CT(lpstrExt);
}

long CFilePicker::GetSelectedType()
{
	return m_nSelectedType;
}

////////////////////////////////////////////////////////////////////////////
// CAVCoder

std::wstring	CAVCoder::c_strPathToCoder;

CAVCoder::CAVCoder(PCWSTR strCoderName /*= L"ffmpeg"*/)
{
	ATLASSERT(strCoderName && *strCoderName);

	if (c_strPathToCoder.empty())
	{
		c_strPathToCoder = LocateModulePath(strCoderName);

		if (c_strPathToCoder.empty())
		{
			ATLTRACE(L"Failed to locate Coder %s\n", strCoderName);
		}
		else
		{
			ATLTRACE(L"Succeeded to locate Coder %s\n", c_strPathToCoder.c_str());
			c_strPathToCoder = std::wstring(L"\"") + c_strPathToCoder + std::wstring(L"\"");
		}
	}
}

HANDLE CAVCoder::StartCoder(PHANDLE phStdIn /* = NULL*/, PHANDLE phStdOut /* = NULL*/)
{
	std::wstring strCmd = c_strPathToCoder;

	if (!m_strParamaters.empty())
	{
		strCmd += L" ";
		strCmd += m_strParamaters;
	}
	return Exec(strCmd.c_str(), 0, phStdIn || phStdOut ? true : false, phStdIn, SW_HIDE, NULL, phStdOut);
}

HANDLE CAVCoder::DecodeAudioToPipe(std::wstring strAudioSourceFile, PHANDLE phPipe, bool bRawPCM /* = true */)
{
	m_strParamaters = std::wstring(L"-i \"")	+ strAudioSourceFile
												+ std::wstring(L"\" -acodec pcm_s16le ")
												+ std::wstring(bRawPCM ? L"-f s16le" : L"-f WAV")
												+ std::wstring(L" pipe:");

	return StartCoder(NULL, phPipe);
}

HANDLE CAVCoder::DecodeAudioToFile(std::wstring strAudioSourceFile, std::wstring strAudioDestFile, bool bRawPCM /*= false*/)
{
	m_strParamaters = std::wstring(L"-y -i \"") + strAudioSourceFile
												+ std::wstring(L"\" -acodec pcm_s16le ")
												+ std::wstring(bRawPCM ? L"-f s16le" : L"-f WAV")
												+ std::wstring(L" \"")
												+ strAudioDestFile
												+ std::wstring(L"\"");

	return StartCoder();
}

HANDLE CAVCoder::EncodeSlideShow(std::wstring strMP4DestFile, PHANDLE phPipe, int nFramesPerSecond /* = 25*/, std::wstring strAudioSourceFile /* = std::wstring()*/, PCWSTR strOutputCodec /*= L"libx264"*/, PCWSTR strInputCodec /*= L"ppm"*/)
{
	if (!phPipe)
		return NULL;

	WCHAR strFFMpegCmd[4096];

	// input video codec is ppm - http://netpbm.sourceforge.net/doc/ppm.html
	if (strAudioSourceFile.length())
	{
		swprintf_s(strFFMpegCmd, 4096, L"-y -v 0 -framerate %d -f image2pipe -vcodec %s -i - -i \"%s\" -vcodec %s -c:a aac -b:a 192k -shortest \"%s\""
			, nFramesPerSecond
			, strInputCodec
			, strAudioSourceFile.c_str()
			, strOutputCodec
			, strMP4DestFile.c_str());
	}
	else
	{
		swprintf_s(strFFMpegCmd, 4096, L"-y -v 0 -framerate %d -f image2pipe -vcodec %s -i - -vcodec %s \"%s\""
			, nFramesPerSecond
			, strInputCodec
			, strOutputCodec
			, strMP4DestFile.c_str());
	}
	m_strParamaters = strFFMpegCmd;
	return StartCoder(phPipe);
}


///////////////////////////////////////////////////////////////////////////
// global functions

HANDLE DuplicateHandle(HANDLE h)
{
	HANDLE hResult = NULL;

	if (!::DuplicateHandle(::GetCurrentProcess(), h, ::GetCurrentProcess(), &hResult, 0, FALSE, DUPLICATE_SAME_ACCESS))
		return NULL;
	return hResult;
}

HANDLE Exec(LPCTSTR strCmd, DWORD dwWait /*= INFINITE*/, bool bStdInput /*= false*/, HANDLE* phWrite /*= NULL*/, WORD wShowWindow /*= SW_HIDE*/, PCWSTR strPath /* = NULL*/, HANDLE* phStdOut /*= NULL*/, HANDLE* phStdErr /*= NULL*/, DWORD nPipeBufferSize /* = 0*/)
{
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));

	si.cb			= sizeof(si);
	si.dwFlags		= STARTF_USESHOWWINDOW;
	si.wShowWindow	= wShowWindow;

	HANDLE hwrite = INVALID_HANDLE_VALUE;

	if (phStdOut)
	{
		*phStdOut = INVALID_HANDLE_VALUE;
	}
	if (phStdErr)
	{
		*phStdErr = INVALID_HANDLE_VALUE;
	}
	if (bStdInput)
	{
		HANDLE hread = INVALID_HANDLE_VALUE;
		HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
		HANDLE herr = GetStdHandle(STD_ERROR_HANDLE);

		SECURITY_ATTRIBUTES   sa;

		sa.nLength				= sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle		= TRUE;

		if (CreatePipe(&hread, &hwrite, &sa, nPipeBufferSize))
		{
			if (phStdOut)
			{
				hout = INVALID_HANDLE_VALUE;
				ATLVERIFY(CreatePipe(phStdOut, &hout, &sa, nPipeBufferSize));
				SetHandleInformation(*phStdOut, HANDLE_FLAG_INHERIT, 0);
			}
			if (phStdErr)
			{
				herr = INVALID_HANDLE_VALUE;
				ATLVERIFY(CreatePipe(phStdErr, &herr, &sa, nPipeBufferSize));
				SetHandleInformation(*phStdErr, HANDLE_FLAG_INHERIT, 0);
			}
			SetHandleInformation(hwrite, HANDLE_FLAG_INHERIT, 0);

			if (phWrite != NULL)
				*phWrite = hwrite;

			si.hStdInput	= hread;
			si.hStdOutput	= hout;
			si.hStdError	= herr;
			si.dwFlags		|= STARTF_USESTDHANDLES;
		}
	}
	CTempBuffer<TCHAR> _strCmd(_tcslen(strCmd) + 1);

	_tcscpy_s(_strCmd, _tcslen(strCmd) + 1, strCmd);

	if (CreateProcess(NULL, _strCmd, NULL, NULL, bStdInput ? TRUE : FALSE, 0, NULL, strPath, &si, &pi))
	{
		CloseHandle(pi.hThread);

		if (si.dwFlags & STARTF_USESTDHANDLES)
		{
			ATLVERIFY(CloseHandle(si.hStdInput));

			if (phStdOut)
			{
				ATLVERIFY(CloseHandle(si.hStdOutput));
			}
			if (phStdErr)
			{
				ATLVERIFY(CloseHandle(si.hStdError));
			}
		}
		if (WaitForSingleObject(pi.hProcess, dwWait) != WAIT_OBJECT_0)
			return pi.hProcess;
		CloseHandle(pi.hProcess);
	}
	if (hwrite != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hwrite);

		if (phWrite != NULL)
			*phWrite = INVALID_HANDLE_VALUE;
	}
	if (phStdOut && *phStdOut != INVALID_HANDLE_VALUE)
	{
		CloseHandle(*phStdOut);
		*phStdOut = INVALID_HANDLE_VALUE;
	}
	if (phStdErr && *phStdErr != INVALID_HANDLE_VALUE)
	{
		CloseHandle(*phStdErr);
		*phStdErr = INVALID_HANDLE_VALUE;
	}
	return NULL;
}

std::wstring GetModulePath(HMODULE hModule /*= NULL*/, PCWSTR strModuleName /* = NULL*/)
{
	WCHAR FilePath[MAX_PATH];

	ATLVERIFY(GetModuleFileNameW(hModule, FilePath, sizeof(FilePath) / sizeof(WCHAR)) > 0);

	if (strModuleName)
	{
		WCHAR* pS = wcsrchr(FilePath, L'\\');

		if (pS == NULL)
		{
			wcscpy_s(FilePath, MAX_PATH, strModuleName);
		}
		else
		{
			wcscpy_s(pS + 1, MAX_PATH - (pS - FilePath) - 1, strModuleName);
		}
	}
	return std::wstring(FilePath);
}

std::wstring LocateModulePath(PCWSTR strModule, PCWSTR strDefaultExtension /*= L".exe"*/)
{
	std::wstring strResult;
	WCHAR buf[MAX_PATH + 4];

	memset(buf, 0, sizeof(buf));

	WCHAR strDrive[_MAX_DRIVE + 1];
	WCHAR strDir[_MAX_DIR + 1];
	WCHAR strName[_MAX_FNAME + 1];
	WCHAR strExt[_MAX_EXT + 1];

	memset(strDrive	, 0, sizeof(strDrive));
	memset(strDir	, 0, sizeof(strDir));
	memset(strName	, 0, sizeof(strName));
	memset(strExt	, 0, sizeof(strExt));

	// demux input
	if (_wsplitpath_s(strModule, strDrive, _MAX_DRIVE, strDir, _MAX_DIR, strName, _MAX_FNAME, strExt, _MAX_EXT) == ERROR_SUCCESS)
	{
		std::wstring strFolder;

		// we have to calculate the folder ourselves
		if (wcslen(strDrive) || wcslen(strDir))
		{
			strFolder = strDrive;
			strFolder += strDir;

			if (*strFolder.rbegin() == L'\\')
				strFolder = strFolder.substr(0, strFolder.length() - 1);
		}
		if (wcslen(strExt) == 0)
		{
			wcscpy_s(strExt, _MAX_EXT, strDefaultExtension);
		}
		// let windows API mux it in order to get the complete path
		if (::SearchPathW(strFolder.empty() ? NULL : strFolder.c_str(), strName, strExt, MAX_PATH, buf, NULL))
		{
			strResult = buf;
		}
		else if (strFolder.empty())
		{
			std::wstring strFile = std::wstring(strName) + std::wstring(strExt);

			HMODULE hModule = ::LoadLibraryW(strFile.c_str());

			if (hModule)
			{
				strResult = GetModulePath(hModule);
				FreeLibrary(hModule);
			}
		}
	}
	return strResult;
}

bool WritePPMData(HANDLE hTo, const BYTE* pFrameBuffer, int nWidth, int nHeight)
{
	char buf[64];

	// create ppm header - http://netpbm.sourceforge.net/doc/ppm.html
	sprintf_s(buf, 64, "P6 %d %d 255 ", nWidth, nHeight);

	DWORD dwWritten = 0;

	// send header and bitmap data to encoder/file
	if (!::WriteFile(hTo, buf, (DWORD)strlen(buf), &dwWritten, NULL) || !::WriteFile(hTo, pFrameBuffer, nWidth * nHeight * 3, &dwWritten, NULL))
	{
		return false;
	}
	return true;
}

ULONGLONG Shovel(HANDLE hFrom, HANDLE hTo, ULONGLONG nBytesToShovel, bool bInputHandleIsPipe /*= false*/)
{
	const ULONGLONG		sizeBuf = 0x100000;	// 1MB
	CTempBuffer<BYTE>	buf((size_t)sizeBuf);

	ULONG				dwRead		= 0;
	ULONG				dwWritten	= 0;

	ULONGLONG			n			= nBytesToShovel;
	int					nTry		= 5;

	while (n && nTry > 0)
	{
		if (!::ReadFile(hFrom, buf, (DWORD)(n < sizeBuf ? n : sizeBuf), &dwRead, NULL))
			break;

		if (dwRead == 0)
		{
			if (bInputHandleIsPipe)
			{
				nTry--;
				Sleep(5);
				continue;
			}
			break;
		}
		nTry		= 5;
		dwWritten	= 0;

		if (!::WriteFile(hTo, buf, dwRead, &dwWritten, NULL) || dwWritten != dwRead)
			return nBytesToShovel - n + dwWritten;

		n -= dwWritten;
	}
	return nBytesToShovel - n;
}

}