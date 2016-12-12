#include "stdafx.h"
#include <process.h>
#include "GVONavish.h"
#include "GVOGameProcess.h"
#include "GVOWorldMap.h"
#include "GVOSurveyCoordExtractor.h"


// 画像解析デバッグ用。GVOGameProcess
//#define GVO_ANALYZE_DEBUG

extern HWND g_hwndMain;
extern HDC g_hdcMain;

namespace {
#ifdef GVO_ANALYZE_DEBUG
	LPCWSTR const k_debugImageFileName = L"..\\debug.png";
#endif

#ifndef NDEBUG
	// デバッグ用自動航行変数
	static double s_xDebugAutoCruise;
	static double s_yDebugAutoCruise;
	static double s_debugAutoCruiseAngle = 0;
	static bool s_debugAutoCruiseEnabled = false;
	static double s_debugAutoCruiseVelocity = 0;
	static uint32_t s_debugAutoCruiseTurnInterval;
	static double s_debugAutoCruiseTurnAngle;
#endif

	LPWSTR const k_gvoWindowClassName = L"Greate Voyages Online Game MainFrame";
	LPWSTR const k_gvoWindowCaption = L"大航海時代 Online";

	const POINT k_surveyCoordOffsetFromRightBottom = { 70, 273 };
	const SIZE k_surveyCoordSize = { 60, 11 };
};



void GVOGameProcess::clear()
{
	if ( m_process ) {
		::CloseHandle( m_process );
		m_process = NULL;
	}
	m_window = NULL;
}


void GVOGameProcess::setup( const GVOConfig& config )
{
	m_surveyCoord = config.m_initialSurveyCoord;
	m_ship.setInitialSurveyCoord( config.m_initialSurveyCoord );
	m_pollingInterval = config.m_pollingInterval;
#ifndef NDEBUG
	s_xDebugAutoCruise = config.m_initialSurveyCoord.x;
	s_yDebugAutoCruise = config.m_initialSurveyCoord.y;
	s_debugAutoCruiseEnabled = config.m_debugAutoCruiseEnabled;
	s_debugAutoCruiseVelocity = config.m_debugAutoCruiseVelocity;
	s_debugAutoCruiseTurnInterval = config.m_debugAutoCruiseTurnInterval;
	s_debugAutoCruiseTurnAngle = config.m_debugAutoCruiseTurnAngle;
#endif

	m_pollingTimerEventID = ::timeSetEvent( m_pollingInterval, 1,
		LPTIMECALLBACK( m_pollingTimerEvent ),
		0, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET );
	m_threadQuitSignal = ::CreateEvent( NULL, TRUE, FALSE, NULL );
	m_workerThread = (HANDLE)::_beginthreadex( NULL, 0, threadMainThunk, this, 0, NULL );
}


void GVOGameProcess::teardown()
{
	if ( m_workerThread ) {
		::SetEvent( m_threadQuitSignal );
		::WaitForSingleObject( m_workerThread, INFINITE );
		::CloseHandle( m_workerThread );
		::CloseHandle( m_threadQuitSignal );
		m_threadQuitSignal = NULL;
		m_workerThread = NULL;
	}
	if ( m_pollingTimerEventID ) {
		::timeKillEvent( m_pollingTimerEventID );
		m_pollingTimerEventID = 0;
	}
}


#ifndef NDEBUG

void GVOGameProcess::enableDebugAutoCruise( bool enabled )
{
	s_debugAutoCruiseEnabled = enabled;
}


void GVOGameProcess::setPollingInterval( DWORD interval )
{
	m_pollingInterval = interval;
	if ( m_pollingTimerEventID ) {
		::timeKillEvent( m_pollingTimerEventID );
	}
	m_pollingTimerEventID = ::timeSetEvent( m_pollingInterval, 1,
		LPTIMECALLBACK( m_pollingTimerEvent ),
		0, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET );
}

#endif

