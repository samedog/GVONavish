#include "stdafx.h"
#include <string>
#include "GVOShip.h"

namespace {
	// �Q�[�����p�x�i�Q�x���x�j�Ɋۂ߂�
	inline double s_roundInGameAngle( const double radian )
	{
		double degree = g_degreeFromRadian( radian );
		degree = ::floor( ::round( degree ) * 0.5 ) * 2.0;
		return g_radianFromDegree( degree );
	}
	inline GVOVector s_roundInGameVector( const GVOVector& v )
	{
		// �^������̊p�x���擾
		double sita = GVOVector( 1, 0 ).angleTo( v );
		// 2�x����360�x����\�Ɋۂ߂�
		sita = s_roundInGameAngle( sita );
		// �ʓx�ɖ߂�
		// ���E���W�n�͏オ0�Ȃ̂�Y���𔽓]������
		return GVOVector( ::cos( sita ), -::sin( sita ) );
	}

	// ���̃x�N�g���ŕ\���o����Q�[�������p�̕���\���Z�o
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

	// �ړ����ĂȂ���Ζ�������B
	if ( m_velocity == 0.0 ) {
		return;
	}
	m_surveyCoord = surveyCoord;

	// ���݂̃x�N�g�����ݒ肳��Ă��Ȃ���΃p�����[�^�[�����̂܂܍̗p����
	if ( m_vector.length() == 0.0 ) {
		m_vector = s_roundInGameVector( v.normalizedVector() );
		return;
	}

	// �V����������قȂ�p�x�ƌ��Ȃ���x�N�g����T������
	GVOVector headVector = v;
	for ( VectorArray::const_reverse_iterator it = m_vectorArray.rbegin(); it != m_vectorArray.rend(); ++it ) {
		headVector.composite( *it );
		if ( s_isAnotherDirection(m_vector, headVector) ) {
			// ����ȍ~�͎Q�l�ɂȂ�Ȃ��̂ō폜����B
			m_vectorArray.erase( m_vectorArray.begin(), std::next( it ).base() );
			break;
		}
	}
	m_vector = s_roundInGameVector( headVector.normalizedVector() );

	// �ŐV�̃x�N�g�������X�g�ɉ�����B
	m_vectorArray.push_back( v );

	// �v�Z���90����Ε���\180�̊p�x���Z�o�ł���͂������덷�𓥂܂���2�{�̋�����ێ����Ă���
	if ( 180 < (headVector.length() - m_vectorArray.front().length()) ) {
		m_vectorArray.pop_front();
	}
}
