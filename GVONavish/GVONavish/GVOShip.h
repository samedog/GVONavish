#pragma once
#include <Windows.h>
#include <vector>
#include <deque>
#include "GVONoncopyable.h"
#include "GVONavish.h"
#include "GVOVector.h"
#include "GVOVelocity.h"





class GVOShip : private GVONoncopyable {
private:
	typedef std::deque<GVOVector> VectorArray;

private:
	POINT m_surveyCoord;		//!<@brief �ŐV���W
	GVOVector m_vector;			//!<@brief ���D�̃x�N�g��

	VectorArray m_vectorArray;
	double m_velocity;
	uint32_t m_timeStamp;
	GVOVelocity m_velocityPerSecond;

public:
	GVOShip() :
		m_surveyCoord(),
		m_velocity(),
		m_timeStamp()
	{
	}

	inline void setInitialSurveyCoord( const POINT& initialSurveyCoord )
	{
		m_surveyCoord = initialSurveyCoord;
	}

	inline const GVOVector& vector() const
	{
		return m_vector;
	}

	//!@brief ���ʍ��W�ɂ��ŐV�ʒu���X�V
	void updateWithSurveyCoord( const POINT& surveyCoord, const uint32_t timeStamp );

	//!@brief ���D�̕b�Ԉړ������i�����l�j
	inline double velocity() const
	{
		return m_velocityPerSecond.velocity();
	}
};
