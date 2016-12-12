#include "stdafx.h"

// PNGÇ∆Ç©ÇÃì«Ç›çûÇ›óp
#include <objbase.h>
#include <gdiplus.h>
#include <gdiplusbitmap.h>

#include "GVOImage.h"


namespace {
	inline uint32_t s_strideFromWidthAndBitsPerPixel( const uint32_t width, const uint32_t bpp )
	{
		uint32_t stride = width * bpp / 8;
		stride = stride + (4 - stride % 4) % 4;
		return stride;
	}
	inline void s_copyImage24From32( uint8_t * const dst, const uint8_t * const src, const uint32_t width, const uint32_t height)
	{
		const uint32_t srcStride = s_strideFromWidthAndBitsPerPixel( width, 32 );
		const uint32_t dstStride = s_strideFromWidthAndBitsPerPixel( width, 24 );

		const uint8_t * s = src;
		uint8_t * d = dst;

		for ( uint32_t y = 0; y < height; ++y ) {
			s = src + (y * srcStride);
			d = dst + (y * dstStride);
			for ( uint32_t x = 0; x < width; ++x ) {
				*d++ = *s++;
				*d++ = *s++;
				*d++ = *s++;
				++s;
			}
		}
	}
}


bool GVOImage::stretchCopy( const GVOImage& src, uint32_t width, uint32_t height )
{
	if ( !createImage( width, height ) ) {
		return false;
	}

	HDC hdc = ::GetDC( NULL );
	HDC hdcSrc = ::CreateCompatibleDC( hdc );
	HDC hdcDst = ::CreateCompatibleDC( hdc );

	::SaveDC( hdcSrc );
	::SaveDC( hdcDst );
	::SelectObject( hdcDst, m_hbmp );
	::SelectObject( hdcSrc, src.m_hbmp );

	if ( m_size.cx != src.m_size.cx || m_size.cy != src.m_size.cy ) {
		POINT org;
		::GetBrushOrgEx( hdcDst, &org );
		::SetStretchBltMode( hdcDst, HALFTONE );
		::SetBrushOrgEx( hdcDst, org.x, org.y, NULL );

		::StretchBlt( hdcDst, 0, 0, m_size.cx, m_size.cy,
			hdcSrc, 0, 0, src.m_size.cx, src.m_size.cy,
			SRCCOPY );
	}
	else {
		::BitBlt( hdcDst, 0, 0, m_size.cx, m_size.cy,
			hdcSrc, 0, 0, SRCCOPY );
	}
	m_pixelFormat = src.m_pixelFormat;


	::RestoreDC( hdcSrc, -1 );
	::DeleteDC( hdcSrc );
	::RestoreDC( hdcDst, -1 );
	::DeleteDC( hdcDst );
	::ReleaseDC( NULL, hdc );
	return true;
}


bool GVOImage::createImage( int width, int height, GVOPixelFormat pixelFormat )
{
	reset();

	uint32_t bitCount = 0;

	if ( pixelFormat == k_GVOPixelFormat_RGB ) {
		BITMAPINFOHEADER bmih = { sizeof(bmih) };
		bmih.biWidth = width;
		bmih.biHeight = -height;
		bmih.biPlanes = 1;
		bmih.biBitCount = 24;
		bmih.biSizeImage = width * height * 3;
		m_hbmp = ::CreateDIBSection( NULL, (LPBITMAPINFO)&bmih, DIB_RGB_COLORS, (void **)&m_bits, NULL, 0 );
		bitCount = bmih.biBitCount;
	}
	else if ( pixelFormat == k_GVOPixelFormat_RGBA ) {
		BITMAPV5HEADER bmih = { sizeof(bmih) };
		bmih.bV5Compression = BI_BITFIELDS;
		bmih.bV5BlueMask = 0xFF;
		bmih.bV5GreenMask = 0xFF << 8;
		bmih.bV5RedMask = 0xFF << 16;
		bmih.bV5AlphaMask = 0xFF << 24;
		bmih.bV5Width = width;
		bmih.bV5Height = -height;
		bmih.bV5Planes = 1;
		bmih.bV5BitCount = 32;
		bmih.bV5SizeImage = width * height * 4;
		m_hbmp = ::CreateDIBSection( NULL, (LPBITMAPINFO)&bmih, DIB_RGB_COLORS, (void **)&m_bits, NULL, 0 );
		bitCount = bmih.bV5BitCount;
	}
	else {
		abort();
		return false;
	}
	if ( !m_hbmp ) {
		return false;
	}
	m_size.cx = width;
	m_size.cy = height;
	m_pixelFormat = pixelFormat;
	m_stride = s_strideFromWidthAndBitsPerPixel( width, bitCount );
	return true;
}


bool GVOImage::loadFromFile( const std::wstring& fileName )
{
	reset();

	std::auto_ptr<Gdiplus::Bitmap> image;
	HBITMAP hbmp = NULL;
	image.reset( Gdiplus::Bitmap::FromFile( fileName.c_str() ) );
	image->GetHBITMAP( Gdiplus::Color( 0, 0, 0 ), &hbmp );
	Gdiplus::PixelFormat srcPixelFormat = image->GetPixelFormat();
	image.reset();
	if ( !hbmp ) {
		return false;
	}

	GVOPixelFormat pixelFormat;

	if ( srcPixelFormat & PixelFormatGDI ){
		pixelFormat = k_GVOPixelFormat_RGB;
	}
	else if ( srcPixelFormat & (PixelFormatGDI | PixelFormatAlpha) ) {
		pixelFormat = k_GVOPixelFormat_RGBA;
	}
	else {
		abort();
		return false;
	}

	BITMAP bmp = { 0 };
	::GetObject( hbmp, sizeof(bmp), &bmp );
	if ( !createImage( bmp.bmWidth, bmp.bmHeight, pixelFormat ) ) {
		::DeleteObject( hbmp );
		return false;
	}

	std::vector<uint8_t> buffer;
	buffer.resize( ::GetBitmapBits( hbmp, 0, NULL ) );
	::GetBitmapBits( hbmp, buffer.size(), &buffer[0] );
	::DeleteObject( hbmp );

	if ( pixelFormat == k_GVOPixelFormat_RGB ) {
		switch ( bmp.bmBitsPixel ) {
		case 24:
			::memcpy( m_bits, &buffer[0], m_stride );
			break;
		case 32:
			::s_copyImage24From32( m_bits, &buffer[0], m_size.cx, m_size.cy );
			break;
		default:
			return false;
		}
	}
	else if ( pixelFormat == k_GVOPixelFormat_RGBA ) {
		::memcpy( m_bits, &buffer[0], m_stride );
	}
	else {
		abort();
		return false;
	}
	return true;
}
