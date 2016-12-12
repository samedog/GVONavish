#include "stdafx.h"
#include <vector>
#include <list>
#include <fstream>

// For reading PNG
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// For path API
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#include <CommDlg.h>
#pragma comment(lib, "Comdlg32.lib")



#include "GVONavish.h"
#include "GVOConfig.h"
#include "GVOGameProcess.h"
#include "GVOWorldMap.h"
#include "GVOShip.h"
#include "GVOShipRouteList.h"
#include "GVORenderer.h"
#include "GVOTexture.h"
#include "GVOShipRouteManageView.h"


// For drawing performance measurement at debugging
//#define GVO_PERF_CHECK



// Global variables:
HINSTANCE g_hinst;	// Current interface
HWND g_hwndMain;
HDC g_hdcMain;


// Function prototype declaration
static ATOM MyRegisterClass( HINSTANCE hInstance );
static BOOL InitInstance( HINSTANCE, int );
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
BOOL CALLBACK aboutDlgProc( HWND, UINT, WPARAM, LPARAM );
static LRESULT s_mainLoop();


// message handler
static bool s_onCreate( HWND, LPCREATESTRUCT );
static void s_onMove( HWND, WORD, WORD );
static void s_onSize( HWND, UINT, WORD, WORD );
static void s_onMouseWheel( HWND, int16_t, UINT, int16_t, int16_t );
static void s_onMouseMove( HWND, UINT, int16_t, int16_t );
static void s_onMouseLeftButtonDown( HWND, UINT, int16_t, int16_t );
static void s_onMouseLeftButtonUp( HWND, UINT, int16_t, int16_t );
static void s_onMouseLeftButtonDoubleClick( HWND, UINT, int16_t, int16_t );
static void s_onMouseRightButtonUp( HWND, UINT, int16_t, int16_t );
static void s_onPaint( HWND );


// Application processing
static std::wstring s_getMapFileName();
static void s_updateFrame(HWND);
static void s_updateWindowTitle( HWND, POINT, double );
static void s_toggleKeepForeground( HWND );
static void s_popupMenu( HWND, int16_t, int16_t );
static void s_popupCoord( HWND, int16_t, int16_t );
static void s_closeShipRoute();


// Local variables
static LPCWSTR const k_appName = L"GVONavish";		// Application Name
static LPCWSTR const k_version = L"ver 1.3.2.1";	// Version number
static LPCWSTR const k_copyright = L"copyright(c) 2014 @MandhelingFreak";	// Copyright notice (One time)

static LPCWSTR const k_windowClassName = L"GVONavish";		// Main window class name
static const LPCWSTR k_configFileName = L"GVONavish.ini";	// Settings file name
static LPCWSTR const k_appMutexName = L"Global\\{7554E265-3247-4FCA-BC60-5AA814658351}";
static HANDLE s_appMutex;

static Gdiplus::GdiplusStartupInput s_gdisi;
static ULONG_PTR s_gdiToken;

static GVOConfig s_config( k_configFileName );
static GVOGameProcess s_gvoGameProcess;
static GVORenderer s_renderer;
static GVOWorldMap s_worldMap;
const std::wstring && k_routeListFilePath = g_makeFullPath(L"RouteList.dat");
static std::unique_ptr<GVOShipRouteList> s_shipRouteList;
static POINT s_latestSurveyCoord;
static GVOVector s_latestShipVector;
static double s_latestShipVelocity;
static DWORD s_latestTimeStamp;
static const DWORD k_surveyCoordLostThreshold = 5000;
static std::unique_ptr<GVOShipRouteManageView> s_shipRouteManageView;

//Temporary placement
static std::unique_ptr<GVOTexture> s_shipTexture;

static UINT s_pollingInterval = 1000;	// Monitoring update interval (1 second)
static bool s_isDragging = false;		// Drag state flag
static SIZE s_clientSize;				// Size of client area
static POINT s_dragOrg;					// Drag origin (For moving amount calculation)

#ifdef GVO_PERF_CHECK
typedef std::deque<double> PerfCountList;
static PerfCountList s_perfCountList;
#endif





