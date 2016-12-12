#include "stdafx.h"
#include "GVOShipRouteList.h"
#include <fstream>

namespace {
	struct FileHeaderV1 {
		enum {
			k_FileVersion = 1,
		};
		const uint32_t version = k_FileVersion;
		uint32_t favoritsCount = 0;
	};

	typedef FileHeaderV1 FileHeader;
}


std::ostream & operator <<(std::ostream & os, const GVOShipRouteList & shipRouteList)
{
	_ASSERT( os.good() );
	if ( !os.good() ) {
		throw std::runtime_error("output stream error.");
	}

	FileHeader fileHeader;

	const auto headPos = os.tellp();
	os.seekp( sizeof(fileHeader), std::ios::cur );

	for ( const auto & shipRoute : shipRouteList.m_shipRouteList ) {
		if ( shipRoute->isFavorite() ) {
			++fileHeader.favoritsCount;
			os << *shipRoute;
		}
	}

	const auto tailPos = os.tellp();

	os.seekp( headPos, std::ios::beg );
	os.write( reinterpret_cast<const char *>(&fileHeader), sizeof(fileHeader) );
	os.seekp( tailPos, std::ios::beg );

	_ASSERT( os.good() );
	return os;
}


std::istream & operator >>(std::istream & is, GVOShipRouteList & shipRouteList)
{
	_ASSERT( is.good() );
	if ( !is.good() ) {
		throw std::runtime_error( "input stream error." );
	}

	FileHeader fileHeader;
	is.read( reinterpret_cast<char *>(&fileHeader), sizeof(fileHeader) );

	if ( fileHeader.version != FileHeader::k_FileVersion ) {
		throw std::runtime_error( "unknown file version." );
	}

	GVOShipRouteList::RouteList workRouteList;

	for ( uint32_t i = 0; i < fileHeader.favoritsCount; ++i ) {
		GVOShipRoutePtr shipRoute( new GVOShipRoute() );
		is >> *shipRoute;

		workRouteList.push_back( std::move( shipRoute ) );
	}

	shipRouteList.m_shipRouteList.swap( workRouteList );

	_ASSERT( is.good() );
	return is;
}


void GVOShipRouteList::closeRoute()
{
	if ( !m_shipRouteList.empty() ) {
		m_shipRouteList.back()->setFix( true );
	}
}


void GVOShipRouteList::addRoutePoint( const GVONormalizedPoint point )
{
	if ( m_shipRouteList.empty() || m_shipRouteList.back()->isFixed() ) {
		addRoute();
	}
	m_shipRouteList.back()->addRoutePoint( point );
	if ( m_observer ) {
		m_observer->onShipRouteListUpdateRoute( m_shipRouteList.back() );
	}
}


void GVOShipRouteList::removeShipRoute( GVOShipRoutePtr shipRoute )
{
	_ASSERT( shipRoute != nullptr );
	auto it = std::find( m_shipRouteList.begin(), m_shipRouteList.end(), shipRoute );
	if ( it == m_shipRouteList.end() ) {
		return;
	}
	GVOShipRoutePtr removeTarget = shipRoute;
	m_shipRouteList.erase( it );
	if ( m_observer ) {
		m_observer->onShipRouteListRemoveItem( removeTarget );
	}
}


void GVOShipRouteList::clearAllItems()
{
	m_shipRouteList.clear();
	if ( m_observer ) {
		m_observer->onShipRouteListRemoveAllItems();
	}
}


void GVOShipRouteList::addRoute()
{
	// 追加と通知
	m_shipRouteList.push_back( GVOShipRoutePtr( new GVOShipRoute() ) );
	if ( m_observer ) {
		m_observer->onShipRouteListAddRoute( m_shipRouteList.back() );
	}

	// 溢れた分を削除
	// とりあえずタココードで愚直に書いておく。
	int favorits = 0;
	for ( auto route : m_shipRouteList ) {
		if ( route->isFavorite() ) {
			++favorits;
		}
	}
	const int overCount = m_shipRouteList.size() - (m_maxRouteCountWithoutFavorits + favorits);
	if ( 0 < overCount ) {
		int removeCount = 0;
		auto it = m_shipRouteList.begin();
		while ( it != m_shipRouteList.end() && removeCount < overCount) {
			if ( (*it)->isFavorite() ) {
				++it;
				continue;
			}
			auto itNext = std::next( it, 1 );
			m_shipRouteList.erase( it );
			++removeCount;
			it = itNext;
		}
	}
}


void GVOShipRouteList::joinPreviousRouteAtReverseIndex( int reverseIndex )
{
	_ASSERT( 0 <= reverseIndex );
	_ASSERT( reverseIndex < (int)m_shipRouteList.size() );

	RouteList::iterator itBase;
	itBase = m_shipRouteList.begin();
	std::advance( itBase, indexFromReverseIndex( reverseIndex ) );
	RouteList::iterator itPrev = std::prev( itBase );
	_ASSERT( itPrev != m_shipRouteList.end() );
	GVOShipRoutePtr baseRoute = *itBase;
	GVOShipRoutePtr prevRoute = *itPrev;

	m_shipRouteList.erase( itPrev );

	// removeした時に通知ハンドラで属性が変更されてしまうので、
	// この時点で強調属性を保存しておく。
	bool isHilight = prevRoute->isHilight() | baseRoute->isHilight();
	if ( m_observer ) {
		m_observer->onShipRouteListRemoveItem( prevRoute );
	}

	baseRoute->jointPreviousLinesWithRoute( *prevRoute );
	baseRoute->setHilight( isHilight );

	// 通知
	if ( m_observer ) {
		m_observer->onShipRouteListUpdateRoute( baseRoute );
	}
}
