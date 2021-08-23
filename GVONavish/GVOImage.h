#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <Windows.h>
#include <string>


#include "GVONoncopyable.h"


//!@brief 画像のピクセルフォーマット
enum GVOPixelFormat {
	k_GVOPixelFormat_Unknown,	//!<@brief 未定義値
	k_GVOPixelFormat_RGB,		//!<@brief R8G8B8
	k_GVOPixelFormat_RGBA,		//!<@brief R8G8B8A8
};

class GVOImage :private GVONoncopyable {
private:
	HBITMAP m_hbmp;
	SIZE m_size;
	GVOPixelFormat m_pixelFormat;
	uint8_t * m_bits;
	uint32_t m_stride;

public:
	GVOImage() :
		m_hbmp(),
		m_size(),
		m_bits(),
		m_stride()
	{
	}
	~GVOImage()
	{
		reset();
	}
	void reset()
	{
		if ( m_hbmp ) {
			::DeleteObject( m_hbmp );
			m_hbmp = NULL;
		}
		m_size = SIZE();
		m_stride = 0;
		m_pixelFormat = k_GVOPixelFormat_Unknown;
	}
	void copy( const GVOImage & src )
	{
		createImage( src.m_size, src.m_pixelFormat );
		::memcpy( m_bits, src.m_bits, src.m_size.cy * src.m_stride );
	}
	bool stretchCopy( const GVOImage& src, const SIZE& size )
	{
		return stretchCopy( src, size.cx, size.cy );
	}
	bool stretchCopy( const GVOImage& src, uint32_t width, uint32_t height );
	bool isCompatible( const SIZE& size ) const
	{
		if ( !m_hbmp ) {
			return false;
		}
		if ( m_size.cx != size.cx || m_size.cy != size.cy ) {
			return false;
		}
		return true;
	}
	HBITMAP bitmapHandle() const
	{
		return m_hbmp;
	}
	const SIZE& size() const
	{
		return m_size;
	}
	LONG width() const
	{
		return m_size.cx;
	}
	LONG height() const
	{
		return m_size.cy;
	}
	GVOPixelFormat pixelFormat() const
	{
		return m_pixelFormat;
	}
	uint32_t stride() const
	{
		return m_stride;
	}
	const uint8_t * imageBits() const
	{
		return m_bits;
	}
	uint8_t * mutableImageBits()
	{
		return m_bits;
	}

	bool createImage( int width, int height, GVOPixelFormat pixelFormat = k_GVOPixelFormat_RGB );
	bool createImage( const SIZE& size, GVOPixelFormat pixelFormat = k_GVOPixelFormat_RGB )
	{
		return createImage( size.cx, size.cy, pixelFormat );
	}
	bool loadFromFile( const std::wstring& fileName );
};