int APIENTRY _tWinMain( _In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	::SetLastError( NOERROR );
	s_appMutex = ::CreateMutex( NULL, TRUE, k_appMutexName );
	if ( ::GetLastError() == ERROR_ALREADY_EXISTS ) {
		HWND hwnd = ::FindWindow( k_windowClassName, NULL );
		if ( hwnd ) {
			::SetForegroundWindow( hwnd );
		}
		return 0;
	}

	::CoInitialize( NULL );
	TIMECAPS tc;
	::timeGetDevCaps( &tc, sizeof(tc) );
	::timeBeginPeriod( tc.wPeriodMin );
	INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES };
	::InitCommonControlsEx( &icc );
	GdiplusStartup( &s_gdiToken, &s_gdisi, NULL );
	s_config.load();

	MyRegisterClass( hInstance );

	// 	Perform initialization of the application:
	if ( !InitInstance( hInstance, nCmdShow ) ) {
		return 0;
	}

	// Main message loop
	const LRESULT retVal = s_mainLoop();

	try {
		std::ofstream ofs;
		ofs.exceptions( std::ios::badbit | std::ios::failbit );
		ofs.open( k_routeListFilePath, std::ios::out | std::ios::binary | std::ios::trunc );
		ofs << *s_shipRouteList;
		ofs.close();
	}
	catch ( const std::exception& e ) {
		::OutputDebugStringA( (std::string( "file save error:" ) + e.what() + "\n").c_str() );
		::MessageBox( NULL, L"Failed to save the route", k_appName, MB_ICONERROR );
	}
	//s_shipRouteList->saveToFile( k_routeListFilePath );
	s_gvoGameProcess.teardown();

	s_config.save();
	Gdiplus::GdiplusShutdown( s_gdiToken );
	::timeEndPeriod( tc.wPeriodMin );
	::CoUninitialize();
	return retVal;
}

static ATOM MyRegisterClass( HINSTANCE hInstance )
{
	WNDCLASSEX wcex = { sizeof(wcex) };

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDR_MAINFRAME ) );
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = (HBRUSH)::GetStockObject( BLACK_BRUSH );
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MAINFRAME);
	wcex.lpszClassName = k_windowClassName;
	wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ) );

	return RegisterClassEx( &wcex );
}

static BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	if ( !s_worldMap.loadFromFile( s_config.m_mapFileName ) ) {
		std::wstring fileName = s_getMapFileName();
		if ( !s_worldMap.loadFromFile( fileName ) ) {
			::MessageBox( NULL,
				L"Could not open the map image.",
				k_appName,
				MB_ICONERROR | MB_SETFOREGROUND | MB_OK );
			return FALSE;
		}
		s_config.m_mapFileName = fileName;
	}

	HWND hwnd;

	g_hinst = hInstance; // Store instance processing in global variable.

	DWORD exStyle = 0;
	if ( s_config.m_keepForeground ) {
		exStyle |= WS_EX_TOPMOST;
	}
	DWORD style = 0;
	style |= WS_OVERLAPPEDWINDOW;
	style |= WS_CLIPCHILDREN;
	style |= WS_CLIPSIBLINGS;
	hwnd = CreateWindowEx( exStyle, k_windowClassName, k_appName, style,
		s_config.m_windowPos.x, s_config.m_windowPos.y,
		s_config.m_windowSize.cx, s_config.m_windowSize.cy,
		NULL, NULL, hInstance, NULL );

	if ( !hwnd ) {
		return FALSE;
	}

	g_hwndMain = hwnd;
	g_hdcMain = ::GetDC( g_hwndMain );

	s_renderer.setup( &s_config, g_hdcMain, &s_worldMap );

	s_shipRouteList.reset( new GVOShipRouteList() );
	try {
		std::ifstream ifs;
		ifs.open( k_routeListFilePath, std::ios::in | std::ios::binary );
		if ( ifs ) {
			ifs.exceptions( std::ios::badbit | std::ios::failbit );
			ifs >> *s_shipRouteList;
			ifs.close();
		}
	}
	catch ( const std::exception& e ) {
		::OutputDebugStringA( (std::string( "file load error:" ) + e.what() + "\n").c_str() );
		::MessageBox( NULL, L"Failed to read path", k_appName, MB_ICONERROR );
	}
	s_pollingInterval = s_config.m_pollingInterval;
	s_gvoGameProcess.setup( s_config );

	s_updateWindowTitle( hwnd, s_config.m_initialSurveyCoord, s_renderer.viewScale() );

	ShowWindow( hwnd, nCmdShow );
	UpdateWindow( hwnd );

	return TRUE;
}

