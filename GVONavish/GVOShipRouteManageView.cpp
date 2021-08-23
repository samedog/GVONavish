#include "stdafx.h"
#include "GVOShipRouteManageView.h"
#include "GVONavish.h"
#include "Resource.h"
#include "GVOShipRouteList.h"



namespace {
	static const wchar_t * const k_windowClassName = L"{8DFEDF35-DD1F-4907-8BC1-12C0CD7080B5}";

	inline std::wstring s_makePointString( const GVONormalizedPoint&point )
	{
		std::wstring str;

		str = std::to_wstring( static_cast<int>(::round( point.x()* k_worldWidth )) )
			+ L","
			+ std::to_wstring( static_cast<int>(::round( point.y()* k_worldHeight )) );

		return std::move( str );
	}
}


GVOShipRouteManageView::~GVOShipRouteManageView()
{
	teardown();
}


bool GVOShipRouteManageView::setup( GVOShipRouteList & shipRouteList )
{
	m_routeList = &shipRouteList;
	m_hwnd = ::CreateDialogParam( g_hinst, MAKEINTRESOURCE( IDD_SHIPROUTEMANAGEVIEW ),
		g_hwndMain, dlgProcThunk, reinterpret_cast<LPARAM>(this));
	if ( !m_hwnd ) {
		m_routeList = NULL;
		return false;
	}
	m_routeList->setObserver( this );
	return true;
}


void GVOShipRouteManageView::teardown()
{
	if ( m_hwnd ) {
		::DestroyWindow( m_hwnd );
		m_hwnd = NULL;
	}
}


void GVOShipRouteManageView::onShipRouteListAddRoute( GVOShipRoutePtr shipRoute )
{
	updateVisibleListItemCount();
	if ( GVOShipRoutePtr selectedRoute = m_selectedRoute.lock() ) {
		const int reverseIndex = m_routeList->reverseIndexFromShipRoute( selectedRoute );
		selectRow( m_selectionIndex, false );
		selectRow( reverseIndex, true );
	}
}


void GVOShipRouteManageView::onShipRouteListUpdateRoute( GVOShipRoutePtr shipRoute )
{
	int reverseIndex = m_routeList->reverseIndexFromShipRoute( shipRoute );
	if ( 0 <= reverseIndex ) {
		if ( shipRoute->isHilight() ) {
			selectRow( m_selectionIndex, false );
			selectRow( reverseIndex, true );
		}
		ListView_RedrawItems( m_listViewCtrl, reverseIndex, reverseIndex );
	}
}


void GVOShipRouteManageView::onShipRouteListRemoveItem( GVOShipRoutePtr shipRoute )
{
	if ( m_selectedRoute.lock() == shipRoute ) {
		if ( 0 <= m_selectionIndex ) {
			selectRow( m_selectionIndex, false );
		}
	}
	updateVisibleListItemCount();
}


void GVOShipRouteManageView::onShipRouteListRemoveAllItems()
{
	ListView_DeleteAllItems( m_hwnd );
}


INT_PTR  CALLBACK GVOShipRouteManageView::dlgProcThunk( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	GVOShipRouteManageView * instance = reinterpret_cast<GVOShipRouteManageView *>(::GetWindowLong( hwnd, GWLP_USERDATA));
	if ( msg == WM_INITDIALOG ) {
		instance = reinterpret_cast<GVOShipRouteManageView *>(lp);
		instance->m_hwnd = hwnd;
		::SetWindowLongPtr( hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(instance) );
	}
	if ( !instance ) {
		return FALSE;
	}
	return instance->dlgProc( msg, wp, lp );
}


