#include "gpu_bvh_scene.h"
#include <iostream>




BVH_GPU_Scene::BVH_GPU_Scene(ScenePrimitives& sc):
	scene_(sc)
{
	bvh_data_.push_back({ 0,0,nullptr });
	bvh_data_.push_back({ 0,0,nullptr });
	mesh_data_.push_back({ 0,0,nullptr,-1 });
}


int BVH_GPU_Scene::add_bvh(SP_BVH_KDtree b)
{
	b->compute();

	int nbv3 = 2 * b->nb_bb();
	int n =  nbv3 / TEX_WIDTH + ((nbv3 % TEX_WIDTH != 0) ? 1 : 0);
	int bbb = n;

	n = b->nb_indices() / TEX_WIDTH + ((b->nb_indices() % TEX_WIDTH != 0) ? 1 : 0);
	int pib = n;

	bvh_data_.push_back({ bbb,pib,b });
	return int(bvh_data_.size()) - 1;
}

void BVH_GPU_Scene::send_bvh(int i)
{
	auto& data = bvh_data_[i];

	int nbv3 = data.bvh_->nb_bb() * 2;
	int nbl = nbv3 / TEX_WIDTH;
	int rem = nbv3 % TEX_WIDTH;

	tex_bb_->update(0, data.bb_begins_, TEX_WIDTH, nbl, data.bvh_->data_BB());
	if (rem > 0)
		tex_bb_->update(0, data.bb_begins_+nbl, rem, 1, data.bvh_->data_BB() + TEX_WIDTH / 2 * nbl);

	nbl = data.bvh_->nb_indices() / TEX_WIDTH;
	rem = data.bvh_->nb_indices() % TEX_WIDTH;
	tex_ind_->update(0, data.prim_ind_begins_, TEX_WIDTH, nbl, data.bvh_->data_ind());
	if (rem > 0)
		tex_ind_->update(0, data.prim_ind_begins_+nbl, rem, 1, data.bvh_->data_ind() + TEX_WIDTH / 2 * nbl);
}


int BVH_GPU_Scene::add_mesh(SP_Mesh m)
{
	std::cout <<"Mesh BB : "<< m->BB().min().transpose() << "  /  " << m->BB().max().transpose() << std::endl;

	auto& data = mesh_data_.back();
	auto nb = m->nb_vertices();
	auto vb = nb / TEX_WIDTH + ((nb % TEX_WIDTH != 0) ? 1 : 0);
	vb +=  data.vert_begins_;

	nb = m->nb_triangles();
	auto tb = nb / TEX_WIDTH + ((nb % TEX_WIDTH != 0) ? 1 : 0);
	tb += data.tri_begins_;

	SceneMesh sm(m);
	SP_BVH_KDtree km = std::make_shared<BVH_KDtree>();
	km->init(sm);
	
	data.ind_bvh_ = add_bvh(km);
	data.mesh_ = m;
	mesh_data_.push_back({ int(vb),int(tb),nullptr,-1});

	
	return int(mesh_data_.size() - 2);
}

void BVH_GPU_Scene::clear()
{
	this->bvh_data_.clear();
	this->mesh_data_.clear();
	this->mesh_bvh_info_.clear();
	this->scene_.clear();
	bvh_data_.push_back({ 0,0,nullptr });
	bvh_data_.push_back({ 0,0,nullptr });
	mesh_data_.push_back({ 0,0,nullptr,-1 });
}

