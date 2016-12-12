#pragma once
#include "GVONoncopyable.h"
#include "GVOVector.h"
#include "GVOImage.h"
#include "GVOShipRoute.h"

class GVOConfig;
class GVOWorldMap;
class GVOTexture;
class GVOShipRouteList;

class GVORenderer :private GVONoncopyable {
private:
	const GVOWorldMap * m_worldMap;	//!<@brief 世界地図
	GVOTexture * m_worldMapTexture;	//!<@brief 世界地図テクスチャー

	HDC m_hdcPrimary;
	HGLRC m_hglrc;

	SIZE m_viewSize;
	double m_viewScale;

	POINT m_focusPointInWorldCoord;		//!<@brief 画面中央に位置するピクセルの世界座標
	POINT m_shipPointInWorld;			//!<@brief 自船の位置
	bool m_shipVectorLineEnabled;		//!<@brief 針路描画フラグ
	bool m_speedMeterEnabled;			//!<@brief 速度計描画フラグ
	bool m_traceShipEnabled;			//!<@brief 自船位置追従フラグ

public:
	GVORenderer() :
		m_hdcPrimary(),
		m_viewSize(),
		m_viewScale(1.0),
		m_focusPointInWorldCoord(),
		m_shipPointInWorld(),
		m_shipVectorLineEnabled( true ),
		m_speedMeterEnabled( true ),
		m_traceShipEnabled( true )
	{
	}
	~GVORenderer()
	{
	}

	void setup( const GVOConfig * config, HDC hdcPrimary, const GVOWorldMap * worldMap );
	void teardown();

	void setViewSize( const SIZE& viewSize );

	bool zoomIn();
	bool zoomOut();
	void resetViewScale();

	inline double viewScale() const
	{
		return m_viewScale;
	}

	//!@note ドラッグ処理用
	void offsetFocusInViewCoord( const POINT& offset );

	//!@brief 自船の状態を設定する
	void setShipPositionInWorld( const POINT& shipPositionInWorld );

	void enableTraceShip( bool enabled )
	{
		m_traceShipEnabled = enabled;
	}

	void setVisibleShipRoute( bool visible )
	{
		m_shipVectorLineEnabled = visible;
	}

	void render( const GVOVector& shipVector, double shipVelocity, GVOTexture * shipTexture, const GVOShipRouteList * shipRouteList );

	void enableSpeedMeter( bool enabled )
	{
		m_speedMeterEnabled = enabled;
	}

	GVOTexture * createTextureFromImage( const GVOImage & image );
private:
	//!@brief 設定情報で初期化する
	void setConfig( const GVOConfig * config );
	void setupGL();
	void setWorldMap( const GVOWorldMap * worldMap );
	SIZE scaledMapSize() const;
	POINT mapOriginInView() const;
	POINT drawOffsetFromWorldCoord( const POINT&worldCoord ) const;
	void renderMap( const GVOVector& shipVector, GVOTexture * shipTexture, const GVOShipRouteList * shipRouteList );
	void renderShipRouteList(int width, int height, const GVOShipRouteList * shipRouteList );
	void renderSpeedMeter( double shipVelocity );
	void renderLines( const GVOShipRoutePtr shipRoute, float mapWidth, float mapHeight);
	void renderTexture( GVOTexture & texture, float w, float h );
	void renderTexture( GVOTexture & texture, float x, float y, float w, float h );
	inline POINT viewCenterPoint() const
	{
		POINT p = {
			m_viewSize.cx / 2,
			m_viewSize.cy / 2
		};
		return p;
	}
};