static LRESULT s_mainLoop()
{
	MSG msg;
	HACCEL hAccelTable;

	hAccelTable = LoadAccelerators( g_hinst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

	for ( ;; ) {
		if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
			if ( msg.message == WM_QUIT ) {
				break;
			}
			if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) ) {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			continue;
		}
		std::vector<HANDLE> handles;

		// 	Add handles to monitor.
		if ( s_gvoGameProcess.processHandle() ) {
			handles.push_back( s_gvoGameProcess.processHandle() );
		}
		//handles.push_back( s_pollingTimerEvent );
		handles.push_back( s_gvoGameProcess.dataReadyEvent() );

		if ( handles.empty() ) {
			::WaitMessage();
			continue;
		}

		DWORD const waitResult = ::MsgWaitForMultipleObjects( handles.size(), &handles[0], FALSE, INFINITE, QS_ALLINPUT );
		if ( handles.size() <= waitResult ) {
			continue;
		}

		// Check the handle to be monitored.
		HANDLE const activeHandle = handles[waitResult];

		if ( activeHandle == s_gvoGameProcess.processHandle() ) {
			// The game process is over.
			s_gvoGameProcess.clear();
			continue;
		}

		if ( activeHandle == s_gvoGameProcess.dataReadyEvent() ) {
			s_updateFrame( g_hwndMain );
			continue;
		}
	}
	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wp, LPARAM lp )
{
	int wmId, wmEvent;

	switch ( message ) {
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		s_onPaint( hwnd );
		break;

	case WM_TIMER:
		s_updateFrame( hwnd );
		break;

	case WM_MOVE:
		s_onMove( hwnd, int16_t( LOWORD( lp ) ), int16_t( HIWORD( lp ) ) );
		break;
	case WM_SIZE:
		s_onSize( hwnd, wp, LOWORD( lp ), HIWORD( lp ) );
		break;

	case WM_COMMAND:
		wmId = LOWORD( wp );
		wmEvent = HIWORD( wp );
		// Analysis of the selected menu :
		switch ( wmId ) {
		case IDM_ABOUT:
			::DialogBox( g_hinst, MAKEINTRESOURCE( IDD_ABOUTBOX ), hwnd, aboutDlgProc );
			break;
		case IDM_EXIT:
			DestroyWindow( hwnd );
			break;
		case IDM_TOGGLE_TRACE_SHIP:
			s_config.m_traceShipPositionEnabled = !s_config.m_traceShipPositionEnabled;
			s_renderer.enableTraceShip( s_config.m_traceShipPositionEnabled );
			break;
		case IDM_ERASE_SHIP_ROUTE:
			s_shipRouteList->clearAllItems();
			break;
		case IDM_TOGGLE_KEEP_FOREGROUND:
			s_toggleKeepForeground( hwnd );
			break;
		case IDM_TOGGLE_SPEED_METER:
			s_config.m_speedMeterEnabled = !s_config.m_speedMeterEnabled;
			s_renderer.enableSpeedMeter( s_config.m_speedMeterEnabled );
			::InvalidateRect( hwnd, NULL, FALSE );
			break;
		case IDM_TOGGLE_VECTOR_LINE:
			s_config.m_shipVectorLineEnabled = !s_config.m_shipVectorLineEnabled;
			s_renderer.setVisibleShipRoute( s_config.m_shipVectorLineEnabled );
			break;
		case IDM_SAME_SCALE:
			if ( s_renderer.viewScale() != 1.0 ) {
				s_renderer.resetViewScale();
			}
			break;
		case IDM_ZOOM_IN:
			if ( s_renderer.zoomIn() ) {
#ifdef GVO_PERF_CHECK
				s_perfCountList.clear();
#endif
				s_updateWindowTitle( hwnd, s_latestSurveyCoord, s_renderer.viewScale() );
				::InvalidateRect( hwnd, NULL, FALSE );
			}
			break;
		case IDM_ZOOM_OUT:
			if ( s_renderer.zoomOut() ) {
#ifdef GVO_PERF_CHECK
				s_perfCountList.clear();
#endif
				s_updateWindowTitle( hwnd, s_latestSurveyCoord, s_renderer.viewScale() );
				::InvalidateRect( hwnd, NULL, FALSE );
			}
			break;
		case IDM_SHOW_SHIPROUTEMANAGEVIEW:
			if ( !s_shipRouteManageView.get() ) {
				s_shipRouteManageView.reset( new GVOShipRouteManageView() );
				if ( !s_shipRouteManageView->setup( *s_shipRouteList.get() ) ) {
					::MessageBox( hwnd, L"Something went wrong", L"Error", MB_OK | MB_ICONERROR );
				}
			}
			else {
				s_shipRouteManageView->activate();
			}
			break;
#ifndef NDEBUG
		case IDM_TOGGLE_DEBUG_AUTO_CRUISE:
			s_config.m_debugAutoCruiseEnabled = !s_config.m_debugAutoCruiseEnabled;
			s_gvoGameProcess.enableDebugAutoCruise( s_config.m_debugAutoCruiseEnabled );
			break;
		case IDM_DEBUG_CLOSE_ROUTE:
			s_closeShipRoute();
			break;
		case IDM_DEBUG_INTERVAL_NORMAL:
			s_pollingInterval = 1000;
			s_gvoGameProcess.setPollingInterval( s_pollingInterval );
			break;
		case IDM_DEBUG_INTERVAL_HIGH:
			s_pollingInterval = 1;
			s_gvoGameProcess.setPollingInterval( s_pollingInterval );
			break;
#endif
		default:
			return DefWindowProc( hwnd, message, wp, lp );
		}
		break;

	case WM_MOUSEWHEEL:
		s_onMouseWheel( hwnd, HIWORD( wp ), LOWORD( wp ), int16_t( LOWORD( lp ) ), int16_t( HIWORD( lp ) ) );
		break;
	case WM_MOUSEMOVE:
		s_onMouseMove( hwnd, wp, int16_t( LOWORD( lp ) ), int16_t( HIWORD( lp ) ) );
		break;
	case WM_LBUTTONDOWN:
		s_onMouseLeftButtonDown( hwnd, wp, int16_t( LOWORD( lp ) ), int16_t( HIWORD( lp ) ) );
		break;
	case WM_LBUTTONUP:
		s_onMouseLeftButtonUp( hwnd, wp, int16_t( LOWORD( lp ) ), int16_t( HIWORD( lp ) ) );
		break;
	case WM_RBUTTONUP:
		s_onMouseRightButtonUp( hwnd, wp, int16_t( LOWORD( lp ) ), int16_t( HIWORD( lp ) ) );
		break;
	case WM_LBUTTONDBLCLK:
		s_onMouseLeftButtonDoubleClick( hwnd, wp, int16_t( LOWORD( lp ) ), int16_t( HIWORD( lp ) ) );
		break;

	case WM_CREATE:
		if ( !s_onCreate( hwnd, reinterpret_cast<LPCREATESTRUCT>(lp) ) ) {
			return -1;
		}
		break;
	case WM_DESTROY:
		if ( s_shipRouteManageView.get() ) {
			s_shipRouteManageView.reset();
		}
		s_renderer.teardown();
		PostQuitMessage( 0 );
		break;
	default:
		return DefWindowProc( hwnd, message, wp, lp );
	}
	return 0;
}


