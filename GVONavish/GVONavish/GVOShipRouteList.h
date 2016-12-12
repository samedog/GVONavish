#pragma once
#include <iostream>
#include <list>
#include "GVONavish.h"
#include "GVOShipRoute.h"
#include "GVONormalizedPoint.h"

class GVOShipRouteList;
class IGVOShipRouteListObserver;

//!@brief 航路リスト管理クラス
class GVOShipRouteList {
	//!@note バイナリ保存用
	friend std::ostream & operator <<(std::ostream & os, const GVOShipRouteList & shipRouteList);
	friend std::istream & operator >>(std::istream & is, GVOShipRouteList & shipRouteList);

private:
	typedef std::list<GVOShipRoutePtr> RouteList;

private:
	RouteList m_shipRouteList;
	IGVOShipRouteListObserver * m_observer = nullptr;
	size_t m_maxRouteCountWithoutFavorits = 30;	//!<@brief お気に入りを除外した航路保存数

public:
	GVOShipRouteList() = default;
	~GVOShipRouteList() = default;

	void setObserver( IGVOShipRouteListObserver * observer )
	{
		m_observer = observer;
	}

	void closeRoute();

	void addRoutePoint(const GVONormalizedPoint point);

	const RouteList & getList() const
	{
		return m_shipRouteList;
	}

	GVOShipRoutePtr getRouteAtReverseIndex( int reverseIndex )
	{
		if ( m_shipRouteList.size() <= (size_t)reverseIndex ) {
			return nullptr;
		}
		RouteList::iterator it;
		it = m_shipRouteList.begin();
		std::advance( it, indexFromReverseIndex( reverseIndex ) );
		_ASSERT( *it != nullptr );
		return *it;
	}

	int reverseIndexFromShipRoute( GVOShipRoutePtr shipRoute ) const
	{
		auto it = std::find( m_shipRouteList.crbegin(), m_shipRouteList.crend(), shipRoute );
		if ( it == m_shipRouteList.crend() ) {
			return -1;
		}
		const int reverseIndex = std::distance( m_shipRouteList.crbegin(), it );
		return reverseIndex;
	}

	void removeShipRoute( GVOShipRoutePtr shipRoute );

	void clearAllItems();

	void joinPreviousRouteAtReverseIndex( int reverseIndex );
private:
	int indexFromReverseIndex( int reverseIndex ) const
	{
		return m_shipRouteList.size() - reverseIndex - 1;
	}
	void addRoute();
};


//!@brief 航路管理リストを監視するインターフェース
class IGVOShipRouteListObserver {
public:
	IGVOShipRouteListObserver() = default;
	virtual ~IGVOShipRouteListObserver() = default;

	virtual void onShipRouteListAddRoute( GVOShipRoutePtr shipRoute ) = 0;
	virtual void onShipRouteListUpdateRoute( GVOShipRoutePtr shipRoute ) = 0;
	virtual void onShipRouteListRemoveItem( GVOShipRoutePtr shipRoute ) = 0;
	virtual void onShipRouteListRemoveAllItems() = 0;
};
