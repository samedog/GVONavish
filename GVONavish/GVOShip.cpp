#include "stdafx.h"
#include <string>
#include "GVOShip.h"

namespace {
	// ゲーム内角度（２度精度）に丸める
	inline double s_roundInGameAngle( const double radian )
	{
		double degree = g_degreeFromRadian( radian );
		degree = ::floor( ::round( degree ) * 0.5 ) * 2.0;
		return g_radianFromDegree( degree );
	}
	inline GVOVector s_roundInGameVector( const GVOVector& v )
	{
		// 真東からの角度を取得
		double sita = GVOVector( 1, 0 ).angleTo( v );
		// 2度ずつの360度分解能に丸める
		sita = s_roundInGameAngle( sita );
		// 弧度に戻す
		// 世界座標系は上が0なのでY軸を反転させる
		return GVOVector( ::cos( sita ), -::sin( sita ) );
	}

	// そのベクトルで表現出来るゲーム内方角の分解能を算出
	inline double s_resolutionForVector( const GVOVector& vector )
	{
		const double length = vector.length();
		if ( length == 0.0 ) {
			return 0.0;
		}
		const double resolution = M_PI_2 / length;
		return resolution;
	}

	inline bool s_isAnotherDirection( const GVOVector& v1, const GVOVector& v2 )
	{
		const double resolution = s_resolutionForVector( v2 );
		const double angle = v1.angleTo( v2 );
		return resolution < ::fabs( angle );
	}
}


void GVOShip::updateWithSurveyCoord( const POINT& surveyCoord, const uint32_t timeStamp )
{
	const GVOVector v( m_surveyCoord, surveyCoord );

	m_velocity = v.length();
	m_velocityPerSecond.setVelocity( m_velocity, timeStamp - m_timeStamp );
	m_timeStamp = timeStamp;

	// 移動してなければ無視する。
	if ( m_velocity == 0.0 ) {
		return;
	}
	m_surveyCoord = surveyCoord;

	// 現在のベクトルが設定されていなければパラメーターをそのまま採用する
	if ( m_vector.length() == 0.0 ) {
		m_vector = s_roundInGameVector( v.normalizedVector() );
		return;
	}

	// 新しい順から異なる角度と見なせるベクトルを探索する
	GVOVector headVector = v;
	for ( VectorArray::const_reverse_iterator it = m_vectorArray.rbegin(); it != m_vectorArray.rend(); ++it ) {
		headVector.composite( *it );
		if ( s_isAnotherDirection(m_vector, headVector) ) {
			// これ以降は参考にならないので削除する。
			m_vectorArray.erase( m_vectorArray.begin(), std::next( it ).base() );
			break;
		}
	}
	m_vector = s_roundInGameVector( headVector.normalizedVector() );

	// 最新のベクトルをリストに加える。
	m_vectorArray.push_back( v );

	// 計算上は90あれば分解能180の角度が算出できるはずだが誤差を踏まえて2倍の距離を保持しておく
	if ( 180 < (headVector.length() - m_vectorArray.front().length()) ) {
		m_vectorArray.pop_front();
	}
}