static bool s_onCreate( HWND hwnd, LPCREATESTRUCT /*cs*/ )
{
	return true;
}


static void s_onMove( HWND hwnd, WORD /*cx*/, WORD /*cy*/ )
{
	const DWORD style = ::GetWindowLong( hwnd, GWL_STYLE );
	if ( style & WS_MAXIMIZE ) {
		return;
	}
	RECT rc = { 0 };
	::GetWindowRect( hwnd, &rc );
	s_config.m_windowPos.x = rc.left;
	s_config.m_windowPos.y = rc.top;
}


static void s_onSize( HWND hwnd, UINT state, WORD cx, WORD cy )
{
	RECT rc = { 0 };

	switch ( state ) {
	case SIZE_RESTORED:
		::GetWindowRect( hwnd, &rc );
		s_config.m_windowSize.cx = rc.right - rc.left;
		s_config.m_windowSize.cy = rc.bottom - rc.top;
		break;
	case SIZE_MAXIMIZED:
		break;
	default:
		return;
	}

	if ( s_clientSize.cx != cx || s_clientSize.cy != cy ) {
		s_clientSize.cx = cx;
		s_clientSize.cy = cy;
		s_renderer.setViewSize( s_clientSize );
	}
}


static void s_onMouseWheel( HWND hwnd, int16_t delta, UINT vkey, int16_t x, int16_t y )
{
	bool isChanged = false;

	if ( 0 < delta ) {
		isChanged = s_renderer.zoomIn();
	}
	else {
		isChanged = s_renderer.zoomOut();
	}

	if ( isChanged ) {
#ifdef GVO_PERF_CHECK
		s_perfCountList.clear();
#endif
		s_updateWindowTitle( hwnd, s_latestSurveyCoord, s_renderer.viewScale() );
		::InvalidateRect( hwnd, NULL, FALSE );
	}
}


