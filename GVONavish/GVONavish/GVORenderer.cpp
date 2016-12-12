#include "stdafx.h"
#include "GVORenderer.h"
#include "GVOWorldMap.h"
#include "GVOConfig.h"
#include "GVOTexture.h"
#include "GVOShipRouteList.h"
#include "GVOShipRoute.h"



namespace {
	const double k_scaleStep = 0.125;	// 12.5%
	const double k_minScale = 0.125;	// 12.5%
	const double k_maxScale = 4.00;		// 400%

	// GoogleêÊê∂ûHÇ≠ÅuínãÖÇÃäOé¸ÇÕ40,075kmÅvÅu1ÉmÉbÉgÇÕ1.85200kmÅv
	// 1ê¢äEç¿ïWÇÕ40,075km/16384points
	// é¿éûä‘1ïbÇ≈ÉQÅ[ÉÄì‡0.4éûä‘
	//
	// ínãÖäOé¸Çê‘ìπîºåaÇ©ÇÁéZèoÇ∑ÇÈÅB
	// ê‘ìπîºåaÇÕ6378.137Ç»ÇÃÇ≈äOé¸ÇÕ2*M_PI_*6378.137
	//
	// 9é¸îNãLîOÉCÉxÉìÉgÇÃÅu0.1kmÅvî≠åæãyÇ—É~ÉbÉVÉáÉìÇPÇ≈ÇÃçqçsãóó£èWåvílÇ©ÇÁ1ê¢äEç¿ïW=0.1kmÇ∆îªñæÅB
	// Ç∆ÇÕÇ¢Ç¶ï¢Ç≥ÇÍÇÈâ¬î\ê´Ç‡çÇÇ¢ÇÃÇ≈ï€óØÇ∆ÇµÇƒÇ®Ç≠ÅB
	inline double s_velocityByKnot( const double velocity )
	{
		static const double k_knotFactor = (2 * M_PI * 6378.137) / 16384.0 / 0.4 / 1.852;
		return velocity * k_knotFactor;
	}
}


void GVORenderer::setup( const GVOConfig * config, HDC hdcPrimary, const GVOWorldMap * worldMap )
{
	m_hdcPrimary = hdcPrimary;
	setConfig( config );
	setupGL();
	setWorldMap( worldMap );
}


void GVORenderer::teardown()
{
	delete m_worldMapTexture;
	m_worldMapTexture = NULL;

	::wglMakeCurrent( NULL, NULL );
	::wglDeleteContext( m_hglrc );
	m_hglrc = NULL;
	m_hdcPrimary = NULL;
}


void GVORenderer::setConfig( const GVOConfig * config )
{
	m_focusPointInWorldCoord = config->m_initialSurveyCoord;
	m_shipPointInWorld.x = -1;
	m_shipPointInWorld.y = -1;
	m_shipVectorLineEnabled = config->m_shipVectorLineEnabled;
	m_speedMeterEnabled = config->m_speedMeterEnabled;
	m_traceShipEnabled = config->m_traceShipPositionEnabled;
}


void GVORenderer::setupGL()
{
	PIXELFORMATDESCRIPTOR pfd = { sizeof(pfd) };
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	::SetPixelFormat( m_hdcPrimary, ::ChoosePixelFormat( m_hdcPrimary, &pfd ), &pfd );
	m_hglrc = ::wglCreateContext( m_hdcPrimary );
	::wglMakeCurrent( m_hdcPrimary, m_hglrc );
	::glDisable( GL_DEPTH_TEST );
	::glDisable( GL_LIGHTING );
	::glEnable( GL_CULL_FACE );
	::glCullFace( GL_BACK );
	::wglMakeCurrent( NULL, NULL );
}


void GVORenderer::setWorldMap( const GVOWorldMap * worldMap )
{
	::wglMakeCurrent( m_hdcPrimary, m_hglrc );
	m_worldMap = worldMap;
	m_worldMapTexture = new GVOTexture();
	m_worldMapTexture->setImage( worldMap->image() );
	::glFlush();
	::wglMakeCurrent( NULL, NULL );
}


