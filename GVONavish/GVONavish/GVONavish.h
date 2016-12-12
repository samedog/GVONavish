#pragma once

#include "resource.h"
#include <Shlwapi.h>
#include <string>

#include "GVONoncopyable.h"

extern HINSTANCE g_hinst;
extern HWND g_hwndMain;
extern HDC g_hdcMain;

const int32_t k_worldWidth = 16384;	//!<@brief ƒQ[ƒ€¢ŠE‚Ì•
const int32_t k_worldHeight = 8192;	//!<@brief ƒQ[ƒ€¢ŠE‚Ì‚‚³



inline std::wstring g_makeFullPath(const std::wstring& fileName)
{
	wchar_t filePath[MAX_PATH] = { 0 };

	if (::PathIsRelative(fileName.c_str())) {
		wchar_t dir[MAX_PATH] = { 0 };
		::GetModuleFileName(::GetModuleHandle(NULL), dir, _countof(dir));
		::PathRemoveFileSpec(dir);
		::PathCombine(filePath, dir, fileName.c_str());
	}
	else {
		::lstrcpy(filePath, fileName.c_str());
	}
	return filePath;
}

inline double g_degreeFromRadian( const double radian )
{
	return radian * (180.0 / M_PI);
}

inline double g_radianFromDegree( const double degree )
{
	return degree * (M_PI / 180.0);
}

inline int64_t g_queryPerformanceCounter()
{
	int64_t v = 0;
	::QueryPerformanceCounter( (LARGE_INTEGER *)&v );
	return v;
}

inline int64_t g_queryPerformanceFrequency()
{
	int64_t v = 0;
	::QueryPerformanceFrequency( (LARGE_INTEGER *)&v );
	return v;
}


inline RECT s_windowRect( HWND hwnd )
{
	RECT rc = { 0 };
	::GetWindowRect( hwnd, &rc );
	return std::move( rc );
}


inline RECT s_clientRect( HWND hwnd )
{
	RECT rc = { 0 };
	::GetClientRect( hwnd, &rc );
	return rc;
}


inline RECT s_clientRectFromScreenRect( HWND hwnd, const RECT & rc )
{
	RECT rcOut = rc;
	::ScreenToClient( hwnd, reinterpret_cast<LPPOINT>(&rcOut.left) );
	::ScreenToClient( hwnd, reinterpret_cast<LPPOINT>(&rcOut.right) );
	return rcOut;
}


inline RECT s_screenRectFromClientRect( HWND hwnd, const RECT & rc )
{
	RECT rcOut = rc;
	::ClientToScreen( hwnd, reinterpret_cast<LPPOINT>(&rcOut.left) );
	::ClientToScreen( hwnd, reinterpret_cast<LPPOINT>(&rcOut.right) );
	return rcOut;
}
