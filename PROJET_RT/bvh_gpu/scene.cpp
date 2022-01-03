#include "scene.h"

void ScenePrimitives::clear()
{
	buffer_.clear();
	buffer_.shrink_to_fit();
}

void SceneMesh::clear()
{
}

int ScenePrimitives::nb() const
{
	return int(buffer_.size());
}

GLVec3 ScenePrimitives::prim_bb(int p, BB& bb) const
{
	bb.min_ = GLVec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	bb.max_ = GLVec3(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

	for (uint32_t v = 0; v < 8; ++v)
	{
		float x = float(v & 1) * 2.01f - 1.005f;
		float y = float(v >> 1 & 1) * 2.01f - 1.005f;
		float z = float(v >> 2 & 1) * 2.01f - 1.005f;

		if (buffer_[p].type_.x() == 5.0f)
			z /= std::abs(z) * 1000.0f;

		GLVec4 B = buffer_[p].transfo_ * GLVec4(x, y, z, 1.0f);
		for (int i = 0; i < 3; ++i)
		{
			if (B[i] < bb.min_[i])
				bb.min_[i] = B[i];
			if (B[i] > bb.max_[i])
				bb.max_[i] = B[i];
		}
	}
	return (bb.min_ + bb.max_) / 2.0f;
}

int ScenePrimitives::add_prim(int prim, const GLMat4& trf, const Material& mat, float area)
{
	int i = int(buffer_.size());
//	GLMat4 tr = (prim == 5) ? trf * Transfo::scale(1, 1, 0.01f) : trf;
	buffer_.push_back({ trf,trf.inverse(),trf,GLVec4(float(prim),0,0,0),mat.color_, GLVec4(mat.shininess_,mat.roughness_,mat.emissivity_,area),GLVec4(0,0,0,0) });

	dbg_buf_meshes_.push_back(nullptr);
	dbg_buf_mline_.push_back(-1);
	return i;
}


int ScenePrimitives::add_mesh(const Mesh& mesh, int mesh_line, const GLMat4& trf, const Material& mat)
{
	int i = int(buffer_.size());
//	GLVec4 col(mat.color_[0], mat.color_[1], mat.color_[2], mat.opacity_);
	const auto& bb = mesh.BB();
	GLMat4 tr_BB = trf * bb.matrix();
	buffer_.push_back({ tr_BB, trf.inverse(), trf, GLVec4(0,float(mesh_line),0,0), mat.color_, GLVec4(mat.shininess_,mat.roughness_,mat.emissivity_,0.0), GLVec4(0,0,0,0) });

	dbg_buf_meshes_.push_back(&mesh);
	dbg_buf_mline_.push_back(mesh_line);
	return i;
}


int ScenePrimitives::sortEmissiveFirst()
{
	auto next_emi = buffer_.begin();
	while ((next_emi != buffer_.end())&&(next_emi->mat_info.z() > 0.0))
		++next_emi;
	auto it = next_emi;
	while (it != buffer_.end())
	{ 
		if (it->mat_info.z() > 0.0)
		{
			PrimData tmp = *next_emi;
			*next_emi++ = *it;
			*it = tmp;
		}
		++it;
	}

	return int(next_emi - buffer_.begin());
}


BB merge(const BB& bb1, const BB& bb2)
{
	BB bb;
	for (int i = 0; i < 3; ++i)
	{
		bb.min_[i] = std::min(bb1.min_[i], bb2.min_[i]);
		bb.max_[i] = std::max(bb1.max_[i], bb2.max_[i]);
	}
	return bb;
}





GLVec3 SceneMesh::prim_bb(int p, BB& bb) const
{
	auto tri = mesh_->triangle_index(p);
	const GLVec3& P0 = mesh_->triangle_vertex0(tri);
	const GLVec3& P1 = mesh_->triangle_vertex1(tri);
	const GLVec3& P2 = mesh_->triangle_vertex2(tri);

	for (int i = 0; i < 3; ++i)
	{
		bb.min_[i] = std::min({ P0[i], P1[i], P2[i] });
		bb.max_[i] = std::max({ P0[i], P1[i], P2[i] });
	}
	bb.min_ -= GLVec3(0.001f, 0.001f, 0.001f);
	bb.max_ += GLVec3(0.001f, 0.001f, 0.001f);

	return (bb.min_ + bb.max_) / 2.0f;
}

int SceneMesh::nb() const
{
	return int(mesh_->nb_triangles());
}

SceneMesh::SceneMesh(SP_Mesh m):
	mesh_(m)
{}



