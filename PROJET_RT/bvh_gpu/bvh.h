#ifndef _BVH_H_
#define _BVH_H_

#include<vector>
#include <gl_eigen.h>
#include "scene.h"



class BVH_KDtree
{
	static const int TEX_WIDTH = 1024;

	GLVVec3 centers_;
	std::vector<BB> bbs_;
	std::vector<int> ids_;
	std::vector<BB> BB_bvh_;
	std::vector<int> ind_bvh_;
	int depth_;

	void split_step(int d, const std::vector<int>& splt, std::vector<int>& splt2);

public:
	inline int depth() const { return depth_; }

	inline int nb_bb() const
	{
		return int(BB_bvh_.size());
	}

	inline int nb_indices() const
	{
		return int(ind_bvh_.size());
	}

	inline const int* data_ind() const
	{
		return ind_bvh_.data();
	}

	inline const BB* data_BB() const
	{
		return BB_bvh_.data();
	}

	//inline int tex_bb_height() const 
	//{
	//	int nbv3 = 2 * BB_bvh_.size();
	//	return nbv3 / TEX_WIDTH + ((nbv3 % TEX_WIDTH != 0) ? 1 : 0);
	//}

	//inline int tex_ind_height() const 
	//{
	//	return ind_bvh_.size() / TEX_WIDTH + ((ind_bvh_.size() % TEX_WIDTH != 0) ? 1 : 0);
	//}

	void init(const Scene& s);

	void compute();
};

using SP_BVH_KDtree = std::shared_ptr<BVH_KDtree>;

#endif

//struct BVH_Data
//{
//	int bb_begins_;
//	int prim_ind_begins_;
//	SP_BVH_KDtree bvh_;
//};
//
//struct Mesh_Data
//{
//	int vert_begins_;
//	int tri_begins_;
//	SP_Mesh mesh_;
//	int ind_bvh_;
//};
//
//class BVH_GPU_Scene
//{
//	static const int TEX_WIDTH = 1024;
//
//	std::vector<BVH_Data> bvh_data_;
//	std::vector<Mesh_Data> mesh_data_;
//	ScenePrimitives& scene_;
//
//	void send_mesh(int m, int bvh_begin_bb, int bvh_begin_ind, int bvh_depth);
//	void send_bvh(int i);
//
//public:
//	SP_Texture2D tex_p_;
//	SP_Texture2D tex_n_;
//	SP_Texture2D tex_tri_;
//
//	SP_Texture2D tex_bb_;
//	SP_Texture2D tex_ind_;
//	SP_Texture2D tex_prim_;
//
//	// Key tri_begin of mesh, data bvh_begin, depth
//	std::map<int, std::pair<int, int>> mesh_bvh_info_;
//
//	BVH_GPU_Scene(ScenePrimitives& sc);
//	
//	int add_bvh(SP_BVH_KDtree b);
//
//	void finalize();
//
//	inline int depth(int i) const
//	{
//		return bvh_data_[i].bvh_->depth();
//	}
//
//	inline int nb_prim() const
//	{
//		return scene_.nb();
//	}
//
//	int add_mesh(SP_Mesh m);
//
//	inline void place_mesh(int i, const GLMat4& trf, const GLVec3& color, float spec)
//	{
//		scene_.add_mesh(*(mesh_data_[i].mesh_), mesh_data_[i].tri_begins_, trf, color, spec);
//	}
//
//	inline void add_sphere(const GLMat4& trf, const GLVec3& color, float spec)
//	{
//		scene_.add_sphere(trf, color, spec);
//	}
//
//	inline void add_cube(const GLMat4& trf, const GLVec3& color, float spec)
//	{
//		scene_.add_cube(trf, color, spec);
//	}
//
//	inline void add_cylinder(const GLMat4& trf, const GLVec3& color, float spec)
//	{
//		scene_.add_cylinder(trf, color, spec);
//	}
//
//	inline void add_cone(const GLMat4& trf, const GLVec3& color, float spec)
//	{
//		scene_.add_cone(trf, color, spec);
//	}
//
//	inline int nb_bb() const
//	{
//		return bvh_data_.size();
//	}
//
//};