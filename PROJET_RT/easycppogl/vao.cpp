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

#include <vao.h>

namespace EZCOGL
{

std::shared_ptr<VAO> VAO::none_ = nullptr;

std::shared_ptr<VAO> VAO::none()
{
	if (none_ == nullptr)
		none_ = std::make_shared<VAO>(std::vector<std::tuple<GLint,std::shared_ptr<VBO>>>());
	return none_;
}

void VAO::bind_none()
{
	VAO::none()->bind();
}
//void VAO::add_vbos(const std::vector<std::tuple<GLint,std::shared_ptr<VBO>>>& att_vbo)
//{
//	if (att_vbo.empty())
//	{
//		nb_ = 0u;
//		return;
//	}

//	nb_ = GLuint(std::get<1>(att_vbo[0])->length());
//	glBindVertexArray(id_);
//	for (const auto& p: att_vbo)
//	{
//		glBindBuffer(GL_ARRAY_BUFFER, std::get<1>(p)->id());
//		GLuint vid = GLuint(std::get<0>(p));
//		glEnableVertexAttribArray(vid);
//		glVertexAttribPointer(vid, GLint(std::get<1>(p)->vector_dimension()), GL_FLOAT, GL_FALSE, 0, nullptr);
//	}
//	glBindVertexArray(0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//}

VAO::VAO(const std::vector<std::tuple<GLint,std::shared_ptr<VBO>>>& att_vbo)
{
	glGenVertexArrays(1, &id_);
	if (att_vbo.empty())
	{
		nb_ = 0u;
		return;
	}

	nb_ = GLuint(std::get<1>(att_vbo[0])->length());
	glBindVertexArray(id_);
	for (const auto& p: att_vbo)
	{
		glBindBuffer(GL_ARRAY_BUFFER, std::get<1>(p)->id());
		GLuint vid = GLuint(std::get<0>(p));
		glEnableVertexAttribArray(vid);
		glVertexAttribPointer(vid, GLint(std::get<1>(p)->vector_dimension()), GL_FLOAT, GL_FALSE, 0, nullptr);
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VAO::VAO(const std::vector<std::tuple<GLint,std::shared_ptr<VBO>,GLint>>& att_vbo)
{
	glGenVertexArrays(1, &id_);
	if (att_vbo.empty())
	{
		nb_ = 0u;
		return;
	}

	nb_ = GLuint(std::get<1>(att_vbo[0])->length());
	glBindVertexArray(id_);
	for (const auto& p: att_vbo)
	{
		glBindBuffer(GL_ARRAY_BUFFER, std::get<1>(p)->id());
		GLuint vid = GLuint(std::get<0>(p));
		glEnableVertexAttribArray(vid);
		glVertexAttribPointer(vid, GLint(std::get<1>(p)->vector_dimension()), GL_FLOAT, GL_FALSE, 0, nullptr);
		glVertexAttribDivisor(vid, std::get<2>(p));
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//VAO::VAO(const std::vector<std::tuple<GLint,std::shared_ptr<VBO>,GLint,GLint,GLint>>& att_vbo)
//{
//	glGenVertexArrays(1, &id_);
//	if (att_vbo.empty())
//	{
//		nb_ = 0u;
//		return;
//	}

//	nb_ = GLuint(std::get<1>(att_vbo[0])->length());
//	glBindVertexArray(id_);
//	for (const auto& p: att_vbo)
//	{
//		glBindBuffer(GL_ARRAY_BUFFER, std::get<1>(p)->id());
//		GLuint vid = GLuint(std::get<0>(p));
//		glEnableVertexAttribArray(vid);
//		auto vd = GLint(std::get<1>(p)->vector_dimension());
//		glVertexAttribPointer(vid, vd, GL_FLOAT, GL_FALSE, std::get<3>(p)*sizeof(float), reinterpret_cast<void*>(std::get<4>(p)*sizeof(float)));
//		glVertexAttribDivisor(vid, std::get<2>(p));
//	}
//	glBindVertexArray(0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//}


VAO::VAO(const std::vector<std::tuple<GLint,std::shared_ptr<VBO>,GLint,GLint,GLint,GLint>>& att_vbo)
{
	glGenVertexArrays(1, &id_);
	if (att_vbo.empty())
	{
		nb_ = 0u;
		return;
	}

	nb_ = GLuint(std::get<1>(att_vbo[0])->length());
	glBindVertexArray(id_);
	for (const auto& p: att_vbo)
	{
		glBindBuffer(GL_ARRAY_BUFFER, std::get<1>(p)->id());
		GLuint vid = GLuint(std::get<0>(p));
		glEnableVertexAttribArray(vid);
		glVertexAttribPointer(vid, GLuint(std::get<2>(p)), GL_FLOAT, GL_FALSE, std::get<3>(p)*sizeof(float), reinterpret_cast<void*>(std::get<4>(p)*sizeof(float)));
		glVertexAttribDivisor(vid, std::get<5>(p));
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}





}
