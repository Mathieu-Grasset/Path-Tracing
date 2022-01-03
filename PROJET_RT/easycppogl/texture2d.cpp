/*******************************************************************************
* EasyCppOGL:   Copyright (C) 2019,                                            *
* Sylvain Thery, IGG Group, ICube, University of Strasbourg, France            *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Contact information: thery@unistra.fr                                        *
*******************************************************************************/

#include <texture2d.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

namespace EZCOGL
{


GLImage::GLImage(const std::string& filename, int force_channels)
{
	stbi_set_flip_vertically_on_load(1);
	data_ = stbi_load(filename.c_str(), &width_, &height_, &bpp_, force_channels);
	if (force_channels !=0)
		bpp_ = force_channels;
	if (data_ == nullptr)
	{
		width_=0;
		height_=0;
		bpp_=0;
		std::cerr << "Error while loading "<< filename << std::endl;
	}
}

GLImage::~GLImage()
{
	stbi_image_free(data_);
}



std::map<GLint,std::pair<GLenum,GLenum>> Texture2D::texture_formats =
{
 {GL_R8,{GL_RED,GL_UNSIGNED_BYTE}},
 {GL_RG8,{GL_RG,GL_UNSIGNED_BYTE}},
 {GL_RGB8,{GL_RGB,GL_UNSIGNED_BYTE}},
 {GL_RGBA8,{GL_RGBA,GL_UNSIGNED_BYTE}},
 {GL_R16,{GL_RED,GL_UNSIGNED_SHORT}},
 {GL_RG16,{GL_RG,GL_UNSIGNED_SHORT}},
 {GL_RGB16,{GL_RGB,GL_UNSIGNED_SHORT}},
 {GL_RGBA16,{GL_RGBA,GL_UNSIGNED_SHORT}},
 {GL_R16F,{GL_RED,GL_HALF_FLOAT}},
 {GL_RG16F,{GL_RG,GL_HALF_FLOAT}},
 {GL_RGB16F,{GL_RGB,GL_HALF_FLOAT}},
 {GL_RGBA16F,{GL_RGBA,GL_HALF_FLOAT}},
 {GL_R32F,{GL_RED,GL_FLOAT}},
 {GL_RG32F,{GL_RG,GL_FLOAT}},
 {GL_RGB32F,{GL_RGB,GL_FLOAT}},
 {GL_RGBA32F,{GL_RGBA,GL_FLOAT}},
 {GL_R16I,{GL_RED_INTEGER,GL_SHORT}},
 {GL_RG16I,{GL_RG_INTEGER,GL_SHORT}},
 {GL_RGB16I,{GL_RGB_INTEGER,GL_SHORT}},
 {GL_RGBA16I,{GL_RGBA_INTEGER,GL_SHORT}},
 {GL_R32I,{GL_RED_INTEGER,GL_INT}},
 {GL_RG32I,{GL_RG_INTEGER,GL_INT}},
 {GL_RGB32I,{GL_RGB_INTEGER,GL_INT}},
 {GL_RGBA32I,{GL_RGBA_INTEGER,GL_INT}},
 {GL_R16UI,{GL_RED_INTEGER,GL_UNSIGNED_SHORT}},
 {GL_RG16UI,{GL_RG_INTEGER,GL_UNSIGNED_SHORT}},
 {GL_RGB16UI,{GL_RGB_INTEGER,GL_UNSIGNED_SHORT}},
 {GL_RGBA16UI,{GL_RGBA_INTEGER,GL_UNSIGNED_SHORT}},
 {GL_R32UI,{GL_RED_INTEGER,GL_UNSIGNED_INT}},
 {GL_RG32UI,{GL_RG_INTEGER,GL_UNSIGNED_INT}},
 {GL_RGB32UI,{GL_RGB_INTEGER,GL_UNSIGNED_INT}},
 {GL_RGBA32UI,{GL_RGBA_INTEGER,GL_UNSIGNED_INT}},
 {GL_RGB10_A2,{GL_RGBA,GL_UNSIGNED_INT_2_10_10_10_REV}},
 {GL_DEPTH_COMPONENT,  {GL_DEPTH_COMPONENT,GL_UNSIGNED_INT}},
 {GL_DEPTH_COMPONENT16,{GL_DEPTH_COMPONENT,GL_UNSIGNED_SHORT}},
 {GL_DEPTH_COMPONENT24,{GL_DEPTH_COMPONENT,GL_UNSIGNED_INT}},
 {GL_DEPTH_COMPONENT32F,{GL_DEPTH_COMPONENT,GL_FLOAT}}
};

//Texture2D::Texture2D():
//		internal_(0),
//		external_(0),
//		data_type_(0),
//		width_(0),
//		height_(0)
//{
//	glGenTextures(1,&id_);
//	glBindTexture(GL_TEXTURE_2D, id_);
//	simple_params({GL_LINEAR, GL_CLAMP_TO_EDGE});
//}


Texture2D::Texture2D(const std::vector<GLint>& params):
		internal_(0),
		external_(0),
		data_type_(0),
		width_(0),
		height_(0)
{
	glGenTextures(1,&id_);
	glBindTexture(GL_TEXTURE_2D, id_);
	simple_params(params);
}


Texture2D::~Texture2D()
{
	glDeleteTextures(1,&id_);
}

void Texture2D::simple_params(const std::vector<GLint>& params)
{
	if (params.empty())
	{
		simple_params({GL_LINEAR,GL_CLAMP_TO_EDGE});
	}
	for(auto p: params)
	{
		if (p == GL_NEAREST || p == GL_LINEAR)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, p);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, p);
		}
		if (p == GL_CLAMP_TO_EDGE || p == GL_CLAMP_TO_BORDER || p == GL_REPEAT || p == GL_MIRRORED_REPEAT)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p);
		}
	}
}

void Texture2D::alloc(GLsizei w, GLsizei h, GLint internal, const GLubyte* ptr)
{
	internal_ = internal;
	auto tf = texture_formats[internal];
	external_ = tf.first;
	width_ = w;
	height_ = h;
	data_type_ = tf.second;
	depth_ = ((internal == GL_DEPTH_COMPONENT32F) || (internal == GL_DEPTH_COMPONENT24));
	if (w*h > 0)
	{
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, internal, w, h, 0, external_, data_type_, ptr);
		unbind();
	}
}


void Texture2D::init(GLint internal)
{
	internal_ = internal;
	auto tf = texture_formats[internal];
	external_ = tf.first;
	data_type_ = tf.second;
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, internal_, 0, 0, 0, external_, data_type_, nullptr);
	unbind();
}


void Texture2D::load(const std::string& filename, bool force_rgba)
{
	GLImage img(filename, force_rgba?4:0);

	switch (img.depth())
	{
		case 1:
			alloc(img.width(),img.height(),GL_R8, img.data());
			break;
		case 3:
			alloc(img.width(),img.height(),GL_RGB8, img.data());
			break;
		case 4:
			alloc(img.width(),img.height(),GL_RGBA8, img.data());
			break;
	}
	bind();
	glGenerateMipmap(GL_TEXTURE_2D);

}

void Texture2D::resize(GLsizei w, GLsizei h)
{
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, internal_, w, h, 0, external_, data_type_,nullptr);
	width_ = w;
	height_ = h;
	unbind();
}

}
