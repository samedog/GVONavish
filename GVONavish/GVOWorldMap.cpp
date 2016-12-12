#include "stdafx.h"
#include <vector>

#include "GVONavish.h"
#include "GVOWorldMap.h"




bool GVOWorldMap::loadFromFile( const std::wstring& fileNmee )
{
	GVOImage workImage;
	std::wstring filePath;

	filePath = g_makeFullPath( fileNmee );
	if ( !workImage.loadFromFile( filePath.c_str() ) ) {
		return false;
	}
	m_mapImage.copy( workImage );
	workImage.reset();
	return true;
}


POINT GVOWorldMap::imageCoordFromWorldCoord( const POINT& worldCoord ) const
{
	const double xNormPos = worldCoord.x / (double)k_worldWidth;
	const double yNormPos = worldCoord.y / (double)k_worldHeight;
	const POINT worldPosInImage = {
		LONG( m_mapImage.width() * xNormPos ),
		LONG( m_mapImage.height() * yNormPos )
	};
	return worldPosInImage;
}