BOOL CALLBACK GVOShipRouteManageView::dlgProc( UINT msg, WPARAM wp, LPARAM lp )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		setupRouteList();
		{
			RECT rc = { 0 };
			rc = s_screenRectFromClientRect( g_hwndMain, rc );
			::SetWindowPos( m_hwnd, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
		}
		break;
	case WM_COMMAND:
		onCommand( HIWORD( wp ), LOWORD( wp ), (HWND)lp );
		break;
	case WM_NOTIFY:
		onNotify( reinterpret_cast<LPNMHDR>(lp) );
		break;
	case WM_SIZE:
	{
		RECT rcClient = s_clientRect( m_hwnd );
		RECT rcList = s_clientRectFromScreenRect( m_hwnd, s_windowRect( m_listViewCtrl ) );
		rcList.right = rcClient.right;
		rcList.bottom = rcClient.bottom;
		const int width = rcList.right - rcList.left;
		const int height = rcList.bottom - rcList.top;
		::SetWindowPos( m_listViewCtrl, NULL, 0, 0,
			rcList.right - rcList.left, rcList.bottom - rcList.top,
			SWP_NOMOVE | SWP_NOZORDER );
	}
		return FALSE;
	default:
		return FALSE;
	}
	return TRUE;
}


void GVOShipRouteManageView::onCommand( WORD eventCode, WORD cmdId, HANDLE ctrl )
{
	switch ( cmdId ) {
	case IDOK:
	case IDCANCEL:
		::ShowWindow( m_hwnd, SW_HIDE );
		break;
	case IDM_DESELECT_ROUTE:
		if ( GVOShipRoutePtr selectedRoute = m_selectedRoute.lock() ) {
			selectRow( m_selectionIndex, false );
		}
		break;
	case IDM_DELETE_SHIP_ROUTE:
		if ( GVOShipRoutePtr selectedRoute = m_selectedRoute.lock() ) {
			selectRow( m_selectionIndex, false );
			m_routeList->removeShipRoute( selectedRoute );
		}
		break;
	case IDM_JOINT_SHIP_ROUTE:
		if ( GVOShipRoutePtr selectedRoute = m_selectedRoute.lock() ) {
			m_routeList->joinPreviousRouteAtReverseIndex( m_selectionIndex );
		}
		break;
	case IDM_TOGGLE_FAVORITE:
		if ( GVOShipRoutePtr selectedRoute = m_selectedRoute.lock() ) {
			selectedRoute->setFavorite( !selectedRoute->isFavorite() );
			ListView_RedrawItems( m_listViewCtrl, m_selectionIndex, m_selectionIndex );
		}
		break;
	case IDM_JOINT_LATEST_ROUTE:
		if ( 1 < m_routeList->getList().size() ) {
			m_routeList->joinPreviousRouteAtReverseIndex( 0 );
		}
		break;
	default:
		break;
	}
}


