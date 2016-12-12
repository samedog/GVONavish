#pragma once
#include <math.h>
#include "GVONavish.h"
#include "GVONormalizedPoint.h"


// �ȈՓ񎟌��x�N�g��
class GVOVector {
private:
	double m_x = 0.0;
	double m_y = 0.0;
	double m_length = 0.0;

public:
	GVOVector() = default;
	GVOVector( const double x, const double y ) :
		m_x( x ),
		m_y( y ),
		m_length( calcLength( x, y ) )
	{
	}

	// ���E���W��2�_����x�N�g�����Z�o����B
	// ������X�������E�̔����ȏ㗣��Ă���Ƌt�����Ńx�N�g�������B
	// �i�~����̋߂����̋������̗p����j
	//
	// TODO: �ł����WorldMap�ɏ������ڂ������B
	GVOVector( const POINT& p1, const POINT& p2 )
	{
		const LONG k_threshold = k_worldWidth / 2;
		LONG dx = p2.x - p1.x;

		if ( k_threshold < dx ) {
			dx -= k_worldWidth;
		}
		else if ( dx < -k_threshold ) {
			dx += k_worldWidth;
		}
		m_x = dx;
		m_y = p2.y - p1.y;
		m_length = calcLength( m_x, m_y );
	}

	// ���K�����ꂽ���E���W2�_����x�N�g�����Z�o����
	//
	// TODO: �ł����WorldMap�ɏ������ڂ������B
	GVOVector( const GVONormalizedPoint & p1, const GVONormalizedPoint&p2 )
	{
		const float k_threshold = 0.5f;
		float dx = p2.x() - p1.x();

		if ( k_threshold < dx ) {
			dx -= k_threshold;
		}
		else if ( dx < -k_threshold ) {
			dx += k_threshold;
		}
		m_x = dx;
		m_y = p2.y() - p1.y();
		m_length = calcLength( m_x, m_y );
	}

	// �x�N�g����x����
	inline double x() const
	{
		return m_x;
	}

	// �x�N�g����y����
	inline double y() const
	{
		return m_y;
	}

	// �x�N�g����
	inline double length() const
	{
		return m_length;
	}

	// �P�ʃx�N�g����Ԃ�
	inline GVOVector normalizedVector() const
	{
		return normalizedVector( length() );
	}

	// �C�ӂ̒����Ńx�N�g���𐳋K������
	inline GVOVector normalizedVector( const double norm ) const
	{
		GVOVector v( m_x, m_y );
		const double length = v.length();
		v.m_x = (v.m_x / length) * norm;
		v.m_y = (v.m_y / length) * norm;
		v.m_length = norm;
		return v;
	}

	// �Q�̃x�N�g���̊p�x�����
	inline double angleTo( const GVOVector& other )const
	{
		return ::atan2( other.m_x * m_y - m_x * other.m_y, m_x * other.m_x + m_y * other.m_y );
	}

	// �x�N�g������������
	inline void composite( const GVOVector& other )
	{
		m_x += other.m_x;
		m_y += other.m_y;
		m_length = calcLength( m_x, m_y );
	}

	//!@brief ���_�ƃx�N�g�������w�肵�č��W�𓾂�
	POINT pointFromOriginWithLength( const POINT& origin, const LONG length ) const
	{
		GVOVector v = normalizedVector();
		const POINT p = {
			origin.x + LONG( v.x() * length ),
			origin.y + LONG( v.y() * length )
		};
		return p;
	}
private:
	// �x�N�g�������v�Z����
	static inline double calcLength(const double x, const double y)
	{
		return ::pow( x * x + y * y, 0.5 );
	}

};
