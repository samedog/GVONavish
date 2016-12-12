#include "stdafx.h"
#include "GVOTexture.h"
#include "GVOImage.h"




GVOTexture::GVOTexture() :
	m_texID(),
	m_width(),
	m_height()
{
	::glGenTextures( 1, &m_texID );
}


GVOTexture::~GVOTexture()
{
	if ( m_texID ) {
		unbind();
		::glDeleteTextures( 1, &m_texID );
	}
}


void GVOTexture::setImage( const GVOImage & image )
{
	bind();

	if ( image.pixelFormat() == k_GVOPixelFormat_RGB ) {
		::glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		::glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
			image.width(), image.height(),
			0, GL_BGR_EXT,
			GL_UNSIGNED_BYTE, image.imageBits() );
	}
	else if ( image.pixelFormat() == k_GVOPixelFormat_RGBA ) {
		::glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
		::glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
			image.width(), image.height(),
			0, GL_BGRA_EXT,
			GL_UNSIGNED_BYTE, image.imageBits() );
	}
	else {
		abort();
	}
	m_width = image.width();
	m_height = image.height();

	unbind();
}


void GVOTexture::bind()
{
	::glEnable( GL_TEXTURE_2D );
	::glBindTexture( GL_TEXTURE_2D, m_texID );

	::glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

	::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
}


void GVOTexture::unbind()
{
	::glBindTexture( GL_TEXTURE_2D, 0 );
	::glDisable( GL_TEXTURE_2D );
}
