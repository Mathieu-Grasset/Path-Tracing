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


#ifndef EASY_CPP_OGL_MESH_H_
#define EASY_CPP_OGL_MESH_H_

#include <gl_eigen.h>
#include <vao.h>
#include <ebo.h>
#include <memory>



struct aiMesh;
struct aiNode;
struct aiScene;

namespace EZCOGL
{


class BoundingBox
{
protected:
	GLVec3 min_;
	GLVec3 max_;
	bool initialized_;
	void direct_add_point(const GLVec3& P);

public:
	inline BoundingBox():
		initialized_(false)
	{}

	void add_point(const GLVec3& P);

	inline GLVec3 center() const
	{
		return (min_+max_)/2.0;
	}

	inline float radius() const
	{
		GLVec3 dv = max_ - min_;
		return dv.norm()/2.0f;
	}

	inline GLMat4 matrix() const
	{
		GLVec3 sc = (max_ - min_)/2.0f;
		return Transfo::translate(center()) * Transfo::scale(sc);
	}


	inline const GLVec3& min() const { return min_; }
	inline const GLVec3& max() const { return max_; }
};


class Mesh;
using SP_Mesh = std::shared_ptr<Mesh>;

class MeshRenderer;
using SP_MeshRenderer = std::shared_ptr<MeshRenderer>;

class Mesh
{
	friend class MeshRenderer;
public:
	std::vector<GLVec3> vertices_;
	std::vector<GLVec3> normals_;
	std::vector<GLVec2> tex_coords_;
	std::vector<GLVec3> colors_;
	std::vector<GLuint> tri_indices;
	std::vector<GLuint> line_indices;
	BoundingBox bb_;
protected:
	void grid_topo(GLint m, GLint n);

	static void ai_process_node(std::vector<SP_Mesh>& meshes, aiNode *node, const aiScene *scene);

public:
	inline Mesh() {}
	Mesh(::aiMesh* aimesh);
	Mesh(const Mesh&) = delete;
	Mesh(Mesh&& m);

	void compute_normals();
	//void linear_loop();

	inline int32_t nb_vertices() const { return int32_t(vertices_.size());}

	inline int32_t nb_triangles() const { return int32_t(tri_indices.size() / 3); }
	
	inline std::vector<GLVec3>& colors() { return colors_ ; }

	inline const BoundingBox& BB() const { return bb_;}

	SP_MeshRenderer renderer(GLint att_pos, GLint att_norm, GLint att_tc, GLint att_col) const;

	static SP_Mesh CubePosOnly();
	static SP_Mesh Cube();
    static SP_Mesh Grid(GLint m=4, GLint n=4);
    static SP_Mesh Wave(GLint m);
	static SP_Mesh Sphere( GLint n);
	//static Mesh IcoSphere(int nb_sub);
	static SP_Mesh ClosedCylinder(int sides, float radius_ratio);
	static SP_Mesh ClosedCone(int sides, float radius_ratio);

    static SP_Mesh Cylinder(GLint m, GLint n, float radius);
    static SP_Mesh Tore(GLint m, GLint n, float radius_ratio);

	static std::vector<SP_Mesh> load(const std::string& mesh_filename);

	inline std::vector<GLuint>::const_iterator triangle_index(int t) const {return tri_indices.begin()+3*t;}
	inline const GLVec3& triangle_vertex0(std::vector<GLuint>::const_iterator t) const { return vertices_[*t];}
	inline const GLVec3& triangle_vertex1(std::vector<GLuint>::const_iterator t) const { return vertices_[*(t+1)];}
	inline const GLVec3& triangle_vertex2(std::vector<GLuint>::const_iterator t) const { return vertices_[*(t+2)];}

	inline const GLVec3& triangle_normal0(std::vector<GLuint>::const_iterator t) const { return normals_[*t];}
	inline const GLVec3& triangle_normal1(std::vector<GLuint>::const_iterator t) const { return normals_[*(t+1)];}
	inline const GLVec3& triangle_normal2(std::vector<GLuint>::const_iterator t) const { return normals_[*(t+2)];}

	using TRI = std::tuple<uint32_t,uint32_t,uint32_t>;


};

class MeshRenderer
{
	friend class Mesh;
protected:
	VAO* vao_;
	EBO ebo_triangles_;
	EBO ebo_lines_;
	BoundingBox bb_;
public:
	MeshRenderer(const Mesh& m, GLint att_pos, GLint att_norm, GLint att_tc, GLint att_col);
	~MeshRenderer();
	void draw(GLenum prim);
};

}
#endif
