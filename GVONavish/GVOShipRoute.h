#pragma once
#include <deque>
#include <ctime>
#include "GVONormalizedPoint.h"


//!@brief çqòH
class GVOShipRoute {
	friend std::ostream & operator << (std::ostream& os, GVOShipRoute& shipRoute);
	friend std::istream & operator >> (std::istream& is, GVOShipRoute& shipRoute);

public:
	typedef std::vector<GVONormalizedPoint> Line;
	typedef std::deque<Line> Lines;
private:
	Lines m_lines;
	double m_length = 0.0;
	bool m_favorite = false;
	bool m_hilight = false;
	bool m_fixed = false;		//!<@brief çqòHå≈íËÉtÉâÉO

public:
	GVOShipRoute() = default;
	~GVOShipRoute() = default;

	//!@attention å≈íËÇ≥ÇÍÇΩçqòHÇ…ç¿ïWÇí«â¡ÇµÇƒÇÕÇ»ÇÁÇ»Ç¢ÅBÉçÉWÉbÉNÉGÉâÅ[Ç»ÇÃÇ≈DebugéûÇÃÇ›ÉGÉâÅ[Ç∆ÇµÇƒÇ¢ÇÈÅB
	void addRoutePoint( const GVONormalizedPoint & point );

	const Lines & getLines() const
	{
		return m_lines;
	}

	bool isFavorite() const
	{
		return m_favorite;
	}

	void setFavorite( bool favorite )
	{
		m_favorite = favorite;
	}

	bool isHilight() const
	{
		return m_hilight;
	}

	void setHilight( bool hilight )
	{
		m_hilight = hilight;
	}

	//!@brief srcRouteÇÃï€éùÇ∑ÇÈçqòHÇëSÇƒëOÇ…òAåãÇ∑ÇÈÅB
	void jointPreviousLinesWithRoute( const GVOShipRoute & srcRoute );

	bool isEmptyRoute() const
	{
		if ( m_lines.empty() ) {
			return true;
		}

		// ÇPÇ¬Ç≈Ç‡ì_Çï€éùÇµÇƒÇ¢ÇÍÇŒãÛçqòHÇ≈ÇÕÇ»Ç¢Ç∆îªífÇ∑ÇÈ
		for ( auto line : m_lines ) {
			if ( !line.empty() ) {
				return false;
			}
		}
		return true;
	}

	bool isFixed() const
	{
		return m_fixed;
	}

	void setFix( bool isFixed)
	{
		m_fixed = isFixed;
	}

	double length() const
	{
		return m_length;
	}

	void addLine( Line && line )
	{
		m_lines.push_back( line );
	}
private:
};

typedef std::shared_ptr<GVOShipRoute> GVOShipRoutePtr;
typedef std::weak_ptr<GVOShipRoute> GVOShipRouteWeakPtr;
