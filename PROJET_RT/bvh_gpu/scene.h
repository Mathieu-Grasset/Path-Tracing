#ifndef SCENE_H_
#define SCENE_H_

#include <gl_eigen.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <texture2d.h>

#include <mesh.h>


using namespace EZCOGL;

struct BB
{
	GLVec3 min_;
	GLVec3 max_;
};


class Scene
{
public:
	virtual GLVec3 prim_bb(int p, BB& bb) const = 0;
	virtual int nb() const = 0;
	virtual void clear() = 0;
};

class Material
{
public:
	GLVec4 color_;
	float shininess_;
	float roughness_;
	float emissivity_;

	inline Material(const GLVec4& color, float shin , float rough, float emi) :
		color_(color), shininess_(shin), roughness_(rough), emissivity_(emi) {}

	inline Material(const GLVec4& color, float shin, float rough) :
		color_(color), shininess_(shin), roughness_(rough), emissivity_(0) {}

	inline Material(const GLVec4& color) :
		color_(color), shininess_(0.0f), roughness_(0.0f), emissivity_(0.0f) {}


	static inline Material light(const GLVec4& col, float emi) { return Material(col, 0.0f, 0.0f,emi); }
};

class SceneMesh : public Scene
{
	SP_Mesh mesh_;
public:
	SceneMesh(SP_Mesh m);

	GLVec3 prim_bb(int p, BB& bb) const ;

	int nb() const override;

	void clear() override;
};

struct PrimData
{
	GLMat4 transfo_;
	GLMat4 inv_transfo_;
	GLMat4 inv_mesh_bb_transfo_;
	GLVec4 type_;
	GLVec4 color_; // RGBA
	GLVec4 mat_info; // r = shininesss, g= roughhness, b = emissivty
	GLVec4 padding2_; // scale mesh BB
};

class ScenePrimitives: public Scene
{
	static const int TEX_WIDTH = 1204; // : 64 * 16 (Mat4 +Mat4 + 4)

	std::vector <PrimData> buffer_;

	int add_prim(int prim, const GLMat4& trf, const Material& mat, float area);


public:

	void clear() override;

	inline const PrimData* prim_data() const
	{
		return buffer_.data();
	}

	inline int tex_width() const { return TEX_WIDTH; }

	inline int tex_height() const
	{
		int nb = int(buffer_.size());
		return nb / 64 + ((nb % 64 != 0) ? 1 : 0);
	}

	int nb() const override;

	int sortEmissiveFirst();

	inline const GLMat4& transfo(int i) const
	{
		return buffer_[i].transfo_;
	}

	inline const GLMat4& transfo_mesh_bb(int i) const
	{
		return buffer_[i].inv_mesh_bb_transfo_;
	}

	inline const GLVec3 color(int i) const
	{
		const auto& C = buffer_[i].color_;
		return GLVec3(C[0], C[1], C[2]);
	}

	inline int type(int i) const
	{
		return int(buffer_[i].type_[0]);
	}

	int add_mesh(const Mesh& mesh, int mesh_line, const GLMat4& trf, const Material& mat);

	inline int add_sphere(const GLMat4& trf, const Material& mat)
	{
		float r = trf.block<3, 1>(0, 0).norm(); // scale uniforme !
		float area = float(2.0*M_PI) * r * r; // la sphere se projete comme un disque ?
		return add_prim(1, trf, mat, area);
	}

	inline int add_cube(const GLMat4& trf, const Material& mat)
	{
		GLVec3 U = Transfo::apply(trf, GLVec3(1, -1, -1)) - Transfo::apply(trf, GLVec3(-1, -1, -1));
		GLVec3 V = Transfo::apply(trf, GLVec3(-1, 1, -1)) - Transfo::apply(trf, GLVec3(-1, -1, -1));
		GLVec3 W = Transfo::apply(trf, GLVec3(-1, -1, 1)) - Transfo::apply(trf, GLVec3(-1, -1, -1));
		float area = 2.0f * ((U.cross(V)).norm() + (U.cross(W)).norm() + (W.cross(V)).norm());
		return add_prim(2, trf, mat,area);
	}

	inline int add_cylinder(const GLMat4& trf, const Material& mat)
	{
		GLVec3 U = Transfo::apply(trf, GLVec3(1, -1, -1)) - Transfo::apply(trf, GLVec3(-1, -1, -1));
		GLVec3 V = Transfo::apply(trf, GLVec3(-1, 1, -1)) - Transfo::apply(trf, GLVec3(-1, -1, -1));
		GLVec3 W = Transfo::apply(trf, GLVec3(-1, -1, 1)) - Transfo::apply(trf, GLVec3(-1, -1, -1));

		float area = (U.dot(U) + V.dot(V))/4.0f*sqrtf(2.0f)*float(M_PI)*W.norm();
		return add_prim(3, trf, mat,area);
	}

	inline int add_cone(const GLMat4& trf, const Material& mat)
	{
		GLVec3 U = Transfo::apply(trf, GLVec3(1, -1, -1)) - Transfo::apply(trf, GLVec3(-1, -1, -1));
		GLVec3 V = Transfo::apply(trf, GLVec3(-1, 1, -1)) - Transfo::apply(trf, GLVec3(-1, -1, -1));
		GLVec3 W = Transfo::apply(trf, GLVec3(-1, -1, 1)) - Transfo::apply(trf, GLVec3(-1, -1, -1));

		//TODO
		float area = 0.0f;
		return add_prim(4, trf, mat,area);
	}


	inline int add_orientedQuad(const GLMat4& trf, const Material& mat)
	{
		GLVec3 U = Transfo::apply(trf, GLVec3(1, -1, 0)) - Transfo::apply(trf, GLVec3(-1, -1, 0));
		GLVec3 V = Transfo::apply(trf, GLVec3(-1, 1, 0)) - Transfo::apply(trf, GLVec3(-1, -1, 0));
		float area = (U.cross(V)).norm();
		return add_prim(5, trf, mat, area);
	}


	GLVec3 prim_bb(int p, BB& bb) const;

	inline int mesh_line(int i) const
	{
		return int(buffer_[i].type_.y());
	}

	std::vector<const Mesh*> dbg_buf_meshes_;
	std::vector<int> dbg_buf_mline_;

};


BB merge(const BB& bb1, const BB& bb2);


#endif