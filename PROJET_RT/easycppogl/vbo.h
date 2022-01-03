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

#ifndef EASY_CPP_OGL_VBO_H_
#define EASY_CPP_OGL_VBO_H_

#include <GL/gl3w.h>
#include <vector>
#include <gl_eigen.h>
#include <memory>

namespace EZCOGL
{

class VBO;
using SP_VBO = std::shared_ptr<VBO>;


class  VBO
{
protected:
	GLuint id_;
	GLuint nb_vectors_;
	GLuint vector_dimension_;

	template<typename T>
	void sub_data(const std::vector<T>& buffer, GLuint offset, GLuint nb)
	{
		glBufferSubData(GL_ARRAY_BUFFER, offset*sizeof(T), nb*sizeof(T),buffer.data() );
	}


public:
	inline VBO(const std::vector<float>& buffer, GLuint vec_dim) :
		nb_vectors_(GLuint(buffer.size()/vec_dim)),
		vector_dimension_(vec_dim)
	{
		glGenBuffers(1, &id_);
		init<GLfloat>(buffer);
	}

	template<typename V>
	inline VBO(const std::vector<V>& buffer) :
		nb_vectors_(GLuint(buffer.size())),
		vector_dimension_(sizeof(V)/sizeof(GLfloat))
	{
		glGenBuffers(1, &id_);
		init<V>(buffer);
	}

	inline VBO(GLuint vec_dim) :
		nb_vectors_(0),
		vector_dimension_(vec_dim)
	{
		glGenBuffers(1, &id_);
	}

	VBO(const VBO&) = delete;

	template<typename T>
	void init(const std::vector<T>& buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(buffer.size()*sizeof(T)), buffer.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	inline static std::shared_ptr<VBO> create( GLuint vec_dim)
	{
		return std::make_shared<VBO>(vec_dim);
	}

	inline static SP_VBO create(const std::vector<float>& buffer, GLuint vec_dim)
	{
		return std::make_shared<VBO>(buffer,vec_dim);
	}

	template<typename V>
	static SP_VBO create(const std::vector<V>& buffer)
	{
		return std::make_shared<VBO>(buffer);
	}


	inline ~VBO()
	{
		glDeleteBuffers(1,&id_);
		id_ = 0;
	}

	inline void bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
	}

	inline static void unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	inline void bind_compute(GLuint binding_point)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, id_);
	}

	inline static void unbind_compute(GLuint binding_point)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, 0);
	}

	/**
	 * @brief allocate VBO memory
	 * @param nb_vectors number of vectors
	 */
	inline void allocate(GLuint nb_vect)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(nb_vect*vector_dimension_*sizeof(GLfloat)), nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		nb_vectors_ = nb_vect;
	}

	/**
	 * @brief update
	 * @param buffer source
	 */
	inline void update(const std::vector<GLfloat>& buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		sub_data<GLfloat>(buffer,0,GLuint(buffer.size()));
	}

	inline void update(const std::vector<GLVec2>& buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		sub_data<GLVec2>(buffer,0,GLuint(buffer.size()));
	}

	inline void update(const std::vector<GLVec3>& buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		sub_data<GLVec3>(buffer,0,GLuint(buffer.size()));
	}

	inline void update(const std::vector<GLVec4>& buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		sub_data<GLVec4>(buffer,0,GLuint(buffer.size()));
	}


	/**
	 * @brief update_sub
	 * @param offset offset in vec
	 * @param buffer source
	 * @param nb vec if not all buffer
	 */
	inline void update_sub(GLuint offset, const std::vector<GLfloat>& buffer, GLuint nb=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		sub_data<GLfloat>(buffer,offset,nb?nb:GLuint(buffer.size()));
	}

	inline void update_sub(GLuint offset, const std::vector<GLVec2>& buffer, GLuint nb=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		sub_data<GLVec2>(buffer,offset,nb?nb:GLuint(buffer.size()));
	}

	inline void update_sub(GLuint offset, const std::vector<GLVec3>& buffer, GLuint nb=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		sub_data<GLVec3>(buffer,offset,nb?nb:GLuint(buffer.size()));
	}

	inline void update_sub(GLuint offset, const std::vector<GLVec4>& buffer, GLuint nb=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		sub_data<GLVec4>(buffer,offset,nb?nb:GLuint(buffer.size()));
	}

	/**
	 * @brief dimension of vectors stored in buffer
	 */
	inline GLuint vector_dimension() const
	{
		return vector_dimension_;
	}

	inline GLint length() const
	{
		return GLint(nb_vectors_);
	}

	inline GLuint id() const
	{
		return id_;
	}
};

} // namespace
#endif // EASY_CPP_OGL_VBO_H_
