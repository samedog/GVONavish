#pragma once
#include <deque>
#include <ctime>
#include "GVONormalizedPoint.h"


//!@brief �q�H
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
	bool m_fixed = false;		//!<@brief �q�H�Œ�t���O

public:
	GVOShipRoute() = default;
	~GVOShipRoute() = default;

	//!@attention �Œ肳�ꂽ�q�H�ɍ��W��ǉ����Ă͂Ȃ�Ȃ��B���W�b�N�G���[�Ȃ̂�Debug���̂݃G���[�Ƃ��Ă���B
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

	//!@brief srcRoute�̕ێ�����q�H��S�đO�ɘA������B
	void jointPreviousLinesWithRoute( const GVOShipRoute & srcRoute );

	bool isEmptyRoute() const
	{
		if ( m_lines.empty() ) {
			return true;
		}

		// �P�ł��_��ێ����Ă���΋�q�H�ł͂Ȃ��Ɣ��f����
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