void GVORenderer::setViewSize( const SIZE& viewSize )
{
	m_viewSize = viewSize;
	::wglMakeCurrent( m_hdcPrimary, m_hglrc );
	::glMatrixMode( GL_PROJECTION );
	::glLoadIdentity();
	::glViewport( 0, 0, m_viewSize.cx, m_viewSize.cy );
	::gluOrtho2D( 0, m_viewSize.cx, m_viewSize.cy, 0 );
	::glFlush();
	::wglMakeCurrent( NULL, NULL );
}


SIZE GVORenderer::scaledMapSize() const
{
	SIZE size = {
		LONG( m_worldMap->image().width() * m_viewScale ),
		LONG( m_worldMap->image().height() * m_viewScale )
	};
	return size;
}


POINT GVORenderer::mapOriginInView() const
{
	const POINT viewCenter = viewCenterPoint();
	const SIZE mapSize = scaledMapSize();
	const POINT worldPosInView = drawOffsetFromWorldCoord( m_focusPointInWorldCoord );

	POINT mapTopLeft = {
		viewCenter.x - worldPosInView.x,
		viewCenter.y - worldPosInView.y
	};
	if ( m_viewSize.cx < mapSize.cx ) {
		while ( 0 < mapTopLeft.x ) {
			mapTopLeft.x -= mapSize.cx;
		}
	}

	return mapTopLeft;
}


void GVORenderer::offsetFocusInViewCoord( const POINT& offset )
{
	const double dx = ((double)offset.x / m_viewScale) / m_worldMap->image().width();
	const double dy = ((double)offset.y / m_viewScale) / m_worldMap->image().height();

	LONG x = m_focusPointInWorldCoord.x + LONG( dx * k_worldWidth );
	LONG y = m_focusPointInWorldCoord.y + LONG( dy * k_worldHeight );
	y = std::max<LONG>( 0, std::min<LONG>( y, k_worldHeight ) );
	while ( x < 0 ) {
		x += k_worldWidth;
	}
	while ( k_worldWidth < x ) {
		x -= k_worldWidth;
	}

	m_focusPointInWorldCoord.x = x;
	m_focusPointInWorldCoord.y = y;
}


bool GVORenderer::zoomIn()
{
	double scale = m_viewScale;
	double step = k_scaleStep;

	scale = m_viewScale + step;
	if ( k_maxScale < scale ) {
		scale = k_maxScale;
	}
	if ( m_viewScale != scale ) {
		m_viewScale = scale;
		return true;
	}
	return false;
}


bool GVORenderer::zoomOut()
{
	double scale = m_viewScale;
	double step = k_scaleStep;

	scale = m_viewScale - step;
	if ( scale < k_minScale ) {
		scale = k_minScale;
	}
	if ( m_viewScale != scale ) {
		m_viewScale = scale;
		return true;
	}
	return false;
}


void GVORenderer::resetViewScale()
{
	m_viewScale = 1.0;
}


POINT GVORenderer::drawOffsetFromWorldCoord( const POINT&worldCoord ) const
{
	const POINT worldPosInImage = m_worldMap->imageCoordFromWorldCoord( worldCoord );
	const POINT drawOffset = {
		LONG( worldPosInImage.x * m_viewScale ),
		LONG( worldPosInImage.y * m_viewScale )
	};
	return drawOffset;
}


void GVORenderer::setShipPositionInWorld( const POINT& shipPositionInWorld )
{
	if ( m_traceShipEnabled ) {
		m_focusPointInWorldCoord = shipPositionInWorld;
	}
	m_shipPointInWorld = shipPositionInWorld;
}


