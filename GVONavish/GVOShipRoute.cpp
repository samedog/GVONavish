#include "stdafx.h"
#include "GVOShipRoute.h"
#include "GVOVector.h"
#include <iostream>

namespace {
	struct ChunkHeader {
		enum  : uint32_t {
			k_Version1 = 1,
		};
		const uint32_t version = k_Version1;
		uint32_t lineCount = 0;
	};

	const float k_worldLoopThreshold = 0.5f;

	inline POINT s_denormalizedPoint( const GVONormalizedPoint & point )
	{
		POINT p = {
			static_cast<long>(::round( point.x() * k_worldWidth )),
			static_cast<long>(::round( point.y() * k_worldHeight )),
		};
		return p;
	}

	inline double s_calcLineLength( const GVOShipRoute::Line & line )
	{
		double length = 0.0;

		for ( auto it = line.begin(); it != line.end(); ++it ) {
			auto p1 = *it;
			if ( std::next(it) == line.end() ) {
				break;
			}
			auto p2 = *std::next( it );
			auto vector = GVOVector( s_denormalizedPoint( p1 ), s_denormalizedPoint( p2 ) );
			length += vector.length();
		}
		return length;
	}
}


std::ostream & operator << (std::ostream& os, GVOShipRoute& shipRoute)
{
	_ASSERT( os.good() );
	if ( !os.good() ) {
		throw std::runtime_error( "output stream error." );
	}

	ChunkHeader header;
	header.lineCount = shipRoute.getLines().size();
	os.write( reinterpret_cast<const char *>(&header), sizeof(header) );

	for ( const auto & line : shipRoute.getLines() ) {
		const size_t count = line.size();
		os.write( reinterpret_cast<const char *>(&count), sizeof(count) );
		if ( !line.empty() ) {
			os.write( reinterpret_cast<const char *>(&line[0]), sizeof(line[0]) * line.size() );
		}
	}

	_ASSERT( os.good() );
	return os;
}


std::istream & operator >> (std::istream& is, GVOShipRoute& shipRoute)
{
	_ASSERT( is.good() );
	if ( !is.good() ) {
		throw std::runtime_error( "input stream error." );
	}

	ChunkHeader header;
	is.read( reinterpret_cast<char *>(&header), sizeof(header) );

	if ( header.version != ChunkHeader::k_Version1 ) {
		throw std::runtime_error( "unknown file version." );
	}

	shipRoute.setFavorite( true );
	shipRoute.setFix( true );

	for ( size_t k = 0; k < header.lineCount; ++k ) {
		size_t pointCount = 0;
		is.read( reinterpret_cast<char *>(&pointCount), sizeof(pointCount) );
		if ( 0 < pointCount ) {
			GVOShipRoute::Line tmp( pointCount );
			is.read( reinterpret_cast<char *>(&tmp[0]), sizeof(tmp[0]) * tmp.size() );
			if ( !shipRoute.getLines().empty() && !tmp.empty() ) {
				auto p1 = shipRoute.getLines().back().back();
				auto p2 = tmp.front();
				shipRoute.m_length += GVOVector( s_denormalizedPoint( p1 ), s_denormalizedPoint( p2 ) ).length();
			}
			shipRoute.m_length += s_calcLineLength( tmp );
			shipRoute.addLine( std::move( tmp ) );
		}
	}

	_ASSERT( is.good() );
	return is;
}