static void s_onMouseMove( HWND hwnd, UINT vkey, int16_t x, int16_t y )
{
	if ( s_isDragging ) {
		const int dx = x - s_dragOrg.x;
		const int dy = y - s_dragOrg.y;
		const int threshold = 1;	// If the sensitivity is too good, follow-up will easily be cut off, so take appropriate measures
		if ( s_config.m_traceShipPositionEnabled ) {
			if ( ::abs( dx ) <= threshold && ::abs( dy ) < threshold ) {
				return;
			}
		}
		const POINT offset = { -dx, -dy };

		s_renderer.offsetFocusInViewCoord( offset );
		::InvalidateRect( hwnd, NULL, FALSE );

		s_dragOrg.x = x;
		s_dragOrg.y = y;
		s_config.m_traceShipPositionEnabled = false;
		s_renderer.enableTraceShip( s_config.m_traceShipPositionEnabled );
	}
	else {

	}
}


static void s_onMouseLeftButtonDown( HWND hwnd, UINT vkey, int16_t x, int16_t y )
{
	if ( s_isDragging ) {

	}
	else {
		::SetCapture( hwnd );
		s_isDragging = true;
		s_dragOrg.x = x;
		s_dragOrg.y = y;
	}
}


static void s_onMouseLeftButtonUp( HWND hwnd, UINT vkey, int16_t x, int16_t y )
{
	if ( s_isDragging ) {
		::ReleaseCapture();
		s_isDragging = false;
		s_dragOrg.x = 0;
		s_dragOrg.y = 0;
	}
	else {

	}
}


static void s_onMouseLeftButtonDoubleClick( HWND hwnd, UINT vkey, int16_t x, int16_t y )
{
	if ( s_isDragging ) {

	}
	else {
		s_popupCoord( hwnd, x, y );
	}
}


static void s_onMouseRightButtonUp( HWND hwnd, UINT vkey, int16_t x, int16_t y )
{
	if ( s_isDragging ) {

	}
	else {
		s_popupMenu( hwnd, x, y );
	}
}


