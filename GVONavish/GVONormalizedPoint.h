#pragma once

class GVONormalizedPoint {
private:
	float m_x;
	float m_y;

public:
	GVONormalizedPoint() :
		m_x(),
		m_y()
	{
	}
	GVONormalizedPoint(float x, float y) :
		m_x( x ),
		m_y( y )
	{
	}

	bool isEqualValue( const GVONormalizedPoint rhs ) const
	{
		return m_x == rhs.m_x && m_y == rhs.m_y;
	}

	float x() const
	{
		return m_x;
	}

	float y() const
	{
		return m_y;
	}

};

// ファイル入出力用にアライメント確認
static_assert(sizeof(GVONormalizedPoint) == (sizeof(float)*2), "bat size.");
