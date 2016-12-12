#pragma once
#include "GVONoncopyable.h"
#include "GVOShipRouteList.h"



//!@brief 航路管理ビュー
class GVOShipRouteManageView : private GVONoncopyable, public IGVOShipRouteListObserver {
private:
	enum ColumnIndex {
		k_ColumnIndex_StartPoint,
		k_ColumnIndex_EndPoint,
		k_ColumnIndex_Length,
	};
	enum IconIndex {
		k_IconIndex_Blank,
		k_IconIndex_Star,
	};
	HWND m_hwnd = nullptr;
	GVOShipRouteList * m_routeList = nullptr;

	HWND m_listViewCtrl = nullptr;
	int m_selectionIndex = -1;	//!<@brief 仮想リストビューなので選択行を自前管理
	GVOShipRouteWeakPtr m_selectedRoute;

	size_t m_visibleCount = 50;

public:
	GVOShipRouteManageView() = default;
	~GVOShipRouteManageView();

	bool setup( GVOShipRouteList & shipRouteList );
	void teardown();

	void activate()
	{
		::ShowWindow( m_hwnd, SW_SHOWNORMAL );
		::SetForegroundWindow( m_hwnd );
	}

	virtual void onShipRouteListAddRoute( GVOShipRoutePtr shipRoute ) override;
	virtual void onShipRouteListUpdateRoute( GVOShipRoutePtr shipRoute ) override;
	virtual void onShipRouteListRemoveItem( GVOShipRoutePtr shipRoute ) override;
	virtual void onShipRouteListRemoveAllItems() override;
private:
	static BOOL CALLBACK dlgProcThunk( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );
	BOOL CALLBACK dlgProc( UINT msg, WPARAM wp, LPARAM lp );
	void onCommand(WORD eventCode, WORD cmdId, HANDLE ctrl);
	void onNotify( LPNMHDR nmh );
	void setupRouteList();
	void updateVisibleListItemCount();
	void selectRow( int index, bool isSelection );
};

