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


#ifndef EASY_CPP_OGL_TRANSFORM_FEEDBACK_H_
#define EASY_CPP_OGL_TRANSFORM_FEEDBACK_H_

#include<vector>
#include<string>
#include<memory>
#include <shader_program.h>



namespace EZCOGL
{
class TransformFeedback;
using SP_TransformFeedback = std::shared_ptr<TransformFeedback>;

class TransformFeedback
{
	GLuint id_;
	ShaderProgram prg_;

public:
	TransformFeedback(const std::vector<std::pair<GLenum,const std::string&>>& sources, const std::vector<std::string>& outs, const std::string& name);

	TransformFeedback(const TransformFeedback&) = delete;
    TransformFeedback& operator=(const TransformFeedback&) = delete;
	inline ~TransformFeedback() {}

	inline static std::shared_ptr<TransformFeedback> create(const std::vector<std::pair<GLenum,const std::string&>>& sources, const std::vector<std::string>& outs, const std::string& name)
	{
		return std::make_shared<TransformFeedback>(sources,outs,name);
	}

    void start(GLenum prim, std::vector<std::shared_ptr<VBO>> vbos);
	void stop();
};

}// namespace

#endif // EASY_CPP_OGL_TRANSFORM_FEEDBACK_H_