bool GVOGameProcess::updateState()
{
	GVOGameStatus status;

	if ( !m_window ) {
		m_window = ::FindWindow( k_gvoWindowClassName, k_gvoWindowCaption );
		if ( m_window ) {
			if ( !m_process ) {
				DWORD pid = 0;
				::GetWindowThreadProcessId( m_window, &pid );
				m_process = ::OpenProcess( SYNCHRONIZE, FALSE, pid );
			}
			extractGameIcon();
		}
	}

#ifdef GVO_ANALYZE_DEBUG
	{
		static bool done = false;
		if ( !done ) {
			done = true;
		}
		else {
			return false;
		}

		static GVOImage debugImage;
		if ( !debugImage.bitmapHandle() ) {
			if ( !debugImage.loadFromFile( ::g_makeFullPath( k_debugImageFileName ) ) ) {
				::MessageBox( NULL, L"デバッグイメージがないよ", L"えらー", MB_ICONERROR );
				exit( 0 );
			}
		}
		HDC hdc = ::GetWindowDC( ::GetDesktopWindow() );
		HDC hdcMem = ::CreateCompatibleDC( hdc );
		::SaveDC( hdcMem );
		::SelectObject( hdcMem, debugImage.bitmapHandle() );
		grabImage( hdcMem, POINT(), debugImage.size() );

		::RestoreDC( hdcMem, -1 );
		::DeleteDC( hdcMem );
		::ReleaseDC( NULL, hdc );

		updateSurveyCoord();
		return true;
	}
#endif

#ifndef NDEBUG
	if ( s_debugAutoCruiseEnabled ) {
		static bool isRandInitialized = false;
		if ( !isRandInitialized ) {
			srand( ::timeGetTime() );
			isRandInitialized = true;
		}

		const double rad = ((s_debugAutoCruiseAngle)* M_PI) / 180;
		const double vx = ::cos( rad );
		const double vy = ::sin( rad );

		s_xDebugAutoCruise += vx * s_debugAutoCruiseVelocity;
		s_yDebugAutoCruise += vy * s_debugAutoCruiseVelocity;

		static DWORD tick = ::timeGetTime();
		static DWORD count = 0;
		if ( (tick + s_debugAutoCruiseTurnInterval) < ::timeGetTime() ) {
			if ( 10 < (++count) ) {
				count = 0;
				s_debugAutoCruiseAngle += 90 + (LONG( rand() / double( RAND_MAX ) * 90 ) & ~0x1);
			}
			else {
				s_debugAutoCruiseAngle += (rand() & 1) ? s_debugAutoCruiseTurnAngle : -s_debugAutoCruiseTurnAngle;
			}
			tick = ::timeGetTime();
		}
		s_debugAutoCruiseAngle = fmod( ::fabs( s_debugAutoCruiseAngle ), 360 );

		if ( s_xDebugAutoCruise < 0 ) {
			s_xDebugAutoCruise += k_worldWidth;
		}
		if ( s_yDebugAutoCruise < 0 ) {
			s_yDebugAutoCruise += k_worldHeight;
		}
		s_xDebugAutoCruise = fmod( s_xDebugAutoCruise, (double)k_worldWidth );
		s_yDebugAutoCruise = fmod( s_yDebugAutoCruise, (double)k_worldHeight );

		//// 地図を跨ぐ処理の確認用デバッグコード
		//if ( 100 <= s_xDebugAutoCruise && s_xDebugAutoCruise <= (GVOWorldMap::k_worldWidth - 100) ) {
		//	s_xDebugAutoCruise = 0;
		//}

		m_surveyCoord.x = LONG( s_xDebugAutoCruise );
		m_surveyCoord.y = LONG( s_yDebugAutoCruise );
		uint32_t timeStamp = ::timeGetTime();
		m_speedMeter.updateVelocity( m_ship.velocity(), timeStamp );
		m_ship.updateWithSurveyCoord( m_surveyCoord, timeStamp );

		status.m_surveyCoord = m_surveyCoord;
		status.m_shipVector = m_ship.vector();
		status.m_shipVelocity = m_speedMeter.velocity();
		status.m_timeStamp = timeStamp;

		::EnterCriticalSection( &m_lock );
		m_statusArray.push_back( status );
		::SetEvent( m_dataReadyEvent );
		::LeaveCriticalSection( &m_lock );
		return true;
	}
#endif

	if ( m_window ) {
		RECT rc;
		POINT clientOrg = { 0 };
		::ClientToScreen( m_window, &clientOrg );
		::GetClientRect( m_window, &rc );
		SIZE size;
		size.cx = rc.right;
		size.cy = rc.bottom;

		HDC hdc = ::GetDC( ::GetDesktopWindow() );
		if ( !hdc ) {
			return false;
		}
		grabImage( hdc, clientOrg, size );
		::ReleaseDC( ::GetDesktopWindow(), hdc );
		m_timeStamp = ::timeGetTime();

		if ( !updateSurveyCoord() ) {
			return false;
		}

		m_speedMeter.updateVelocity( m_ship.velocity(), m_timeStamp );
		m_ship.updateWithSurveyCoord( m_surveyCoord, m_timeStamp );

		status.m_surveyCoord = m_surveyCoord;
		status.m_shipVector = m_ship.vector();
		status.m_shipVelocity = m_speedMeter.velocity();
		status.m_timeStamp = m_timeStamp;

		::EnterCriticalSection( &m_lock );
		m_statusArray.push_back( status );
		::SetEvent( m_dataReadyEvent );
		::LeaveCriticalSection( &m_lock );

		return true;
	}
	return false;
}