void GVORenderer::render( const GVOVector& shipVector, double shipVelocity, GVOTexture * shipTexture, const GVOShipRouteList * shipRouteList )
{
	::wglMakeCurrent( m_hdcPrimary, m_hglrc );
	::glClearColor( 0.2f, 0.2f, 0.3f, 0.0f );
	::glClear( GL_COLOR_BUFFER_BIT );
	::glDisable( GL_BLEND );

	renderMap( shipVector, shipTexture, shipRouteList );

	if ( m_speedMeterEnabled ) {
		renderSpeedMeter( shipVelocity );
	}

	::glFlush();
	::SwapBuffers( m_hdcPrimary );
	::wglMakeCurrent( NULL, NULL );
}


void GVORenderer::renderMap( const GVOVector& shipVector, GVOTexture * shipTexture, const GVOShipRouteList * shipRouteList )
{
	_ASSERT( m_worldMapTexture != NULL );

	const SIZE mapSize = scaledMapSize();

	const POINT mapTopLeft = mapOriginInView();
	int xDrawOrigin, yDrawOrigin;
	xDrawOrigin = mapTopLeft.x;
	yDrawOrigin = mapTopLeft.y;
	if ( 0 < xDrawOrigin ) {
		xDrawOrigin = (xDrawOrigin % mapSize.cx) - mapSize.cx;
	}
	const int xInitial = xDrawOrigin;	// ç∂í[ÇÃï`âÊäJénxç¿ïW
	int drawn = xInitial;

	// ê¢äEínê}Çâ°Ç…ï¿Ç◊Çƒï`âÊ
	// Åiï`âÊç≈ìKâªÇÕè»ó™Åj
	::glMatrixMode( GL_MODELVIEW );
	::glLoadIdentity();
	::glTranslatef( (float)xDrawOrigin, (float)yDrawOrigin, 0 );
	while ( drawn < m_viewSize.cx ) {
		// ínê}Ç1ñáï`âÊ
		renderTexture( *m_worldMapTexture, (float)mapSize.cx, (float)mapSize.cy );

		// çqòHÇ1ñáï™ï`âÊ
		renderShipRouteList( mapSize.cx, mapSize.cy, shipRouteList );

		xDrawOrigin += mapSize.cx;
		drawn += mapSize.cx;
		::glTranslatef( (float)mapSize.cx, 0.0f, 0.0f );
	}


	// ïsê≥Ç»é©ëDà íuÇ»ÇÁÇ±ÇÍà»ç~ÇÃï`âÊÇÕÇµÇ»Ç¢ÅB
	if ( m_shipPointInWorld.x < 0 || m_shipPointInWorld.y < 0 ) {
		return;
	}

	const POINT shipPointOffset = drawOffsetFromWorldCoord( m_shipPointInWorld );

	// êjòHó\ë™ê¸Çï`âÊ
	if ( shipVector.length() != 0.0 && m_shipVectorLineEnabled ) {
		const float lineWidth = max<float>( 1, float( 1 * m_viewScale ) );
		::glLineWidth( lineWidth );
		::glColor3f( 255, 0, 255 );

		const LONG k_lineLength = k_worldHeight;
		const POINT reachPointOffset = drawOffsetFromWorldCoord(
			shipVector.pointFromOriginWithLength( m_shipPointInWorld, k_lineLength )
			);

		// å©Ç¶ÇƒÇÈínê}âÊëúÇÃï™ÇæÇØï`âÊÇ∑ÇÈ
		drawn = xInitial;
		xDrawOrigin = xInitial;
		::glMatrixMode( GL_MODELVIEW );
		::glLoadIdentity();
		::glTranslatef( (float)xDrawOrigin, (float)yDrawOrigin, 0 );
		while ( drawn < m_viewSize.cx ) {
			::glBegin( GL_LINES );
			::glVertex2i( shipPointOffset.x, shipPointOffset.y );
			::glVertex2i( reachPointOffset.x, reachPointOffset.y );
			::glEnd();

			xDrawOrigin += mapSize.cx;
			drawn += mapSize.cx;
			::glTranslatef( (float)mapSize.cx, 0.0f, 0.0f );
		}
	}


	// é©ëDÇÃà íuÇï`âÊ
	if ( shipTexture ) {
		const float shipMarkSize = 16.0f;

		// å©Ç¶ÇƒÇÈínê}âÊëúÇÃï™ÇæÇØï`âÊÇ∑ÇÈ
		drawn = xInitial;
		xDrawOrigin = xInitial;
		::glMatrixMode( GL_MODELVIEW );
		::glLoadIdentity();
		::glLineWidth( 1.0f );
		::glTranslatef( (float)xDrawOrigin, (float)yDrawOrigin, 0 );
		while ( drawn < m_viewSize.cx ) {
			const float x = shipPointOffset.x - shipMarkSize / 2.0f;
			const float y = shipPointOffset.y - shipMarkSize / 2.0f;

			::glEnable( GL_BLEND );
			::glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			::glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			renderTexture( *shipTexture, x, y, shipMarkSize, shipMarkSize );
			::glDisable( GL_BLEND );

			xDrawOrigin += mapSize.cx;
			drawn += mapSize.cx;
			::glTranslatef( (float)mapSize.cx, 0.0f, 0.0f );
		}
	}
}


