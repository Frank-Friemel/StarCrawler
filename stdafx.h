// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

// Change these values to use different versions
#define _WIN32_WINNT	_WIN32_WINNT_VISTA	
#define _WIN32_IE		0x0501
#define _RICHEDIT_VER	0x0500

#include <SDKDDKVer.h>

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atlmisc.h>
#include <atlCtrlx.h>
#include <atldlgs.h>
#include <atlddx.h>
#include <atlcrack.h>
#include <atlgdi.h>
#include <atlstr.h>
#include <atlsync.h>

#include <stdint.h>
#include <time.h>

#include <list>
#include <queue>
#include <deque>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <memory>
#include <set>
#include <array>

using namespace std;

#include "glm\glm.hpp"
#include "glm\gtx\transform.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "utils.h"

using namespace local_utils;

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