void BVH_GPU_Scene::send_mesh(int m, int bvh_begin_bb, int bvh_begin_ind, int bvh_depth)
{
	Mesh_Data& md = mesh_data_[m];
	Mesh& mesh = *(md.mesh_);

	decltype(mesh.tri_indices) buffer;
	buffer.reserve(mesh.tri_indices.size() + 3);
	// info
	buffer.push_back(bvh_begin_bb); // line in BVH_BB
	buffer.push_back(bvh_begin_ind); // line in BVH_INDD
	buffer.push_back(GLuint(pow(2.0, bvh_depth) - 1.0)); // 2^depth - 1
	GLuint d = TEX_WIDTH * md.vert_begins_;
	for (auto i : mesh.tri_indices)
		buffer.push_back(i + d);

	auto nbl = (mesh.nb_triangles()+1) / TEX_WIDTH;
	auto c1 = (mesh.nb_triangles()+1) % TEX_WIDTH;
	if (nbl>0)
		tex_tri_->update(0, md.tri_begins_, TEX_WIDTH, int(nbl), buffer.data());
	if (c1 > 0)
		tex_tri_->update(0, int(md.tri_begins_ + nbl), int(c1), 1, buffer.data() + 3* nbl * TEX_WIDTH);

	nbl = mesh.nb_vertices() / TEX_WIDTH;
	c1 = mesh.nb_vertices() % TEX_WIDTH;
	tex_p_->update(0, md.vert_begins_, TEX_WIDTH, int(nbl), mesh.vertices_.data());
	tex_n_->update(0, md.vert_begins_, TEX_WIDTH, int(nbl), mesh.normals_.data());
	if (c1 > 0)
	{
		tex_p_->update(0, int(md.vert_begins_ + nbl), int(c1), 1, mesh.vertices_.data() + nbl * TEX_WIDTH);
		tex_n_->update(0, int(md.vert_begins_ + nbl), int(c1), 1, mesh.normals_.data() + nbl * TEX_WIDTH);
	}
}


void BVH_GPU_Scene::finalize()
{
	nb_emi_ = scene_.sortEmissiveFirst();
	SP_BVH_KDtree rootk = std::make_shared<BVH_KDtree>();
	rootk->init(scene_);
	rootk->compute();
	auto& data = bvh_data_[1];
	data.bvh_ = rootk;
	int nbv3 = 2 * rootk->nb_bb();
	int n = nbv3 / TEX_WIDTH + ((nbv3 % TEX_WIDTH != 0) ? 1 : 0);
	data.bb_begins_ = n;

	n = rootk->nb_indices() / TEX_WIDTH + ((rootk->nb_indices() % TEX_WIDTH != 0) ? 1 : 0);
	data.prim_ind_begins_ = n;
	//  chnage begins from size to begin
	for (int i = 1; i < bvh_data_.size(); ++i)
	{
		bvh_data_[i].bb_begins_ += bvh_data_[i - 1].bb_begins_;
		bvh_data_[i].prim_ind_begins_ += bvh_data_[i - 1].prim_ind_begins_;
		bvh_data_[i - 1].bvh_ = bvh_data_[i].bvh_;
	}

	tex_ind_ = Texture2D::create();
	tex_ind_->alloc(TEX_WIDTH, bvh_data_.back().prim_ind_begins_, GL_R32I);
	tex_bb_ = Texture2D::create();
	tex_bb_->alloc(TEX_WIDTH, bvh_data_.back().bb_begins_, GL_RGB32F);
	// remove last which contain size
	bvh_data_.pop_back();// save size ?
	//send to GPU
	for (int i = 0; i < bvh_data_.size(); ++i)
		send_bvh(i);

	tex_prim_ = Texture2D::create();
	tex_prim_->alloc(scene_.tex_width(), scene_.tex_height(), GL_RGBA32F);
	n = scene_.nb() / 64;
	int r = scene_.nb() % 64;
	if (n > 0)
		tex_prim_->update(0, 0, TEX_WIDTH, n, scene_.prim_data());
	if (r > 0)
		tex_prim_->update(0, n, r * 16, 1, scene_.prim_data() + n * 64);

	auto md = mesh_data_.back();
	tex_tri_ = Texture2D::create();
	tex_tri_->alloc(TEX_WIDTH, md.tri_begins_, GL_RGB32I);
	tex_p_ = Texture2D::create();
	tex_p_->alloc(TEX_WIDTH, md.vert_begins_, GL_RGB32F);
	tex_n_ = Texture2D::create();
	tex_n_->alloc(TEX_WIDTH, md.vert_begins_, GL_RGB32F);

	int nbm = int(mesh_data_.size()) - 1;
	for (int i = 0; i < nbm; ++i)
	{
		const auto& data = bvh_data_[i + 1];
		send_mesh(i, data.bb_begins_, data.prim_ind_begins_, data.bvh_->depth());
	}

	for (auto& data : mesh_data_)
	{
		int j = data.ind_bvh_-1;
		if (j >= 0)
		{
			int b = bvh_data_[j].bb_begins_;
			int d = bvh_data_[j].bvh_->depth();
			mesh_bvh_info_.insert(std::make_pair(data.tri_begins_, std::make_pair(b, d)));
		}
	}
}