void GVOShipRouteManageView::onNotify( LPNMHDR nmh )
{
	switch ( nmh->idFrom ) {
	case IDC_SHIPROUTELIST:
		switch ( nmh->code ) {
		case LVN_GETDISPINFO:
		{
			LV_DISPINFO * dispInfo = reinterpret_cast<LV_DISPINFO *>(nmh);
			LVITEM & item = dispInfo->item;
			dispInfo->hdr.idFrom;
			GVOShipRoutePtr route = m_routeList->getRouteAtReverseIndex( item.iItem );
			// is it sometimes not enough time for deletion processing and drawing processing_
			if ( !route ) {
				updateVisibleListItemCount();
				break;
			}
			if ( item.mask & LVIF_TEXT ) {

				std::wstring str;

				switch ( item.iSubItem ) {
				case k_ColumnIndex_StartPoint:
					if ( route->getLines().empty() ) {
						str = L"-";
					}
					else {
						if ( !route->getLines().front().empty() ) {
							str = s_makePointString( route->getLines().front().front() );
						}
					}
					break;
				case k_ColumnIndex_EndPoint:
					if ( route->getLines().empty() ) {
						str = L"-";
					}
					else {
						if ( !route->getLines().back().empty() ) {
							str = s_makePointString( route->getLines().back().back() );
						}
					}
					break;
				case k_ColumnIndex_Length:
					if ( route->getLines().empty() ) {
						str = L"-";
					}
					else {
						//float length = route->length() / 10.0f;
						//std::vector<wchar_t> buf( 4096 );
						//::swprintf( &buf[0], buf.size(), L"%.1f km", length );
						//str = &buf[0];
						str = std::to_wstring( static_cast<int>(::round( route->length() )) );
					}
					break;
				default:
					break;
				}
				::lstrcpy( item.pszText, str.c_str() );
			}
			if ( item.mask & LVIF_IMAGE ) {
				if ( route->isFavorite() ) {
					item.iImage = k_IconIndex_Star;
				}
				else {
					item.iImage = k_IconIndex_Blank;
				}
			}
		}
			break;
		case NM_RCLICK:
			if ( GVOShipRoutePtr selectedRoute = m_selectedRoute.lock() ) {
				POINT point = { 0 };
				::GetCursorPos( &point );
				HMENU menu = ::LoadMenu( g_hinst, MAKEINTRESOURCE( IDR_SHIPROUTEMANAGEPOPUPMENU ) );
				menu = ::GetSubMenu( menu, 0 );

				const size_t count = m_routeList->getList().size();
				if ( count == (m_selectionIndex + 1) ) {
					::EnableMenuItem( menu, IDM_JOINT_SHIP_ROUTE, MF_BYCOMMAND | MF_DISABLED );
				}
				if ( selectedRoute->isFavorite() ) {
					::CheckMenuItem( menu, IDM_TOGGLE_FAVORITE, MF_BYCOMMAND | MF_CHECKED );
				}
				::TrackPopupMenu( menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NOANIMATION,
					point.x, point.y, 0, m_hwnd, NULL );
			}
			break;
		case LVN_ITEMCHANGED:
		{
			LPNMLISTVIEW nmlv = reinterpret_cast<LPNMLISTVIEW>(nmh);
			if ( ((nmlv->uOldState & LVIS_SELECTED) == 0) && (nmlv->uNewState & LVIS_SELECTED) ) {
				m_selectionIndex = nmlv->iItem;
				GVOShipRoutePtr selectedRoute = m_routeList->getRouteAtReverseIndex( m_selectionIndex );
				selectedRoute->setHilight( true );
				m_selectedRoute = selectedRoute;
				::InvalidateRect( g_hwndMain, NULL, FALSE );
			}
			else if ( (nmlv->uOldState & LVIS_SELECTED) && ((nmlv->uNewState & LVIS_SELECTED) == 0) ) {
				if ( GVOShipRoutePtr selectedRoute = m_selectedRoute.lock() ) {
					selectedRoute->setHilight( false );
					m_selectedRoute.reset();
				}
				m_selectionIndex = -1;
				::InvalidateRect( g_hwndMain, NULL, FALSE );
			}
		}
			break;
		//case LVN_ODSTATECHANGED:
		//	break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}


void GVOShipRouteManageView::setupRouteList()
{
	m_listViewCtrl = ::GetDlgItem( m_hwnd, IDC_SHIPROUTELIST );

	DWORD exStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( m_listViewCtrl, exStyle );

	HIMAGELIST imageList;
	imageList = ::ImageList_Create( 16, 16, ILC_COLOR32 | ILC_MASK, 2, 0 );
	ImageList_AddIcon( imageList, ::LoadIcon( g_hinst, MAKEINTRESOURCE( IDI_BLANK ) ) );
	ImageList_AddIcon( imageList, ::LoadIcon( g_hinst, MAKEINTRESOURCE( IDI_STAR ) ) );
	ListView_SetImageList( m_listViewCtrl, imageList, LVSIL_SMALL );

	int index = 0;
	LV_COLUMN column = { 0 };
	column.mask = LVCF_TEXT;

	column.pszText = L"Departure";
	ListView_InsertColumn( m_listViewCtrl, index, &column );
	++index;

	column.pszText = L"Arrival";
	ListView_InsertColumn( m_listViewCtrl, index, &column );
	++index;

	column.pszText = L"Distance";
	ListView_InsertColumn( m_listViewCtrl, index, &column );
	++index;

	updateVisibleListItemCount();

	for ( int i = 0; i < index; i++ ) {
		ListView_SetColumnWidth( m_listViewCtrl, i, LVSCW_AUTOSIZE_USEHEADER );
	}
}


void GVOShipRouteManageView::updateVisibleListItemCount()
{
	ListView_SetItemCountEx( m_listViewCtrl, m_routeList->getList().size(), LVSICF_NOSCROLL );
}


void GVOShipRouteManageView::selectRow( int index, bool isSelection )
{
	if ( index < 0 ) {
		return;
	}
	ListView_SetItemState( m_listViewCtrl, index, isSelection ? LVIS_SELECTED : 0, LVIS_SELECTED );
}