std::vector<GVOGameStatus> GVOGameProcess::getState()
{
	std::vector<GVOGameStatus> statusArray;

	::EnterCriticalSection( &m_lock );
	m_statusArray.swap( statusArray );
	::ResetEvent( m_dataReadyEvent );
	::LeaveCriticalSection( &m_lock );

	return statusArray;
}


UINT CALLBACK GVOGameProcess::threadMainThunk( LPVOID arg )
{
	GVOGameProcess *self = reinterpret_cast<GVOGameProcess *>(arg);
	self->threadMain();
	return 0;
}


void GVOGameProcess::threadMain()
{
	std::vector<HANDLE> signals;
	signals.push_back( m_threadQuitSignal );
	signals.push_back( m_pollingTimerEvent );

	for ( ; ; ) {
		const DWORD ret = ::WaitForMultipleObjects( signals.size(), &signals[0], FALSE, INFINITE );
		if ( signals.size() <= ret ) {
			exit( -1 );
		}
		HANDLE const active = signals[ret];
		if ( active == m_threadQuitSignal ) {
			break;
		}

		if ( active == m_pollingTimerEvent ) {
			::ResetEvent( m_pollingTimerEvent );
			
			updateState();

			continue;
		}
	}
}


void GVOGameProcess::grabImage( HDC hdc, const POINT& offset, const SIZE& size )
{
	if ( !m_surveyCoordImage.bitmapHandle() ) {
		m_surveyCoordImage.createImage( k_surveyCoordSize );
	}

	HDC hdcMem = ::CreateCompatibleDC( hdc );
	::SaveDC( hdcMem );

	const int leftEdge = offset.x;
	const int rightEdge = leftEdge + size.cx;
	const int topEdge = offset.y;
	const int bottomEdge = offset.y + size.cy;

	// 測量座標キャプチャ
	const int xSurvey = rightEdge - k_surveyCoordOffsetFromRightBottom.x;
	const int ySurvey = bottomEdge - k_surveyCoordOffsetFromRightBottom.y;
	::SelectObject( hdcMem, m_surveyCoordImage.bitmapHandle() );
	::BitBlt( hdcMem, 0, 0, k_surveyCoordSize.cx, k_surveyCoordSize.cy, hdc, xSurvey, ySurvey, SRCCOPY );
	::GdiFlush();

	::RestoreDC( hdcMem, -1 );
	::DeleteDC( hdcMem );
}


bool GVOGameProcess::updateSurveyCoord()
{
	bool succeeded = false;
	GVOSurveyCoordExtractor extractor( m_surveyCoordImage );

	const std::vector<int>& values = extractor.extractNumbers();
	if ( values.size() == 2 ) {
		m_surveyCoord.x = values[0];
		m_surveyCoord.y = values[1];
		succeeded = true;
	}
	return succeeded;
}


void GVOGameProcess::extractGameIcon()
{
	if ( m_shipIconImage.bitmapHandle() ) {
		return;
	}

	HICON icon = (HICON)::GetClassLongPtr( m_window, GCLP_HICONSM );
	if ( !icon ) {
		return;
	}

	HDC hdcMem = ::CreateCompatibleDC( g_hdcMain );
	::SaveDC( hdcMem );

	ICONINFO iconInfo = { 0 };
	::GetIconInfo( icon, &iconInfo );
	BITMAP bmp = { 0 };
	::GetObject( iconInfo.hbmColor, sizeof(bmp), &bmp );
	const int width = bmp.bmWidth;
	const int height = bmp.bmHeight;

	::EnterCriticalSection( &m_lock );
	m_shipIconImage.createImage( width, height, k_GVOPixelFormat_RGBA );

	uint32_t * d = reinterpret_cast<uint32_t*>(m_shipIconImage.mutableImageBits());
	for ( int y = 0; y < height; ++y ) {
		for ( int x = 0; x < width; ++x ) {
			::SelectObject( hdcMem, iconInfo.hbmColor );
			COLORREF color = ::GetPixel( hdcMem, x, y );
			const uint8_t r = GetRValue( color );
			const uint8_t g = GetGValue( color );
			const uint8_t b = GetBValue( color );
			::SelectObject( hdcMem, iconInfo.hbmMask );
			COLORREF mask = ::GetPixel( hdcMem, x, y );
			const uint8_t a = mask ? 0x00 : 0xFF;
			// DIBのBGRAフォーマットで格納する
			*d++ = RGB( b, g, r ) | (a << 24);
		}
	}
	::LeaveCriticalSection( &m_lock );

	::RestoreDC( hdcMem, -1 );
	::DeleteDC( hdcMem );
}
