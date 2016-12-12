#pragma once
#include <cinttypes>
#include <Windows.h>

#include "GVOVector.h"


//!@brief ゲームプロセスから取得した状態を纏めたモノ
class GVOGameStatus
{
public:
	DWORD m_timeStamp;
	POINT m_surveyCoord;		//!<@brief 測量座標
	GVOVector m_shipVector;		//!<@brief 自船の向き
	double m_shipVelocity;		//!<@brief 自船の速度(推測値）

	GVOGameStatus() : m_shipVelocity()
	{
	}

};
