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
#include <texture3d.h>

namespace EZCOGL
{

Texture3D::Texture3D():
		internal_(0),
		external_(0),
		data_type_(0),
		width_(0),
		height_(0),
		depth_(false)
{
	glGenTextures(1,&id_);
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


Texture3D::~Texture3D()
{
	glDeleteTextures(1,&id_);
}

void Texture3D::simple_params(const std::vector<GLint>& params)
{
	for(auto p: params)
	{
		if (p == GL_NEAREST || p == GL_LINEAR)
		{
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, p);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, p);
		}
		if (p == GL_CLAMP_TO_EDGE || p == GL_REPEAT || p == GL_MIRRORED_REPEAT)
		{
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, p);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, p);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, p);
		}
	}
}

void Texture3D::alloc(GLsizei w, GLsizei h, GLsizei d, GLint internal, const GLubyte* ptr)
{
	internal_ = internal;
	auto tf = Texture2D::texture_formats[internal];
	external_ = tf.first;
	width_ = w;
	height_ = h;
	depth_ = d;
	data_type_ = tf.second;
	if (w*h*d > 0)
	{
		bind();
		glTexImage3D(GL_TEXTURE_3D, 0, internal, w, h, d, 0, external_, data_type_, ptr);
		unbind();
	}
}


void Texture3D::init(GLint internal)
{
	internal_ = internal;
	auto tf = Texture2D::texture_formats[internal];
	external_ = tf.first;
	data_type_ = tf.second;
	bind();
	glTexImage3D(GL_TEXTURE_3D, 0, internal_, 0, 0, 0, 0, external_, data_type_, nullptr);
	unbind();
}



void Texture3D::resize(GLsizei w, GLsizei h, GLsizei d)
{
	bind();
	glTexImage3D(GL_TEXTURE_3D, 0, internal_, w, h, d, 0, external_, data_type_,nullptr);
	width_ = w;
	height_ = h;
	unbind();
}

}
