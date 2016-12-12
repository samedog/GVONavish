#pragma once
#include <math.h>
#include "GVONavish.h"
#include "GVONormalizedPoint.h"


// 簡易二次元ベクトル
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

	// 世界座標の2点からベクトルを算出する。
	// ただしX軸が世界の半分以上離れていると逆方向でベクトルを取る。
	// （円筒上の近い方の距離を採用する）
	//
	// TODO: できればWorldMapに処理を移したい。
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

	// 正規化された世界座標2点からベクトルを算出する
	//
	// TODO: できればWorldMapに処理を移したい。
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

	// ベクトルのx成分
	inline double x() const
	{
		return m_x;
	}

	// ベクトルのy成分
	inline double y() const
	{
		return m_y;
	}

	// ベクトル長
	inline double length() const
	{
		return m_length;
	}

	// 単位ベクトルを返す
	inline GVOVector normalizedVector() const
	{
		return normalizedVector( length() );
	}

	// 任意の長さでベクトルを正規化する
	inline GVOVector normalizedVector( const double norm ) const
	{
		GVOVector v( m_x, m_y );
		const double length = v.length();
		v.m_x = (v.m_x / length) * norm;
		v.m_y = (v.m_y / length) * norm;
		v.m_length = norm;
		return v;
	}

	// ２つのベクトルの角度を取る
	inline double angleTo( const GVOVector& other )const
	{
		return ::atan2( other.m_x * m_y - m_x * other.m_y, m_x * other.m_x + m_y * other.m_y );
	}

	// ベクトルを合成する
	inline void composite( const GVOVector& other )
	{
		m_x += other.m_x;
		m_y += other.m_y;
		m_length = calcLength( m_x, m_y );
	}

	//!@brief 原点とベクトル長を指定して座標を得る
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
	// ベクトル長を計算する
	static inline double calcLength(const double x, const double y)
	{
		return ::pow( x * x + y * y, 0.5 );
	}

};