void GVORenderer::renderShipRouteList( int width, int height, const GVOShipRouteList * shipRouteList )
{
	_ASSERT( 0 <= width );
	_ASSERT( 0 <= height );
	_ASSERT(shipRouteList != NULL);

	// TODO: É^ÉRÉRÅ[Éh

	const float lineWidth = max<float>( 1, float( 1 * m_viewScale ) );
	const float hilightLineWidth = lineWidth * 1.5f;
	::glLineWidth( lineWidth );
	// ç≈êVçqòHà»äOÇîºìßñæÇ≈ï`âÊ
	if ( 1 < shipRouteList->getList().size() ) {
		::glEnable( GL_BLEND );
		::glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		::glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
	}

	// Ç®ãCÇ…ì¸ÇËÇ≈Ç‡ÉnÉCÉâÉCÉgÇ≈Ç‡Ç»Ç¢çqòHÇï`âÊ
	::glLineWidth( lineWidth );
	::glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
	for ( const GVOShipRoutePtr route : shipRouteList->getList() ) {
		if ( route->isFavorite() || route->isHilight() ) {
			continue;
		}
		// ç≈êVçqòHÇæÇØÇïsìßñæÇ≈ï`âÊ
		if ( shipRouteList->getList().back() == route ) {
			::glLineWidth( lineWidth );
			::glColor3f( 1.0f, 1.0f, 1.0f );
			::glDisable( GL_BLEND );
		}

		if ( route->isFavorite() ) {
			::glColor4f( 1.0f, 1.0f, 0.0f, 0.75f );
			if ( route->isHilight() ) {
				::glLineWidth( hilightLineWidth );
			}
		}
		renderLines( route, (float)width, (float)height );
	}

	// Ç®ãCÇ…ì¸ÇËçqòHÇï`âÊ
	::glColor4f( 1.0f, 1.0f, 0.0f, 0.75f );
	::glEnable( GL_BLEND );
	for ( const GVOShipRoutePtr route : shipRouteList->getList() ) {
		if ( !route->isFavorite() || route->isHilight() ) {
			continue;
		}

		// ç≈êVçqòHÇæÇØÇïsìßñæÇ≈ï`âÊ
		if ( shipRouteList->getList().back() == route ) {
			::glDisable( GL_BLEND );
		}

		if ( route->isHilight() ) {
			::glLineWidth( hilightLineWidth );
		}
		renderLines( route, (float)width, (float)height );
	}

	// ÉnÉCÉâÉCÉgçqòHÇï\é¶
	::glEnable( GL_BLEND );
	::glLineWidth( hilightLineWidth );
	for ( const GVOShipRoutePtr route : shipRouteList->getList() ) {
		if ( !route->isHilight() ) {
			continue;
		}

		if ( route->isFavorite() ) {
			::glColor4f( 1.0f, 1.0f, 0.0f, 0.75f );
		}
		else {
			::glColor4f( 0.5f, 1.0f, 1.0f, 0.75f );
		}

		// ç≈êVçqòHÇæÇØÇïsìßñæÇ≈ï`âÊ
		if ( shipRouteList->getList().back() == route ) {
			::glDisable( GL_BLEND );
		}

		renderLines( route, (float)width, (float)height );
	}

	::glDisable( GL_BLEND );
}