static void s_onPaint( HWND hwnd )
{
#ifdef GVO_PERF_CHECK
	const int64_t perfBegin = g_queryPerformanceCounter();
#endif
	s_renderer.render( s_latestShipVector, s_latestShipVelocity, s_shipTexture.get(), s_shipRouteList.get() );
	::ValidateRect( hwnd, NULL );

#ifdef GVO_PERF_CHECK
	const int64_t perfEnd = g_queryPerformanceCounter();
	const int64_t freq = g_queryPerformanceFrequency();
	const double deltaPerSec = (double(perfEnd - perfBegin) / double(freq)) * 1000.0;
	s_perfCountList.push_back(deltaPerSec);

	const double ave = std::accumulate( s_perfCountList.begin(), s_perfCountList.end(), 0.0 ) / s_perfCountList.size();
	if ( 100 < s_perfCountList.size() ) {
		s_perfCountList.pop_front();
	}

	std::wstring s;
	s = std::wstring( L"Drawing speed:" ) + std::to_wstring( ave ) + L"(ms)\n";
	::SetWindowText( hwnd, s.c_str() );
#endif
}


// Let me select a map image
static std::wstring s_getMapFileName()
{
	wchar_t dir[MAX_PATH] = { 0 };
	::GetModuleFileName( g_hinst, dir, _countof( dir ) );
	::PathRemoveFileSpec( dir );

	wchar_t filePath[MAX_PATH] = { 0 };
	OPENFILENAME ofn = { sizeof(ofn) };
	ofn.lpstrTitle = L"Please select a map image file.";
	ofn.lpstrInitialDir = &dir[0];
	ofn.lpstrFilter = L"Image file\0L" L"*.bmp;*.jpg;*.jpeg;*.png;*.gif;*.tif;*.tiff" L"\0"
		L"All Files\0" L"*.*" L"\0\0";
	ofn.Flags = OFN_READONLY | OFN_FILEMUSTEXIST;
	ofn.nMaxFile = _countof( filePath );
	ofn.lpstrFile = &filePath[0];
	if ( !::GetOpenFileName( &ofn ) ) {
		return L"";
	}
	return filePath;
}


static void s_updateFrame( HWND hwnd )
{
	std::vector<GVOGameStatus> gameStats;
	gameStats = s_gvoGameProcess.getState();
	if ( gameStats.empty() ) {
		return;
	}

	// If the icon texture of the game process has not been created, it attempts to create it
	if ( !s_shipTexture.get() ) {
		const GVOImage * image = s_gvoGameProcess.shipIconImage();
		if ( image ) {
			s_shipTexture.reset( s_renderer.createTextureFromImage( *image ) );
		}
	}

	for ( std::vector<GVOGameStatus>::const_iterator it = gameStats.begin(); it != gameStats.end(); ++it ) {
		const GVOGameStatus& status = *it;
		s_latestSurveyCoord = status.m_surveyCoord;
		s_latestShipVector = status.m_shipVector;
		s_latestShipVelocity = status.m_shipVelocity;
		s_config.m_initialSurveyCoord = s_latestSurveyCoord;
		s_renderer.setShipPositionInWorld( s_latestSurveyCoord );
		if ( (s_latestTimeStamp + k_surveyCoordLostThreshold) < status.m_timeStamp ) {
			s_closeShipRoute();
		}
		s_latestTimeStamp = status.m_timeStamp;
		s_shipRouteList->addRoutePoint( s_worldMap.normalizedPoint( s_latestSurveyCoord ) );
	}
#ifndef GVO_PERF_CHECK
	s_updateWindowTitle( hwnd, s_latestSurveyCoord, s_renderer.viewScale() );
#endif
	::InvalidateRect( hwnd, NULL, FALSE );
}


static void s_updateWindowTitle( HWND hwnd, POINT surveyCoord, double viewScale )
{
	std::vector<wchar_t> buf( 4096 );
	::swprintf( &buf[0], buf.size(), L"%d,%d - (%.1f%%) - %s %s",
		surveyCoord.x, surveyCoord.y,
		viewScale * 100.0,
		k_appName, k_version
		);
	::SetWindowText( hwnd, &buf[0] );
}


