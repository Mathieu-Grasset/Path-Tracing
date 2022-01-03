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


#ifndef EASY_CPP_OGL_FBO_H_
#define EASY_CPP_OGL_FBO_H_

#include <GL/gl3w.h>
#include <vector>
#include <array>
#include <texture2d.h>

// TODO compatible with TEXUTRE_CUBEMAP

namespace EZCOGL
{
class FBO;
using SP_FBO = std::shared_ptr<FBO>;


class FBO
{
public:
	FBO(const std::vector<SP_Texture2D>& textures);

	static inline SP_FBO create(const std::vector<SP_Texture2D>& textures)
	{
		return std::make_shared<FBO>(textures);
	}

	virtual ~FBO();

	FBO(const FBO&) = delete;

	void bind();

	static void push();

	static void pop();

	virtual void resize(int w, int h);

	inline GLint width() const { return tex_.front()->width(); }

	inline GLint height() const { return tex_.front()->height(); }

	inline SP_Texture2D texture(std::size_t i) { return tex_[i]; }

	inline std::size_t nb_textures() { return tex_.size(); }

	inline virtual SP_Texture2D depth_texture() { return nullptr; }

protected:
	GLuint id_;
	std::vector<SP_Texture2D> tex_;
	std::vector<GLenum> attach_;
	static std::vector<std::array<GLint, 5>> stack_viewport_fbo_;
};


class FBO_Depth : public FBO
{
protected:
public:
	FBO_Depth(const std::vector<std::shared_ptr<Texture2D>>& textures, std::shared_ptr<FBO_Depth> from = nullptr);
	static inline std::shared_ptr<FBO_Depth> create(const std::vector<std::shared_ptr<Texture2D>>& textures, std::shared_ptr<FBO_Depth> from = nullptr)
	{
		return std::make_shared<FBO_Depth>(textures,from);
	}

	void resize(int w, int h) override;

protected:
	GLuint depth_render_buffer_;
};

class FBO_DepthTexture : public FBO
{
protected:
public:
	FBO_DepthTexture(const std::vector<std::shared_ptr<Texture2D>>& textures, std::shared_ptr<Texture2D> from = nullptr);

	FBO_DepthTexture(const std::vector<std::shared_ptr<Texture2D>>& textures, std::shared_ptr<FBO_DepthTexture> from = nullptr);


	static inline std::shared_ptr<FBO_DepthTexture> create(const std::vector<std::shared_ptr<Texture2D>>& textures, std::shared_ptr<Texture2D> from = nullptr)
	{
		return std::make_shared<FBO_DepthTexture>(textures,from);
	}

	void resize(int w, int h) override;

	inline std::shared_ptr<Texture2D> depth_texture() override { return depth_tex_; }

protected:
	std::shared_ptr<Texture2D> depth_tex_;
};

}
#endif