void GVORenderer::renderSpeedMeter( double shipVelocity )
{
	// THEéËî≤Ç´ï∂éöÉeÉNÉXÉ`ÉÉÅ[çÏê¨

	HDC hdcMem = ::CreateCompatibleDC( m_hdcPrimary );
	::SaveDC( hdcMem );

	const double velocity = s_velocityByKnot( shipVelocity );
	wchar_t buf[4096] = { 0 };
	swprintf( buf, _countof( buf ), L"%.2f kt", velocity );

	RECT rc = { 0 };
	::DrawText( hdcMem, buf, -1, &rc, DT_SINGLELINE | DT_RIGHT | DT_TOP | DT_CALCRECT );
	const int width = rc.right - rc.left;
	const int stride = width + (4 - width % 4) % 4;
	const int height = rc.bottom - rc.top;
	GVOImage workImage;
	workImage.createImage( stride, height );
	::SelectObject( hdcMem, workImage.bitmapHandle() );
	::DrawText( hdcMem, buf, -1, &rc, DT_SINGLELINE | DT_RIGHT | DT_TOP );
	::RestoreDC( hdcMem, -1 );
	::DeleteDC( hdcMem );

	GVOTexture workTexture;
	workTexture.setImage( workImage );

	::glMatrixMode( GL_MODELVIEW );
	::glLoadIdentity();
	renderTexture( workTexture, float( m_viewSize.cx - stride ), 0.0f, (float)stride, (float)height );
}


void GVORenderer::renderLines( const GVOShipRoutePtr shipRoute, float mapWidth, float mapHeight )
{
	for ( const GVOShipRoute::Line & line : shipRoute->getLines() ) {
		if ( line.size() < 2 ) {
			// 2ì_ñ¢ñûÇ≈ÇÕê¸Çà¯ÇØÇ»Ç¢
			continue;
		}
		::glBegin( GL_LINE_STRIP );
		for ( const GVONormalizedPoint & point : line ) {
			const float x = point.x() * mapWidth;
			const float y = point.y() * mapHeight;
			::glVertex2f( x, y );
		}
		::glEnd();
	}
}


void GVORenderer::renderTexture( GVOTexture & texture, float w, float h )
{
	renderTexture( texture, 0, 0, w, h );
}

void GVORenderer::renderTexture( GVOTexture & texture, float x, float y, float w, float h )
{
	texture.bind();

	::glBegin( GL_QUADS );

	::glTexCoord2d( 0, 0 );
	::glVertex2d( x, y );

	::glTexCoord2d( 0, 1 );
	::glVertex2d( x, y + h );

	::glTexCoord2d( 1, 1 );
	::glVertex2d( x + w, y + h );

	::glTexCoord2d( 1, 0 );
	::glVertex2d( x + w, y );

	::glEnd();

	texture.unbind();
}


GVOTexture * GVORenderer::createTextureFromImage( const GVOImage & image )
{
	::wglMakeCurrent( m_hdcPrimary, m_hglrc );
	std::auto_ptr<GVOTexture> texture( new GVOTexture() );
	texture->setImage( image );
	::glFlush();
	::wglMakeCurrent( NULL, NULL );

	return texture.release();
}
