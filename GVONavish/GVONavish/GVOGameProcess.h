#pragma once
#include "GVONoncopyable.h"
#include "GVOImage.h"
#include "GVOConfig.h"
#include "GVOSpeedMeter.h"
#include "GVOShip.h"

#include "GVOGameStatus.h"




//!@brief 大航海時代Onlineプロセス
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

	//!@brief ゲームプロセスに関するリソースを解放する。
	void clear();

	//!@brief 設定情報で初期化する
	void setup( const GVOConfig& config );
	void teardown();
#ifndef NDEBUG
	void enableDebugAutoCruise( bool enabled );
	void setPollingInterval( DWORD interval );
#endif

	//!@brief ゲーム画面を読み取った結果を受け取る
	//!@return 読み取り成功時には測量座標、自船ベクトル、自船速度が結果として渡される。
	//!@note 実行すると蓄積された状態は全て消去される
	std::vector<GVOGameStatus> getState();

	//!@brief ゲーム画面読み取り成功時の時間
	DWORD timeStamp() const
	{
		return m_timeStamp;
	}

	// Wait用
	HANDLE dataReadyEvent() const
	{
		return m_dataReadyEvent;
	}

#ifndef NDEBUG
	//!@brief デバッグ用測量座標画像
	const GVOImage& surveyCoordImage() const
	{
		return m_surveyCoordImage;
	}
#endif

	//!@attention 画像構築中にアクセスされないよう注意。
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
