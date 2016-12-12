#pragma once
#include "GVONoncopyable.h"
#include "GVOImage.h"
#include "GVOConfig.h"
#include "GVOVector.h"
#include "GVONormalizedPoint.h"




//!@brief ���E�n�}
//!@brief �\�����W�n�Ɛ��E���W�n�̕ϊ��Ȃǂ��d���B
class GVOWorldMap : private GVONoncopyable {
	friend class GVORenderer;
public:
private:
	GVOImage m_mapImage;

public:
	GVOWorldMap()
	{
	}

	virtual ~GVOWorldMap()
	{
	}

	bool loadFromFile( const std::wstring& fileNmee );

	const GVOImage& image() const
	{
		return m_mapImage;
	}

	POINT imageCoordFromWorldCoord( const POINT& worldCoord ) const;

	GVONormalizedPoint normalizedPoint( const POINT worldCoord ) const
	{
		return GVONormalizedPoint( worldCoord.x / (float)k_worldWidth, worldCoord.y / (float)k_worldHeight );
	}
};