static void s_toggleKeepForeground( HWND hwnd )
{
	if ( ::GetWindowLong( hwnd, GWL_EXSTYLE ) & WS_EX_TOPMOST ) {
		::SetWindowPos( hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );
		s_config.m_keepForeground = false;
	}
	else {
		::SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );
		s_config.m_keepForeground = true;
	}
}


static void s_popupMenu( HWND hwnd, int16_t x, int16_t y )
{
	HMENU hmenu = ::LoadMenu( g_hinst, MAKEINTRESOURCE( IDR_POPUPMENU ) );
	HMENU popupMenu = ::GetSubMenu( hmenu, 0 );

	::CheckMenuItem( popupMenu, IDM_TOGGLE_TRACE_SHIP, s_config.m_traceShipPositionEnabled ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( popupMenu, IDM_TOGGLE_KEEP_FOREGROUND, s_config.m_keepForeground ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( popupMenu, IDM_TOGGLE_SPEED_METER, s_config.m_speedMeterEnabled ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( popupMenu, IDM_TOGGLE_VECTOR_LINE, s_config.m_shipVectorLineEnabled ? MF_CHECKED : MF_UNCHECKED );

#ifndef NDEBUG
	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_TYPE | MIIM_ID;
	mii.fType = MFT_STRING;

	// Switching automatic navigation for debugging
	mii.wID = IDM_TOGGLE_DEBUG_AUTO_CRUISE;
	mii.dwTypeData = L"[DEBUG]Enable automatic sailing";
	::InsertMenuItem( popupMenu, ::GetMenuItemCount( popupMenu ), TRUE, &mii );
	::CheckMenuItem( popupMenu, IDM_TOGGLE_DEBUG_AUTO_CRUISE, s_config.m_debugAutoCruiseEnabled ? MF_CHECKED : MF_UNCHECKED );

	mii.wID = IDM_DEBUG_CLOSE_ROUTE;
	mii.dwTypeData = L"[DEBUG]Close route";
	::InsertMenuItem( popupMenu, ::GetMenuItemCount( popupMenu ), TRUE, &mii );

	mii.wID = IDM_DEBUG_INTERVAL_NORMAL;
	mii.dwTypeData = L"[DEBUG]Update interval - standard";
	::InsertMenuItem( popupMenu, ::GetMenuItemCount( popupMenu ), TRUE, &mii );

	mii.wID = IDM_DEBUG_INTERVAL_HIGH;
	mii.dwTypeData = L"[DEBUG]Update interval - high";
	::InsertMenuItem( popupMenu, ::GetMenuItemCount( popupMenu ), TRUE, &mii );
#endif

	// While the menu is displayed, it can not escape from the message pump, so monitor the update with the timer.
	const UINT_PTR timerID = ::SetTimer( hwnd, 0, s_pollingInterval, NULL );
	s_updateFrame( hwnd );

	POINT p = { x, y };
	::ClientToScreen( hwnd, &p );
	::TrackPopupMenu( popupMenu, TPM_NONOTIFY | TPM_NOANIMATION | TPM_LEFTALIGN | TPM_TOPALIGN,
		p.x, p.y, 0, hwnd, NULL );
	::DestroyMenu( popupMenu );

	::KillTimer( hwnd, timerID );
}


static void s_popupCoord( HWND hwnd, int16_t x, int16_t y )
{
}


static void s_closeShipRoute()
{
	s_shipRouteList->closeRoute();
}


BOOL CALLBACK aboutDlgProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch ( msg ) {
	case WM_INITDIALOG:
	{
		std::wstring versionString = std::wstring(k_appName) + L" " + k_version;
		std::wstring copyRightString = std::wstring( k_copyright );

		::SetDlgItemText( hwnd, IDC_VERSION_LABEL, versionString.c_str() );
		::SetDlgItemText( hwnd, IDC_COPYRIGHT_LABEL, copyRightString.c_str() );
	}
		break;
	case WM_COMMAND:
		switch ( LOWORD( wp ) ) {
		case IDOK:
		case IDCANCEL:
			::EndDialog( hwnd, 0 );
			break;
		default:
			return FALSE;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
