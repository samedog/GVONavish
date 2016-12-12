#pragma once
#include <cinttypes>
#include <Windows.h>

#include "GVOVector.h"


//!@brief �Q�[���v���Z�X����擾������Ԃ�Z�߂����m
class GVOGameStatus
{
public:
	DWORD m_timeStamp;
	POINT m_surveyCoord;		//!<@brief ���ʍ��W
	GVOVector m_shipVector;		//!<@brief ���D�̌���
	double m_shipVelocity;		//!<@brief ���D�̑��x(�����l�j

	GVOGameStatus() : m_shipVelocity()
	{
	}

};
