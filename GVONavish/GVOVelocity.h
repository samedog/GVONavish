#pragma once
#include <cstdint>


// 1秒あたりの移動速度
class GVOVelocity {
private:
	double m_velocityPerSecond;

public:
	GVOVelocity() :
		m_velocityPerSecond()
	{
	}
	GVOVelocity( const double velocity, const uint32_t dt )
	{
		setVelocity( velocity, dt );
	}
	~GVOVelocity()
	{
	}

	void setVelocity( const double velocity, const uint32_t dt )
	{
		m_velocityPerSecond = (velocity / dt) * 1000.0;
	}
	double velocity() const
	{
		return m_velocityPerSecond;
	}
};
