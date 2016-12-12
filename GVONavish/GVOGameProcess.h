#pragma once
#include "GVONoncopyable.h"
#include "GVOImage.h"
#include "GVOConfig.h"
#include "GVOSpeedMeter.h"
#include "GVOShip.h"

#include "GVOGameStatus.h"




//!@brief ��q�C����Online�v���Z�X
class GVOGameProcess : private GVONoncopyable {
private:
	HANDLE m_process;
	HWND m_window;
	GVOImage m_shipIconImage;

	GVOImage m_surveyCoordImage;
	POINT m_surveyCoord;
	DWORD m_timeStamp;

	GVOSpeedMeter m_speedMeter;
	GVOShip m_ship;

	uint32_t m_pollingInterval;
	HANDLE m_pollingTimerEvent;
	UINT m_pollingTimerEventID;

	HANDLE m_workerThread;
	HANDLE m_threadQuitSignal;
	HANDLE m_dataReadyEvent;
	CRITICAL_SECTION m_lock;

	std::vector<GVOGameStatus> m_statusArray;
public:
	GVOGameProcess() :
		m_process( NULL ),
		m_window( NULL ),
		m_surveyCoord(),
		m_timeStamp(),
		m_pollingInterval(),
		m_pollingTimerEvent( ::CreateEvent( NULL, TRUE, TRUE, NULL ) ),
		m_pollingTimerEventID(),
		m_workerThread(),
		m_dataReadyEvent( ::CreateEvent( NULL, TRUE, FALSE, NULL ) )
	{
		::InitializeCriticalSection( &m_lock );
	}
	virtual ~GVOGameProcess()
	{
		clear();
		::CloseHandle( m_dataReadyEvent );
		::CloseHandle( m_pollingTimerEvent );
		::DeleteCriticalSection( &m_lock );
	}

	HANDLE processHandle() const
	{
		return m_process;
	}

	//!@brief �Q�[���v���Z�X�Ɋւ��郊�\�[�X���������B
	void clear();

	//!@brief �ݒ���ŏ���������
	void setup( const GVOConfig& config );
	void teardown();
#ifndef NDEBUG
	void enableDebugAutoCruise( bool enabled );
	void setPollingInterval( DWORD interval );
#endif

	//!@brief �Q�[����ʂ�ǂݎ�������ʂ��󂯎��
	//!@return �ǂݎ�萬�����ɂ͑��ʍ��W�A���D�x�N�g���A���D���x�����ʂƂ��ēn�����B
	//!@note ���s����ƒ~�ς��ꂽ��Ԃ͑S�ď��������
	std::vector<GVOGameStatus> getState();

	//!@brief �Q�[����ʓǂݎ�萬�����̎���
	DWORD timeStamp() const
	{
		return m_timeStamp;
	}

	// Wait�p
	HANDLE dataReadyEvent() const
	{
		return m_dataReadyEvent;
	}

#ifndef NDEBUG
	//!@brief �f�o�b�O�p���ʍ��W�摜
	const GVOImage& surveyCoordImage() const
	{
		return m_surveyCoordImage;
	}
#endif

	//!@attention �摜�\�z���ɃA�N�Z�X����Ȃ��悤���ӁB
	const GVOImage * shipIconImage()
	{
		const GVOImage * img = NULL;
		::EnterCriticalSection( &m_lock );
		if ( m_shipIconImage.bitmapHandle() ) {
			img = &m_shipIconImage;
		}
		::LeaveCriticalSection( &m_lock );
		return img;
	}
private:
	bool updateState();
	static UINT CALLBACK threadMainThunk( LPVOID arg );
	void threadMain();
	void grabImage( HDC hdc, const POINT& offset, const SIZE& size );
	bool updateSurveyCoord();
	void extractGameIcon();
};
