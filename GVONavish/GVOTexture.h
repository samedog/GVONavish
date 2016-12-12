#pragma once
#include "GVONoncopyable.h"
#include "GVOImage.h"




//!@brief OpenGLテクスチャクラス
class GVOTexture : private GVONoncopyable {
private:
	GLuint m_texID;
	int m_width;
	int m_height;

public:
	GVOTexture();
	~GVOTexture();

	int width() const
	{
		return m_width;
	}

	int height() const
	{
		return m_height;
	}

	void setImage( const GVOImage & image );

	void bind();

	void unbind();
};
