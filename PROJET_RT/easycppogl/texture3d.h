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


#ifndef EASY_CPP_OGL_TEXTURE3D_H_
#define EASY_CPP_OGL_TEXTURE3D_H_

#include <GL/gl3w.h>
#include <vector>

namespace EZCOGL
{

class Texture3D;
using SP_Texture3D = std::shared_ptr<Texture3D>;

class Texture3D
{
public:

	Texture3D();

	~Texture3D();

	inline GLuint id()
	{
		return id_;
	}

	void simple_params(const std::vector<GLint>& params);

	void alloc(GLsizei w, GLsizei h, GLsizei d, GLint internal, const GLubyte* ptr=nullptr);

	void init(GLint internal);

	void resize(GLsizei w, GLsizei h, GLsizei d);

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

protected:
	GLuint id_;
	GLint internal_;
	GLenum external_;
	GLenum data_type_;
	GLsizei width_;
	GLsizei height_;
	GLsizei depth_;
};

}
#endif
