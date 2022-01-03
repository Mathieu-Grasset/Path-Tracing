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

#ifndef EASY_CPP_OGL_VAO_H_
#define EASY_CPP_OGL_VAO_H_

#include <GL/gl3w.h>
#include <vbo.h>
#include <tuple>

namespace EZCOGL
{
class VAO;
using SP_VAO = std::shared_ptr<VAO>;

class VAO
{
protected:
	GLuint id_;
	GLuint nb_;
	static SP_VAO none_;

public:

	VAO(const std::vector<std::tuple<GLint,SP_VBO>>& att_vbo);

	VAO(const std::vector<std::tuple<GLint,SP_VBO,GLint>>& att_vbo);

	VAO(const std::vector<std::tuple<GLint,SP_VBO,GLint,GLint,GLint,GLint>>& att_vbo);


	VAO(const VAO&) = delete ;

	static SP_VAO none();
	static void bind_none();

	/**
	 * @brief create VAO
	 * @param att_vbo couple attribute id, VBO
	 * @return
	 */
	inline static SP_VAO create(const std::vector<std::tuple<GLint,SP_VBO>>& att_vbo)
	{
		return std::make_shared<VAO>(att_vbo);
	}

	/**
	 * @brief create VAO for instancing
	 * @param att_vbo triplet attribute id, VBO, instancing divisor (read the OGL doc!)
	 * @return
	 */
	inline static SP_VAO create(const std::vector<std::tuple<GLint,SP_VBO, GLint>>& att_vbo)
	{
		return std::make_shared<VAO>(att_vbo);
	}

	/**
	 * @brief create with all param (do not use, expert only)
	 * @param att_vbo
	 * @return
	 */
	inline static SP_VAO create_interleaved(const std::vector<std::tuple<GLint,SP_VBO, GLint, GLint, GLint,GLint>>& att_vbo)
	{
		return std::make_shared<VAO>(att_vbo);
	}


	inline void bind()
	{
		glBindVertexArray(id_);
	}

	static inline void unbind()
	{
		glBindVertexArray(0);
	}

	inline GLint length() const
	{
		return GLint(nb_);
	}
};

} // namespace

#endif // EASY_CPP_OGL_VAO_H_
