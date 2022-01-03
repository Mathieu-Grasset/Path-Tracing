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

#include <mesh.h>
#include <gl_eigen.h>
#include <iostream>
#include <array>
#include <thread>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include <cassert>
#include <condition_variable>

class Barrier
{

public:

	Barrier(std::size_t nb_threads)
		: m_mutex(),
		m_condition(),
		m_nb_threads(nb_threads)
	{
		assert(0u != m_nb_threads);
	}

	Barrier(const Barrier& barrier) = delete;

	Barrier(Barrier&& barrier) = delete;

	~Barrier() noexcept
	{
		assert(0u == m_nb_threads);
	}

	Barrier& operator=(const Barrier& barrier) = delete;

	Barrier& operator=(Barrier&& barrier) = delete;

	void Wait()
	{
		std::unique_lock< std::mutex > lock(m_mutex);

		assert(0u != m_nb_threads);

		if (0u == --m_nb_threads)
		{
			m_condition.notify_all();
		}
		else
		{
			m_condition.wait(lock, [this]() { return 0u == m_nb_threads; });
		}
	}

private:

	std::mutex m_mutex;

	std::condition_variable m_condition;

	std::size_t m_nb_threads;
};

#pragma warning( disable : 4244 4245 4018)

namespace EZCOGL
{

const float M_PIF = float(M_PI);

void BoundingBox::direct_add_point(const GLVec3& P)
{
    min_[0] = std::min(min_[0],P[0]);
    min_[1] = std::min(min_[1],P[1]);
    min_[2] = std::min(min_[2],P[2]);
    max_[0] = std::max(max_[0],P[0]);
    max_[1] = std::max(max_[1],P[1]);
    max_[2] = std::max(max_[2],P[2]);
}

void BoundingBox::add_point(const GLVec3& P)
{
	if (!initialized_)
	{
		min_ = P;
		max_ = P;
		initialized_ = true;
	}
	else
		direct_add_point(P);
}


Mesh::Mesh(Mesh&& m):
	vertices_(m.vertices_),
	normals_(m.normals_),
	tex_coords_(m.tex_coords_),
	tri_indices(m.tri_indices),
	line_indices(m.line_indices),
	bb_(m.bb_)
{}

void Mesh::compute_normals()
{
	normals_.resize(vertices_.size(),GLVec3(0,0,0));

	auto nbt = tri_indices.size()/3u;
	for (auto i=0u; i<nbt; ++i)
	{
		const GLVec3& A = vertices_[tri_indices[3*i]];
		const GLVec3& B = vertices_[tri_indices[3*i+1]];
		const GLVec3& C = vertices_[tri_indices[3*i+2]];
		normals_[tri_indices[3*i]] += (B-A).cross(C-A);
		normals_[tri_indices[3*i+1]] += (C-B).cross(A-B);
		normals_[tri_indices[3*i+2]] += (A-C).cross(B-C);
	}

	for (auto& n: normals_)
		n.normalize();


//	static const int NBTH=16;
//	std::vector<std::thread*> threads;
//	threads.reserve(NBTH);

//	for (int i=0; i<NBTH; ++i)
//	{
//		threads.push_back(new std::thread([this,i] ()
//		{
//			for (int j=i; j<normals_.size(); j+=NBTH)
//			{
//				normals_[j].normalize();
//			}
//		}));
//	}

//	for (auto* t : threads)
//	{
//		t->join();
//		delete t;
//	}
}

SP_MeshRenderer Mesh::renderer(GLint att_pos, GLint att_norm, GLint att_tc, GLint att_col) const
{
    return std::make_shared<MeshRenderer>(*this,att_pos,att_norm,att_tc,att_col);
}

MeshRenderer::MeshRenderer(const Mesh& m, GLint att_pos, GLint att_norm, GLint att_tc, GLint att_col):
	bb_(m.bb_)
{
    std::vector<std::tuple<GLint,std::shared_ptr<VBO>>> params;
	if( att_pos>0)
	{
		auto vbop = VBO::create(m.vertices_);
		params.emplace_back(att_pos,vbop);
	}
	if( att_norm>0)
	{
		auto vbon = VBO::create(m.normals_);
		params.emplace_back(att_norm,vbon);
	}
	if( att_tc>0)
	{
		auto vbot = VBO::create(m.tex_coords_);
		params.emplace_back(att_tc,vbot);
	}

	if( att_col>0)
	{
		auto vboc = VBO::create(m.colors_);
		params.emplace_back(att_col,vboc);
	}


	vao_ = new VAO(params);
	ebo_triangles_.init(m.tri_indices);
	ebo_lines_.init(m.line_indices);
}


MeshRenderer::~MeshRenderer()
{
	delete vao_;
}

void MeshRenderer::draw(GLenum prim)
{
	vao_->bind();
	switch (prim)
	{
		case GL_POINTS:
			if (vao_->length()>0)
				glDrawArrays(GL_POINTS, 0, vao_->length());
			break;
		case GL_LINES:
			if (ebo_lines_.length()>0)
			{
				ebo_lines_.bind();
				glDrawElements(GL_LINES, ebo_lines_.length(), GL_UNSIGNED_INT, nullptr);
			}
			break;
		case GL_TRIANGLES:
			if (ebo_triangles_.length()>0)
			{
				ebo_triangles_.bind();
				glDrawElements(GL_TRIANGLES, ebo_triangles_.length(), GL_UNSIGNED_INT, nullptr);
			}
			break;
	}
}


SP_Mesh Mesh::CubePosOnly()
{
	SP_Mesh m = std::make_shared<Mesh>();
	float V=1.0;
	float v=-1.0;

	m->bb_.add_point(GLVec3(v,v,v));
	m->bb_.add_point(GLVec3(V,V,V));

	m->vertices_ = GLVVec3{{v,v,v}, {V,v,v}, {V,V,v}, {v,V,v}, {v,v,V}, {V,v,V}, {V,V,V}, {v,V,V}};
    m->tri_indices = std::vector<GLuint>{2,1,0,3,2,0, 4,5,6,4,6,7, 0,1,5,0,5,4, 1,2,6,1,6,5, 2,3,7,2,7,6, 3,0,4,3,4,7};
    m->line_indices = std::vector<GLuint>{0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4,0,4,1,5,2,6,3,7};

	return m;
}

SP_Mesh Mesh::Cube()
{
	SP_Mesh m = std::make_shared<Mesh>();

	float V=1.0;
	float v=-1.0;

	m->bb_.add_point(GLVec3(v,v,v));
	m->bb_.add_point(GLVec3(V,V,V));

	m->vertices_ = GLVVec3{
			{v,v,v}, {V,v,v}, {V,V,v}, {v,V,v},
			{v,v,V}, {V,v,V}, {V,V,V}, {v,V,V},
			{v,v,V}, {v,v,v}, {v,V,v}, {v,V,V},
			{V,v,V}, {V,v,v}, {V,V,v}, {V,V,V},
			{v,v,V}, {V,v,V}, {V,v,v}, {v,v,v},
			{v,V,V}, {V,V,V}, {V,V,v}, {v,V,v}};

	m->normals_ = GLVVec3{
			{0,0,-1}, {0,0,-1},{0,0,-1},{0,0,-1},
			{0,0,1}, {0,0,1},{0,0,1},{0,0,1},
			{-1,0,0}, {-1,0,0}, {-1,0,0}, {-1,0,0},
			{1,0,0}, {1,0,0}, {1,0,0}, {1,0,0},
			{0,-1,0}, {0,-1,0}, {0,-1,0}, {0,-1,0},
			{0,1,0}, {0,1,0}, {0,1,0}, {0,1,0}};


	m->tri_indices = std::vector<GLuint>{0,3,2,0,2,1, 4,5,6,4,6,7, 8,11,10,8,10,9, 12,13,14,12,14,15, 16,19,18,16,18,17, 20,21,22,20,22,23};
	m->line_indices = std::vector<GLuint>{0,1,1,2,2,3,3,0, 4,5,5,6,6,7,7,4, 0,4,1,5,2,6,3,7};

	return m;
}

void Mesh::grid_topo(GLint m, GLint n)
{
	this->tri_indices.reserve(6*(n-1)*(m-1));
	auto push_quad = [&] (GLuint k)
	{
		tri_indices.push_back(k);
		tri_indices.push_back(k-n-1);
		tri_indices.push_back(k-n);

		tri_indices.push_back(k-n-1);
		tri_indices.push_back(k);
		tri_indices.push_back(k-1);
	};

	for(GLint j=1;j<m;++j)
		for(GLint i=1;i<n;++i)
			push_quad(GLuint(j*n+i));

	this->line_indices.reserve(2*m*(n-1)+2*n*(m-1));

	for(GLint j=0;j<m;++j)
		for(GLint i=1;i<n;++i)
		{
			GLint k =j*n+i;
			line_indices.push_back(k);
			line_indices.push_back(k-1);
		}

	for(GLint j=1;j<m;++j)
		for(GLint i=0;i<n;++i)
		{
			GLint k =j*n+i;
			line_indices.push_back(k);
			line_indices.push_back(k-n);
		}
}

SP_Mesh Mesh::Grid(GLint m, GLint n)
{
	SP_Mesh grid = std::make_shared<Mesh>();

    grid->grid_topo(m,n);

    GLint n1 = n - 1;
    GLint m1 = m - 1;
    if (m>n)
    {
        grid->bb_.add_point({-float(m)/n,-1,-0.01f});
        grid->bb_.add_point({float(m)/n,1,0.01f});
    }
    else
    {
        grid->bb_.add_point({-1,-float(n)/m,-0.01f});
        grid->bb_.add_point({1,float(n)/m,0.01f});
    }

    for(int j=0;j<m;++j)
    {
        float v = (1.0f/m1)*j;
        for(int i=0;i<n;++i)
        {
            float u = (1.0f/n1)*i;

            grid->tex_coords_.push_back(GLVec2(u,v));
            grid->vertices_.push_back(GLVec3(grid->bb_.max().x()*(u-0.5f)*2,grid->bb_.max().y()*(v-0.5f)*2,0.0f));
            grid->normals_.push_back(GLVec3(0,0,1));
        }
    }
    return grid;
}

SP_Mesh Mesh::Wave(GLint n)
{
	SP_Mesh wave = std::make_shared<Mesh>();
    wave->grid_topo(n,n);

    GLint n1 = n - 1;
    wave->bb_.add_point({-1,-1,-0.01f});
    wave->bb_.add_point({1,1,0.01f});

    for(int j=0;j<n;++j)
    {
        float v = (1.0f/n1)*j;
        for(int i=0;i<n;++i)
        {
            float u = (1.0f/n1)*i;
            float x = (u-0.5f)*2;
            float y = (v-0.5f)*2;
            float r = std::sqrt(x*x+y*y);
			float h = 0.2f*(1.0f-r/2.0f)*std::sin(float(M_PI)/2+r*8);
            wave->tex_coords_.push_back(GLVec2(u,v));
            wave->vertices_.push_back(GLVec3(x,y,h));
			float dh = -0.2f/2*std::sin(float(M_PI)/2+r*8) +
					0.2f*(1.0f-r/2)*8*std::cos(float(M_PI)/2+r*8);
            GLVec3 n(-x/r*dh,-y/r*dh,1);
            n.normalize();
            wave->normals_.push_back(n);
        }
    }
    return wave;
}

SP_Mesh Mesh::Cylinder(GLint m, GLint n, float radius)
{
	SP_Mesh cylinder = std::make_shared<Mesh>();
    cylinder->grid_topo(m,n);

    GLint n1 = n - 1;
    GLint m1 = m - 1;

    cylinder->vertices_.reserve(m*n);
    cylinder->normals_.reserve(cylinder->vertices_.size());
    cylinder->tex_coords_.reserve(cylinder->vertices_.size());

    GLVVec3 cpos;// = create_Vec_buffer(3,n);
    cpos.reserve(n);
    GLVVec3 cnorm;//	let cnorm = create_Vec_buffer(3,n);
    cnorm.reserve(n);
    for(int i=0;i<n;++i)
    {
        double alpha = ((1.0/n1)*i)*2*M_PI;
        GLVec3 p(std::sin(alpha),std::cos(alpha),0);
        cnorm.push_back(p);
        cpos.push_back(p*radius);
    }
    for(int j=0;j<m;++j)
    {
        GLMat4 tr = Transfo::translate(GLVec3(0,0,-1.0f+2.0f/m1*j));
        GLMat3 ntr = Transfo::sub33(tr); // no need to inverse_transpose because no scale
        double v = (1.0/n1)*j;
        for(int i=0;i<n;++i)
        {
            double u = (1.0/n1)*i;
            cylinder->tex_coords_.push_back(GLVec2(u,v));
            cylinder->vertices_.push_back(Transfo::apply(tr,cpos[i]));
            cylinder->normals_.push_back(Transfo::apply(ntr,cnorm[i]));
        }
    }

    cylinder->bb_.add_point({-radius,-radius,-1});
    cylinder->bb_.add_point({ radius, radius,1});

    return cylinder;
}


SP_Mesh Mesh::Sphere( GLint n)
{
	SP_Mesh sphere = std::make_shared<Mesh>();
	sphere->grid_topo(n,n);

	GLint n1 = n - 1;

	sphere->vertices_.reserve(n*n);
	sphere->normals_.reserve(sphere->vertices_.size());
	sphere->tex_coords_.reserve(sphere->vertices_.size());

	for(int j=0;j<n;++j)
	{
		double v = (1.0/n1)*j;
		double beta = ((1.0/n1)*j)*M_PI+M_PI/2;
		float r = std::cos(beta);
		float h = std::sin(beta);
		for(int i=0;i<n;++i)
		{
			double u = (1.0/n1)*i;
			double alpha = ((1.0/n1)*i)*2*M_PI;
			GLVec3 p(r*std::sin(alpha),r*std::cos(alpha),h);
			p.normalize();
			sphere->vertices_.push_back(p);
			sphere->normals_.push_back(p);
			sphere->tex_coords_.push_back(GLVec2(u,v));
		}
	}

	sphere->bb_.add_point({-1,-1,-1});
	sphere->bb_.add_point({ 1,1,1});

	return sphere;
}



SP_Mesh Mesh::ClosedCylinder(int sides, float radius_ratio)
{
	SP_Mesh Cyl = std::make_shared<Mesh>(); 

	int sides2 = 2*sides;
	int sides3 = 3*sides;

	Cyl->vertices_.push_back(GLVec3(0, 0, 1));
	Cyl->normals_.push_back(GLVec3(0, 0, 1));

	float a = float(M_PI/4.0);
	float b = float(2.0*M_PI)/sides;
	for (int i=0;i<sides;++i)
	{
		GLVec3 P = GLVec3(radius_ratio*std::cos(a),radius_ratio*std::sin(a),1);
		Cyl->vertices_.push_back(P);
		Cyl->normals_.push_back(GLVec3(0,0,1));
		a+=b;
	}

	Cyl->vertices_.push_back(GLVec3(0, 0, -1));
	Cyl->normals_.push_back(GLVec3(0, 0, -1));

	for (int i=1;i<=sides;++i)
	{
		GLVec3 P = Cyl->vertices_[i];
		P.z() = -1;
		Cyl->vertices_.push_back(P);
		Cyl->normals_.push_back(GLVec3(0, 0, -1));
	}
	
	for (int i=0;i<sides;++i)
	{
		GLVec3 P = Cyl->vertices_[i+1];
		Cyl->vertices_.push_back(P);
		P.z() = 0.0f;
		Cyl->normals_.push_back(P.normalized());
	}
	
	for (int i = 0; i < sides; ++i)
	{
		GLVec3 P = Cyl->vertices_[i + 1];
		P.z() = -1.0f;
		Cyl->vertices_.push_back(P);
		P.z() = 0.0f;
		Cyl->normals_.push_back(P.normalized());
	}


	for (int i = 0; i < sides; ++i)
	{
		Cyl->tri_indices.push_back(0);
		Cyl->tri_indices.push_back(i + 1);
		Cyl->tri_indices.push_back(((i + 1) % sides) + 1);
	}

	for (int i = 0; i < sides; ++i)
	{
		Cyl->tri_indices.push_back(sides+1);
		Cyl->tri_indices.push_back(((i + 1) % sides)+sides+2);
		Cyl->tri_indices.push_back(i + sides + 2);
	}

	int DEC = 2*sides + 2;
	for (int i=0;i<sides;++i)
	{
		Cyl->tri_indices.push_back(DEC + ((i + 1) % sides));
		Cyl->tri_indices.push_back(DEC + i);
		Cyl->tri_indices.push_back(DEC + i + sides);

		Cyl->tri_indices.push_back(DEC + ((i + 1) % sides));
		Cyl->tri_indices.push_back(DEC + i + sides);
		Cyl->tri_indices.push_back(DEC + ((i + 1) % sides)+sides);
	}

	Cyl->bb_.add_point(GLVec3(-1,-radius_ratio,-radius_ratio));
	Cyl->bb_.add_point(GLVec3(1,radius_ratio,radius_ratio));

	return Cyl;
}



SP_Mesh Mesh::ClosedCone(int sides, float radius_ratio)
{
	SP_Mesh Cone = std::make_shared<Mesh>();
	float radius = radius_ratio;
	float a = 0.0f;
	float b = 2.0f*M_PI/sides;
	for (int i=0;i<sides;++i)
	{
		Cone->vertices_.push_back(GLVec3(0,0, 1));
	}
	for (int i=0;i<2*sides;++i)
	{
		Cone->vertices_.push_back(GLVec3(radius*cos(a),radius*sin(a),-1));
		a+=b;
	}

	for (int i=0;i<sides;++i)
	{
		const GLVec3& U = Cone->vertices_[sides+(i+1)%sides] - Cone->vertices_[sides+(i-1+sides)%sides];
		const GLVec3& V = Cone->vertices_[i] - Cone->vertices_[i+sides];
		Cone->normals_.push_back(U.cross(V).normalized());
	}
	for (int i=0;i<sides;++i)
		Cone->normals_.push_back(Cone->normals_[i]);

	for (int i=0;i<sides;++i)
		Cone->normals_.push_back(GLVec3(0,0,-1));


	int cb = int(Cone->vertices_.size());
	Cone->vertices_.push_back(GLVec3(0,0,-1));
	Cone->normals_.push_back(GLVec3(0,0,-1));


	for (int i=0;i<sides;++i)
	{
		Cone->tri_indices.push_back(i);
		Cone->tri_indices.push_back(sides+i);
		Cone->tri_indices.push_back(sides+(i+1)%sides);

		Cone->tri_indices.push_back((i+1)%sides+2*sides);
		Cone->tri_indices.push_back(i+2*sides);
		Cone->tri_indices.push_back(cb);
	}

	Cone->bb_.add_point(GLVec3(-1,-radius,-radius));
	Cone->bb_.add_point(GLVec3(1,radius,radius));

	return Cone;
}

SP_Mesh Mesh::Tore(GLint m, GLint n, float radius_ratio)
{
	SP_Mesh tore = std::make_shared<Mesh>();
	tore->grid_topo(m,n);

	GLint n1 = n - 1;
	GLint m1 = m - 1;
	float radius1 = radius_ratio;

    tore->vertices_.reserve(m*n);
    tore->normals_.reserve(tore->vertices_.size());
    tore->tex_coords_.reserve(tore->vertices_.size());

    GLVVec3 cpos;// = create_Vec_buffer(3,n);
    cpos.reserve(n);
    GLVVec3 cnorm;//	let cnorm = create_Vec_buffer(3,n);
    cnorm.reserve(n);
    for(int i=0;i<n;++i)
	{
		double alpha = ((1.0/n1)*i)*2*M_PI;
        GLVec3 p(0,std::sin(alpha),std::cos(alpha));
        cnorm.push_back(p);
        cpos.push_back(p*radius1);
    }
    for(int j=0;j<m;++j)
    {
        GLMat4 tr = Transfo::rotateZ((360.0/m1)*j)* Transfo::translate(GLVec3(0,1,0));
        GLMat3 ntr = Transfo::sub33(tr); // no need to inverse_transpose because no scale
        double v = (1.0/n1)*j;
        for(int i=0;i<n;++i)
        {
            double u = (1.0/n1)*i;
            tore->tex_coords_.push_back(GLVec2(u,v));
            tore->vertices_.push_back(Transfo::apply(tr,cpos[i]));
            tore->normals_.push_back(Transfo::apply(ntr,cnorm[i]));
        }
    }

    tore->bb_.add_point({-1.0f-radius_ratio,-1.0f-radius_ratio,-radius_ratio});
    tore->bb_.add_point({1.0f+radius_ratio,1.0f+radius_ratio,radius_ratio});

	return tore;
}

Mesh::Mesh(::aiMesh* aimesh)
{
	if(aimesh->HasPositions())
	{
		vertices_.reserve(aimesh->mNumVertices);
		for(GLuint i = 0; i < aimesh->mNumVertices; ++i)
		{
			GLVec3 P(aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z);
			bb_.add_point(P);
			vertices_.push_back(P);
		}
	}
	if(aimesh->HasNormals())
	{
		normals_.reserve(aimesh->mNumVertices);
		for(GLuint i = 0; i < aimesh->mNumVertices; ++i)
			normals_.push_back(GLVec3(aimesh->mNormals[i].x,aimesh->mNormals[i].y,aimesh->mNormals[i].z));
	}
	if(aimesh->HasTextureCoords(0))
	{
		tex_coords_.reserve(aimesh->mNumVertices);
		for(GLuint i = 0; i < aimesh->mNumVertices; ++i)
			tex_coords_.push_back(GLVec2(aimesh->mTextureCoords[0][i].x,aimesh->mTextureCoords[0][i].y));
	}
    std::vector<std::vector<GLuint>> accel;

	if(aimesh->HasFaces())
	{
		accel.resize(vertices_.size());

		GLuint nb_tri_ind=aimesh->mNumFaces * 3;
		tri_indices.reserve(nb_tri_ind);

		for(GLuint i = 0; i < aimesh->mNumFaces; ++i)
		{
			auto A = aimesh->mFaces[i].mIndices[0];
			auto B = aimesh->mFaces[i].mIndices[1];
			auto C = aimesh->mFaces[i].mIndices[2];

			tri_indices.push_back(A);
			tri_indices.push_back(B);
			tri_indices.push_back(C);

            if (std::find(accel[B].begin(),accel[B].end(),A) == accel[B].end())
				accel[A].push_back(B);
            if (std::find(accel[C].begin(),accel[C].end(),B) == accel[C].end())
				accel[B].push_back(C);
            if (std::find(accel[A].begin(),accel[A].end(),C) == accel[A].end())
				accel[C].push_back(A);
		}

		line_indices.reserve(nb_tri_ind);
		for (GLuint i=0; i<accel.size(); ++i)
		{
            std::vector<GLuint>& vv = accel[i];
			for (GLuint j=0; j<vv.size(); ++j)
			{
				line_indices.push_back(i);
				line_indices.push_back(vv[j]);
			}
		}
	}

//	if(! aimesh->HasNormals())
//		compute_normals();

    std::cout << "NB Vertices = "<<vertices_.size() <<std::endl;
    std::cout << "NB Triangles = "<<tri_indices.size()/3 <<std::endl;


}

void Mesh::ai_process_node(std::vector<SP_Mesh>& meshes, aiNode *node, const aiScene *scene)
{
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(std::make_shared<Mesh>(aimesh));	//emplace_back(aimesh);
	}
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		Mesh::ai_process_node(meshes, node->mChildren[i], scene);
	}
}




std::vector<SP_Mesh> Mesh::load(const std::string& mesh_filename)
{
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(mesh_filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals |  aiProcess_CalcTangentSpace);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
        std::cerr << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
         return std::vector<SP_Mesh>();
	 }
     std::string dir = mesh_filename.substr(0, mesh_filename.find_last_of('/'));

     std::vector<SP_Mesh> meshes;
	 Mesh::ai_process_node(meshes, scene->mRootNode, scene);

     std::cout  << "RETURN MESHES "<< std::endl;
	 return meshes;
}




}
