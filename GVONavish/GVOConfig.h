#pragma once
#include <Windows.h>
#include <Shlwapi.h>
#include <string>
#include <vector>


//!@brief ???[?J????Åf??t?@?C??????ÅeÅ˜????????Åe?
class GVOConfig {
private:
	const std::wstring m_fileName;
	const LPCWSTR m_coreSectionName = L"core";
	const LPCWSTR m_windowSectionName = L"window";
	const LPCWSTR m_surveyCoordSectionName = L"survey"; 
#ifndef NDEBUG
	const LPCWSTR m_debugSectionName = L"debug";
#endif


public:
	std::wstring m_mapFileName;
	UINT m_pollingInterval;
	POINT m_windowPos;
	SIZE m_windowSize;
	bool m_keepForeground;
	bool m_traceShipPositionEnabled;
	bool m_speedMeterEnabled;
	bool m_shipVectorLineEnabled;
	POINT m_initialSurveyCoord;
#ifndef NDEBUG
	bool m_debugAutoCruiseEnabled;
	double m_debugAutoCruiseVelocity;
	double m_debugAutoCruiseTurnAngle;
	uint32_t m_debugAutoCruiseTurnInterval;
#endif

	GVOConfig( LPCWSTR fileName )
		: m_fileName( g_makeFullPath( fileName ) )
		, m_mapFileName( L"map.png" )
		, m_pollingInterval( 1000 )
		, m_windowPos( defaultPosition() )
		, m_windowSize( defaultSize() )
		, m_keepForeground()
		, m_traceShipPositionEnabled( true )
		, m_speedMeterEnabled( true )
		, m_shipVectorLineEnabled( true )
		, m_initialSurveyCoord( defaultSurveyCoord() )
#ifndef NDEBUG
		, m_debugAutoCruiseEnabled()
		, m_debugAutoCruiseVelocity( 1.0 )
		, m_debugAutoCruiseTurnAngle( 12.0 )
		, m_debugAutoCruiseTurnInterval( 7 * 1000 )
#endif
	{
	}
	~GVOConfig()
	{

	}
	void save()
	{
		LPCWSTR fn = m_fileName.c_str();
		LPCWSTR section;

		section = m_coreSectionName;
		::WritePrivateProfileString( section, L"map", m_mapFileName.c_str(), fn );
		::WritePrivateProfileString( section, L"pollingInterval", std::to_wstring( m_pollingInterval ).c_str(), fn );
		::WritePrivateProfileString( section, L"traceEnabled", std::to_wstring( m_traceShipPositionEnabled ).c_str(), fn );
		::WritePrivateProfileString( section, L"speedMeterEnabled", std::to_wstring( m_speedMeterEnabled ).c_str(), fn );
		::WritePrivateProfileString( section, L"shipVectorLineEnabled", std::to_wstring( m_shipVectorLineEnabled ).c_str(), fn );

		section = m_windowSectionName;
		::WritePrivateProfileString(section, L"x", std::to_wstring(m_windowPos.x).c_str(), fn);
		::WritePrivateProfileString(section, L"y", std::to_wstring(m_windowPos.y).c_str(), fn);
		::WritePrivateProfileString(section, L"cx", std::to_wstring(m_windowSize.cx).c_str(), fn);
		::WritePrivateProfileString(section, L"cy", std::to_wstring(m_windowSize.cy).c_str(), fn);
		::WritePrivateProfileString( section, L"keepForeground", std::to_wstring( m_keepForeground ).c_str(), fn );

		section = m_surveyCoordSectionName;
		::WritePrivateProfileString( section, L"x", std::to_wstring( m_initialSurveyCoord.x ).c_str(), fn );
		::WritePrivateProfileString( section, L"y", std::to_wstring( m_initialSurveyCoord.y ).c_str(), fn );

#ifndef NDEBUG
		section = m_debugSectionName;
		::WritePrivateProfileString( section, L"autoCruiseEnabled", std::to_wstring( m_debugAutoCruiseEnabled ).c_str(), fn );
		::WritePrivateProfileString( section, L"autoCruiseVelocity", std::to_wstring( m_debugAutoCruiseVelocity ).c_str(), fn );
		::WritePrivateProfileString( section, L"autoCruiseTurnAngle", std::to_wstring( m_debugAutoCruiseTurnAngle ).c_str(), fn );
		::WritePrivateProfileString( section, L"autoCruiseTurnInterval", std::to_wstring( m_debugAutoCruiseTurnInterval ).c_str(), fn );
#endif

		// ?t?@?C???Åe???o??
		::WritePrivateProfileString(NULL, NULL, NULL, fn);
	}
	void load()
	{
		LPCWSTR fn = m_fileName.c_str();
		LPCWSTR section;
		std::vector<wchar_t> buf(4096);

		section = m_coreSectionName;
		::GetPrivateProfileStringW(section, L"map", m_mapFileName.c_str(), &buf[0], buf.size(), fn);
		m_mapFileName = &buf[0];
		m_pollingInterval = ::GetPrivateProfileInt( section, L"pollingInterval", m_pollingInterval, fn );
		m_traceShipPositionEnabled = ::GetPrivateProfileInt( section, L"traceEnabled", m_traceShipPositionEnabled, fn ) != 0;
		m_speedMeterEnabled = ::GetPrivateProfileInt( section, L"speedMeterEnabled", m_speedMeterEnabled, fn ) != 0;
		m_shipVectorLineEnabled = ::GetPrivateProfileInt( section, L"shipVectorLineEnabled", m_shipVectorLineEnabled, fn ) != 0;
		

		section = m_windowSectionName;
		m_windowPos.x = ::GetPrivateProfileInt(section, L"x", m_windowPos.x, fn);
		m_windowPos.y = ::GetPrivateProfileInt(section, L"y", m_windowPos.y, fn);
		m_windowSize.cx = ::GetPrivateProfileInt(section, L"cx", m_windowSize.cx, fn);
		m_windowSize.cy = ::GetPrivateProfileInt(section, L"cy", m_windowSize.cy, fn);
		m_keepForeground = ::GetPrivateProfileInt( section, L"keepForeground", m_keepForeground, fn ) != 0;

		section = m_surveyCoordSectionName;
		m_initialSurveyCoord.x = ::GetPrivateProfileInt( section, L"x", m_initialSurveyCoord.x, fn );
		m_initialSurveyCoord.y = ::GetPrivateProfileInt( section, L"y", m_initialSurveyCoord.y, fn );

#ifndef NDEBUG
		section = m_debugSectionName;
		m_debugAutoCruiseEnabled = ::GetPrivateProfileInt( section, L"autoCruiseEnabled", m_debugAutoCruiseEnabled, fn ) != 0;
		::GetPrivateProfileString( section, L"autoCruiseVelocity", std::to_wstring( m_debugAutoCruiseVelocity).c_str(), &buf[0], buf.size(), fn );
		m_debugAutoCruiseVelocity = std::stod( std::wstring( &buf[0] ) );
		::GetPrivateProfileString( section, L"autoCruiseTurnAngle", std::to_wstring( m_debugAutoCruiseTurnAngle ).c_str(), &buf[0], buf.size(), fn );
		m_debugAutoCruiseTurnAngle = std::stod( std::wstring( &buf[0] ) );
		m_debugAutoCruiseTurnInterval = ::GetPrivateProfileInt( section, L"autoCruiseTurnInterval", m_debugAutoCruiseTurnInterval, fn );
#endif
	}

private:
	static POINT defaultPosition()
	{
		POINT pt = { CW_USEDEFAULT, 0 };
		return pt;
	}
	static SIZE defaultSize()
	{
		SIZE size = { CW_USEDEFAULT, 0 };
		return size;
	}
	static POINT defaultSurveyCoord()
	{
		// ???X?{?Åg?????W
		POINT p = {15785, 3204};
		return p;
	}
};

