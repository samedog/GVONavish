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
	const GVOWorldMap * m_worldMap;	//!<@brief ���E�n�}
	GVOTexture * m_worldMapTexture;	//!<@brief ���E�n�}�e�N�X�`���[

	HDC m_hdcPrimary;
	HGLRC m_hglrc;

	SIZE m_viewSize;
	double m_viewScale;

	POINT m_focusPointInWorldCoord;		//!<@brief ��ʒ����Ɉʒu����s�N�Z���̐��E���W
	POINT m_shipPointInWorld;			//!<@brief ���D�̈ʒu
	bool m_shipVectorLineEnabled;		//!<@brief �j�H�`��t���O
	bool m_speedMeterEnabled;			//!<@brief ���x�v�`��t���O
	bool m_traceShipEnabled;			//!<@brief ���D�ʒu�Ǐ]�t���O

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

	//!@note �h���b�O�����p
	void offsetFocusInViewCoord( const POINT& offset );

	//!@brief ���D�̏�Ԃ�ݒ肷��
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
	//!@brief �ݒ���ŏ���������
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
