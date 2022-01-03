#ifndef _GPU_BVH_SCENE_H_
#define _GPU_BVH_SCENE_H_
#include "bvh.h"


class BVH_GPU
{
public:
	static const int uniform_nb_prims = 1;
	static const int uniform_bvh_depth = 2;
	static const int uniform_flat_face = 3;
	static const int uniform_nbp = 4;
	static const int uniform_date = 5;
	static const int uniform_invV = 6;
	static const int uniform_invPV = 10;
};



struct BVH_Data
{
	int bb_begins_;
	int prim_ind_begins_;
	SP_BVH_KDtree bvh_;
};

struct Mesh_Data
{
	int vert_begins_;
	int tri_begins_;
	SP_Mesh mesh_;
	int ind_bvh_;
};

class BVH_GPU_Scene
{
	static const int TEX_WIDTH = 1024;

	std::vector<BVH_Data> bvh_data_;
	std::vector<Mesh_Data> mesh_data_;

	int  nb_emi_;
	ScenePrimitives& scene_;

	void send_mesh(int m, int bvh_begin_bb, int bvh_begin_ind, int bvh_depth);
	void send_bvh(int i);

public:
	SP_Texture2D tex_p_;
	SP_Texture2D tex_n_;
	SP_Texture2D tex_tri_;

	SP_Texture2D tex_bb_;
	SP_Texture2D tex_ind_;
	SP_Texture2D tex_prim_;

	// Key tri_begin of mesh, data bvh_begin, depth
	std::map<int, std::pair<int, int>> mesh_bvh_info_;

	BVH_GPU_Scene(ScenePrimitives& sc);

	
	void clear();
	
	int add_bvh(SP_BVH_KDtree b);

	void finalize();

	inline int depth(int i) const
	{
		return bvh_data_[i].bvh_->depth();
	}

	inline int nb_prim() const
	{
		return scene_.nb();
	}

	int add_mesh(SP_Mesh m);

	inline void place_mesh(int i, const GLMat4& trf, const Material& mat)
	{
		scene_.add_mesh(*(mesh_data_[i].mesh_), mesh_data_[i].tri_begins_, trf, mat);
	}

	inline void add_sphere(const GLMat4& trf, const Material& mat)
	{
		scene_.add_sphere(trf, mat);
	}

	inline void add_cube(const GLMat4& trf, const Material& mat)
	{
		scene_.add_cube(trf, mat);
	}

	inline void add_cylinder(const GLMat4& trf, const Material& mat)
	{
		scene_.add_cylinder(trf, mat);
	}

	inline void add_cone(const GLMat4& trf, const Material& mat)
	{
		scene_.add_cone(trf, mat);
	}

	inline void add_orientedQuad(const GLMat4& trf, const Material& mat)
	{
		scene_.add_orientedQuad(trf,mat);
	}

	inline int nb_bb() const
	{
		return int(bvh_data_.size());
	}

	inline int nb_emissives() const
	{
		return nb_emi_;
	}

};

#endif
