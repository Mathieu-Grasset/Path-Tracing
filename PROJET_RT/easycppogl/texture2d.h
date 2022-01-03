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

#ifndef EASY_CPP_OGL_TEXTURE_H_
#define EASY_CPP_OGL_TEXTURE_H_

#include <GL/gl3w.h>
#include <map>
#include <string>
#include <vector>
#include <memory>

namespace EZCOGL
{


class GLImage
{
	GLubyte* data_;
	GLint width_;
	GLint height_;
	GLint bpp_;

public:
	GLImage(const std::string& filename,int force_channels=0);
	~GLImage();
	inline GLint depth() const { return bpp_;}
	inline GLint width() const { return width_;}
	inline GLint height() const { return height_;}
	inline const GLubyte* data() const { return data_;}
};

class Texture2D;
using SP_Texture2D = std::shared_ptr<Texture2D>;

class Texture2D
{
protected:
	GLuint id_;
	GLint internal_;
	GLenum external_;
	GLenum data_type_;
	GLsizei width_;
	GLsizei height_;
	bool depth_;

public:
	Texture2D(const std::vector<GLint>& params = {});
	static std::map<GLint,std::pair<GLenum,GLenum>> texture_formats;

	~Texture2D();

	static SP_Texture2D create(const std::vector<GLint>& params = {})
	{
		return std::make_shared<Texture2D>(params);
	}

	Texture2D(const Texture2D&) = delete ;

	inline GLuint id()
	{
		return id_;
	}

	void simple_params(const std::vector<GLint>& params);

	void alloc(GLsizei w, GLsizei h, GLint internal, const GLubyte* ptr=nullptr);

	void init(GLint internal);

	void load(const std::string& filename, bool force_rgba=false);

	void resize(GLsizei w, GLsizei h);

	template <typename T>
	void update(GLint x, GLint y, GLint w, GLint h, const T* data);

	inline GLsizei width() const
	{
		return width_;
	}

	inline GLsizei height() const
	{
		return height_;
	}

	inline void bind()
	{
		glBindTexture(GL_TEXTURE_2D, id_);
	}

	inline GLint bind(GLint unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, id_);
		return unit;
	}

	inline static void unbind()
	{
		glBindTexture(GL_TEXTURE_2D,0);
	}

	inline GLuint bind_compute_in(GLuint img_binding)
	{
		glBindImageTexture(img_binding, id_, 0, GL_FALSE, 0, GL_READ_ONLY, internal_);
		return img_binding;
	}

	inline GLuint bind_compute_out(GLuint img_binding)
	{
		glBindImageTexture(img_binding, id_, 0, GL_FALSE, 0, GL_WRITE_ONLY, internal_);
		return img_binding;
	}

	inline static void unbind_compute_in(GLuint img_binding)
	{
		glBindImageTexture(img_binding, 0, 0, GL_FALSE, 0, GL_READ_ONLY, 0);
	}

	inline static void unbind_compute_out(GLuint img_binding)
	{
		glBindImageTexture(img_binding, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, 0);
	}

};

template <typename T>
void Texture2D::update(GLint x, GLint y,GLint w, GLint h, const T* data)
{
	bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, x,y,w,h, external_, data_type_, data);
	unbind();
}


}
#endif
