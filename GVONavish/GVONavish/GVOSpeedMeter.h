#pragma once
#include <deque>
#include <algorithm>
#include "GVONoncopyable.h"

class GVOSpeedMeter : private GVONoncopyable {
private:
	typedef std::deque<double> Array;
	struct VelocityLogItem {
		uint32_t timeStamp;
		double velocity;

		VelocityLogItem() :
			timeStamp(),
			velocity()
		{
		}
		VelocityLogItem( const uint32_t timeStamp, const double velocity ) :
			timeStamp( timeStamp ),
			velocity( velocity )
		{
		}
	};
	typedef std::deque<VelocityLogItem> VelocityyArray;
	typedef std::deque<double> VelocityLog;
	const uint32_t k_velocityMeasuringDistance = 5000;

private:
	VelocityyArray m_velocityArray;
	VelocityLog m_velocityLog;
	double m_velocity;

public:

	GVOSpeedMeter() :
		m_velocity()
	{
	}

	~GVOSpeedMeter()
	{
	}

	inline void updateVelocity( const double velocity, const uint32_t timeStamp )
	{
		m_velocityArray.push_back( VelocityLogItem( timeStamp, velocity ) );

		removeOldItem( timeStamp );
		updateVelocityLog();

		m_velocity = fastestVelocity();;
	}

	double velocity() const
	{
		return m_velocity;
	}

private:
	inline double calcVelocityPerSecond()
	{
		double velocity = 0.0;

		if ( m_velocityArray.size() < 2 ) {
			return 0.0;
		}

		// ˆÚ“®•½‹Ï’l
		for ( VelocityyArray::const_iterator it = m_velocityArray.begin(); it != m_velocityArray.end(); ++it ) {
			velocity += it->velocity;
		}
		velocity /= m_velocityArray.size();
		return velocity;
	}

	inline void removeOldItem(const uint32_t timeStamp)
	{
		VelocityyArray::const_iterator removeMark = m_velocityArray.end();
		for ( VelocityyArray::const_iterator it = m_velocityArray.begin(); it != m_velocityArray.end(); ++it ) {
			const uint32_t dt = timeStamp - it->timeStamp;
			if ( dt <= k_velocityMeasuringDistance ) {
				break;
			}
			removeMark = it;
		}
		if ( removeMark != m_velocityArray.end() ) {
			m_velocityArray.erase( m_velocityArray.begin(), removeMark );
		}
	}

	inline void updateVelocityLog()
	{
		m_velocityLog.push_back( calcVelocityPerSecond() );
		if ( 3 < m_velocityLog.size() ) {
			m_velocityLog.pop_front();
		}
	}

	inline double fastestVelocity() const
	{
		double fastest = 0.0;

		VelocityLog::const_iterator it = std::max_element( m_velocityLog.begin(), m_velocityLog.end() );
		if ( it != m_velocityLog.end() ) {
			fastest = *it;
		}
		return fastest;
	}
};