void GVOShipRoute::addRoutePoint( const GVONormalizedPoint & point )
{
	_ASSERT( !isFixed() );

	if ( m_lines.empty() ) {
		m_lines.push_back( Line() );
	}

	Line & line = m_lines.back();
	if ( line.empty() ) {
		line.push_back( point );
		return;
	}

	GVOVector vector( s_denormalizedPoint( line.back() ), s_denormalizedPoint( point ) );
	m_length += vector.length();

	const GVONormalizedPoint & prevPoint = line.back();
	if ( prevPoint.isEqualValue( point ) ) {
		return;
	}

	// ê¢äEÇå◊Ç¢ÇæèÍçáÇÕê¸Çï™äÑÇ∑ÇÈ
	if ( prevPoint.x() < point.x() && (k_worldLoopThreshold <= (point.x() - prevPoint.x())) ) {
		// êºÇ…å¸Ç©Ç¡Çƒå◊Ç¢ÇæèÍçá
		const GVONormalizedPoint leftSideSubPoint( point.x() - 1.0f, point.y() );
		const GVONormalizedPoint rightSideSubPoint( prevPoint.x() + 1.0f, prevPoint.y() );

		line.push_back( leftSideSubPoint );
		m_lines.emplace( m_lines.end(), std::move( Line{ rightSideSubPoint, point } ) );
	}
	else if ( point.x() < prevPoint.x() && (k_worldLoopThreshold <= (prevPoint.x() - point.x())) ) {
		// ìåÇ…å¸Ç©Ç¡Çƒå◊Ç¢ÇæèÍçá
		const GVONormalizedPoint rightSideSubPoint( point.x() + 1.0f, point.y() );
		const GVONormalizedPoint leftSideSubPoint( prevPoint.x() - 1.0f, prevPoint.y() );

		line.push_back( rightSideSubPoint );
		m_lines.emplace( m_lines.end(), std::move( Line{ leftSideSubPoint, point } ) );
	}
	else {
		line.push_back( point );
	}
}


void GVOShipRoute::jointPreviousLinesWithRoute( const GVOShipRoute & srcRoute )
{
	// òAåãå≥ÇÃçqòHÇ™ê¸Çï€éùÇµÇƒÇ¢Ç»ÇØÇÍÇŒñ≥éãÇ∑ÇÈÅB
	if ( srcRoute.isEmptyRoute() ) {
		return;
	}
	// òAåãêÊÇÃçqòHÇ™ê¸Çï€éùÇµÇƒÇ¢Ç»ÇØÇÍÇŒñ≥éãÇ∑ÇÈÅB
	if ( isEmptyRoute() ) {
		m_lines = srcRoute.m_lines;
		return;
	}


	// ÇQÇ¬ÇÃçqòHÇÃí∑Ç≥Çë´Ç∑
	m_length += srcRoute.m_length;


	Lines tmp = srcRoute.m_lines;

	// ëOÇÃçqòHÇÃèIì_Ç∆åªç›ÇÃçqòHÇÃénì_Ç™òAåãâ¬î\Ç»ÇÁÉâÉCÉìÇåqÇ∞Ç»ÇØÇÍÇŒÇ»ÇÁÇ»Ç¢

	Line & prevLine = tmp.back();
	Line & nextLine = m_lines.front();

	// ÇQÇ¬ÇÃê¸Ç™ì_Çï€éùÇµÇƒÇ¢ÇÈÇ©ämîFÇµÇƒÇ®Ç≠
	if ( !prevLine.empty() && !nextLine.empty() ) {
		GVONormalizedPoint prevPoint = prevLine.back();
		GVONormalizedPoint nextPoint = nextLine.front();

		// çqòHä‘ÇÃí∑Ç≥Çë´Ç∑
		const double betweenLength = GVOVector( s_denormalizedPoint( prevPoint ), s_denormalizedPoint( nextPoint ) ).length();
		m_length += betweenLength;

		// ê¢äEÇå◊Ç¢Ç≈Ç¢Ç»Ç¢ÇÊÇ§Ç»ÇÁê¸Çê⁄ë±Ç∑ÇÈ
		if ( (std::max( prevPoint.x(), nextPoint.x() ) - std::min(prevPoint.x(), nextPoint.x())) < k_worldLoopThreshold ) {
			prevLine.insert( prevLine.end(), nextLine.begin(), nextLine.end() );
			m_lines.erase( m_lines.begin() );
		}
	}

	tmp.insert( tmp.end(), m_lines.begin(), m_lines.end() );
	m_lines.swap(tmp);

	// Ç«ÇøÇÁÇ©Ç™Ç®ãCÇ…ì¸ÇËçqòHÇ»ÇÁòAåãå„Ç‡Ç®ãCÇ…ì¸ÇËçqòHÇ∆å©Ç»Ç∑
	setFavorite( isFavorite() | srcRoute.isFavorite() );
}
