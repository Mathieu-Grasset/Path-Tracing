#include <thread>
#include <fstream>
#include<iterator>

#include <shader_program.h>
#include <vao.h>
#include <mesh.h>
#include <texture2d.h>
#include <fbo.h>
#include <gl_viewer.h>

#include "gpu_bvh_scene.h"
#include <chrono>
#include <random>

#pragma warning( disable : 4244 )

using namespace EZCOGL;

SrcLoader loader({"raytrace_tp_phong.frag","raytrace_tp_shadow.frag","raytrace_tp_reflets.frag","raytrace_tp_refracts.frag"},TP_SHADER_PATH);


const GLVec4 ROUGE   = {1,0,0,1};
const GLVec4 VERT    = {0,1,0,1};
const GLVec4 BLEU    = {0,0,1,1};
const GLVec4 JAUNE   = {1,1,0,1};
const GLVec4 CYAN    = {0,1,1,1};
const GLVec4 MAGENTA = {1,0,1,1};
const GLVec4 BLANC   = {1,1,1,1};
const GLVec4 GRIS    = {0.5,0.5,0.5,1};
const GLVec4 NOIR    = {0,0,0,1};
const GLVec4 ROUGE2   = {0.5,0,0,1};
const GLVec4 VERT2    = {0,0.5,0,1};
const GLVec4 ORANGE   = {1,0.5,0,1};

inline GLVec4 OPA(GLVec4 c, float o) { c[3] = o; return c; }

static const std::string fs_vert = R"(
#version 430
out vec2 tc;
void main()
{
	tc = vec2(gl_VertexID%2,gl_VertexID/2);
	gl_Position = vec4(2.0*tc-1.0,0,1);
}
)";


static const std::string fs_frag = R"(
#version 430
out vec3 frag_out;
in vec2 tc;
layout(binding=1) uniform sampler2D TU;
void main()
{
	frag_out =  texture(TU,tc).rgb;
}
)";


class RTViewer: public GLViewer
{

	SP_ShaderProgram prg_ray;
	SP_ShaderProgram prg_phong;
	SP_ShaderProgram prg_bb;
	SP_ShaderProgram prg_mesh;
	SP_MeshRenderer cube_;
	SP_MeshRenderer sphere_;
	SP_MeshRenderer cylinder_;
	SP_FBO fbo;
	SP_ShaderProgram prg_fs;
	SP_Texture2D tex_hsph_;

	GLVec3 light_position;
	float light_power;
	float k_opa;
	int nb_shadows;
	bool draw_rt_;
	GLint nb_bounces_;
	GLint sub_sampling_;
	GLint depth_bb_draw;
	GLint depth_mesh_draw;

	ScenePrimitives scene_;
	BVH_GPU_Scene bvh_gpu_scene_;

	SP_VAO vao_bb;
	GLVec4 Colors[5];

	std::chrono::high_resolution_clock::time_point last_time_;
	float alpha_;
	float speed_;

	void menger(const GLMat4& m, int d, float sc, const Material& mater);
	void menger_sphere(const GLMat4& m, int d, float sc, const Material& mater);

public:
	RTViewer();
	void scene1();
	void init_ogl() override;
	void draw_ogl() override;
	void interface_ogl() override;
	void resize_ogl(int32_t w, int32_t h) override;
	void key_press_ogl(int32_t k) override;
};
 
RTViewer::RTViewer() :
	light_position(30, 30, 100),
	light_power(1.0f),
	k_opa(1.4f),
	nb_shadows(1),
	draw_rt_(true),
	nb_bounces_(5),
	sub_sampling_(0),
	depth_bb_draw(0),
	depth_mesh_draw(0),
	bvh_gpu_scene_(scene_),
	speed_(5.0f),
	alpha_(0.0f)
{
}

void RTViewer::menger(const GLMat4& m, int d, float sc, const Material& mater)
{
	float x = 2.0f/3.0f;
	float y = sc/3.0f;
	auto f = [&](const GLMat4& t)
	{
		GLMat4 mm = m*t;
		if (d>0)
			menger(mm, d-1, sc,mater);
		else
			{
			bvh_gpu_scene_.add_cube(mm, mater);
			}
	};

	f(Transfo::translate(x,x,0)*Transfo::scale(y));
	f(Transfo::translate(-x,x,0)*Transfo::scale(y));
	f(Transfo::translate(-x,-x,0)*Transfo::scale(y));
	f(Transfo::translate(x,-x,0)*Transfo::scale(y));
	f(Transfo::translate(x,0,x)*Transfo::scale(y));
	f(Transfo::translate(-x,0,x)*Transfo::scale(y));
	f(Transfo::translate(-x,0,-x)*Transfo::scale(y));
	f(Transfo::translate(x,0,-x)*Transfo::scale(y));
	f(Transfo::translate(0,x,x)*Transfo::scale(y));
	f(Transfo::translate(0,-x,x)*Transfo::scale(y));
	f(Transfo::translate(0,-x,-x)*Transfo::scale(y));
	f(Transfo::translate(0,x,-x)*Transfo::scale(y));

	f(Transfo::translate(x,x,x)*Transfo::scale(y));
	f(Transfo::translate(-x,x,x)*Transfo::scale(y));
	f(Transfo::translate(-x,-x,x)*Transfo::scale(y));
	f(Transfo::translate(x,-x,x)*Transfo::scale(y));
	f(Transfo::translate(x,x,-x)*Transfo::scale(y));
	f(Transfo::translate(-x,x,-x)*Transfo::scale(y));
	f(Transfo::translate(-x,-x,-x)*Transfo::scale(y));
	f(Transfo::translate(x,-x,-x)*Transfo::scale(y));
}


void RTViewer::menger_sphere(const GLMat4& m, int d, float sc, const Material& mater)
{
	float x = 2.0f / 3.0f;
	float y = sc / 3.0f;
	auto f = [&](const GLMat4& t)
	{
		GLMat4 mm = m * t;
		if (d > 0)
			menger_sphere(mm, d - 1, sc, mater);
		else
		{
			bvh_gpu_scene_.add_sphere(mm, mater);
		}
	};

	f(Transfo::translate(x, x, 0) * Transfo::scale(y));
	f(Transfo::translate(-x, x, 0) * Transfo::scale(y));
	f(Transfo::translate(-x, -x, 0) * Transfo::scale(y));
	f(Transfo::translate(x, -x, 0) * Transfo::scale(y));
	f(Transfo::translate(x, 0, x) * Transfo::scale(y));
	f(Transfo::translate(-x, 0, x) * Transfo::scale(y));
	f(Transfo::translate(-x, 0, -x) * Transfo::scale(y));
	f(Transfo::translate(x, 0, -x) * Transfo::scale(y));
	f(Transfo::translate(0, x, x) * Transfo::scale(y));
	f(Transfo::translate(0, -x, x) * Transfo::scale(y));
	f(Transfo::translate(0, -x, -x) * Transfo::scale(y));
	f(Transfo::translate(0, x, -x) * Transfo::scale(y));

	f(Transfo::translate(x, x, x) * Transfo::scale(y));
	f(Transfo::translate(-x, x, x) * Transfo::scale(y));
	f(Transfo::translate(-x, -x, x) * Transfo::scale(y));
	f(Transfo::translate(x, -x, x) * Transfo::scale(y));
	f(Transfo::translate(x, x, -x) * Transfo::scale(y));
	f(Transfo::translate(-x, x, -x) * Transfo::scale(y));
	f(Transfo::translate(-x, -x, -x) * Transfo::scale(y));
	f(Transfo::translate(x, -x, -x) * Transfo::scale(y));
}


inline std::string load_shader_src_lib(const std::string& fname)
{
	auto ifs =  std::ifstream(SHADER_PATH + fname);
	auto s = std::string(std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>() );
	return s;
}


void RTViewer::key_press_ogl(int32_t k)
{
	 // Warning Layout toujours en QWERTY !!
	switch (k)
	{
	case 'O': //
		loader.prev();
			prg_ray = ShaderProgram::create({{GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
									{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") +
									 loader.load() + load_shader_src_lib("main.frag") } }, "Raytracer");
		break;
	case 'P':
		loader.next();
			prg_ray = ShaderProgram::create({{GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
									{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") +
									 loader.load() + load_shader_src_lib("main.frag") } }, "Raytracer");
		break;
	}

}

void RTViewer::init_ogl()
{
	prg_mesh = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("mesh_phong.vert")},{GL_FRAGMENT_SHADER,load_shader_src_lib("phong.frag")} }, "mesh");
	prg_bb = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("bb.vert")},{GL_FRAGMENT_SHADER,load_shader_src_lib("bb.frag")} }, "bb");
	prg_phong = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("phong.vert")},{GL_FRAGMENT_SHADER,load_shader_src_lib("phong.frag")} }, "phong");
	prg_ray = ShaderProgram::create({{GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
									{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") +
									 loader.load() + load_shader_src_lib("main.frag") } }, "Raytracer");
	prg_fs = ShaderProgram::create({ {GL_VERTEX_SHADER,fs_vert},{GL_FRAGMENT_SHADER,fs_frag} }, "FS");

	cube_ = Mesh::Cube()->renderer(1,2,-1,-1);
	sphere_ = Mesh::Sphere(50)->renderer(1,2,-1,-1);
	cylinder_ = Mesh::Cylinder(10, 3, 1.0)->renderer(1, 2, -1, -1);

	std::cout << "compute scene in " << std::endl;
	auto start = std::chrono::high_resolution_clock::now();

	scene1();
	
	bvh_gpu_scene_.finalize();

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed_seconds = end - start;
	std::cout << " compute bvh in : " << 1000.0f * elapsed_seconds.count() << "ms" << std::endl;

	last_time_ = std::chrono::high_resolution_clock::now();
	alpha_ = 0;
	
	auto vbo_bb = VBO::create(GLVVec3{
		{-1,-1,-1},{-1,-1, 1},
		{ 1,-1,-1},{ 1,-1, 1},
		{ 1, 1,-1},{ 1, 1, 1},
		{-1, 1,-1},{-1, 1, 1},
		{-1,-1,-1},{ 1,-1,-1},
		{ 1,-1,-1},{ 1, 1,-1},
		{ 1, 1,-1},{-1, 1,-1},
		{-1, 1,-1},{-1,-1,-1},
		{-1,-1, 1},{ 1,-1, 1},
		{ 1,-1, 1},{ 1, 1, 1},
		{ 1, 1, 1},{-1, 1, 1},
		{-1, 1, 1},{-1,-1, 1}
		});

	vao_bb = VAO::create({ {1,vbo_bb} });

	auto t = Texture2D::create({ GL_LINEAR });
	t->init(GL_RGBA8);
	fbo = FBO::create({ t });

	set_scene_center(GLVec3(0,0,0));
	set_scene_radius(95);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
}

void RTViewer::draw_ogl()
{
	auto now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed_seconds = now - last_time_;
	alpha_ += elapsed_seconds.count() * speed_;
	last_time_ = now;

	const GLMat4& proj = this->get_projection_matrix();
	GLMat4 view = this->get_modelview_matrix() * Transfo::rotateZ(alpha_);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (draw_rt_)
	{
		if (sub_sampling_ != 0)
		{
			FBO::push();
			fbo->bind();
		}

		glDisable(GL_DEPTH_TEST);
		prg_ray->bind();


		bvh_gpu_scene_.tex_prim_->bind(1);
		bvh_gpu_scene_.tex_bb_->bind(2);
		bvh_gpu_scene_.tex_ind_->bind(3);

		bvh_gpu_scene_.tex_tri_->bind(4);
		bvh_gpu_scene_.tex_p_->bind(5);
		bvh_gpu_scene_.tex_n_->bind(6);


		set_uniform_value(BVH_GPU::uniform_invPV, (proj * view).inverse());
		set_uniform_value(BVH_GPU::uniform_invV, view.inverse());
		set_uniform_value(BVH_GPU::uniform_nb_prims, bvh_gpu_scene_.nb_prim());
		set_uniform_value(BVH_GPU::uniform_bvh_depth, bvh_gpu_scene_.depth(0));
		set_uniform_value(BVH_GPU::uniform_date, 0);

		GLVec4 L4(light_position.x(), light_position.y(), light_position.z(), light_power*light_position.dot(light_position));
	
		set_uniform_value(20, L4);
		set_uniform_value(21, nb_bounces_);
		set_uniform_value(22, k_opa);
		VAO::none()->bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		if (sub_sampling_ != 0)
		{
			FBO::pop();
			glDisable(GL_DEPTH_TEST);
			prg_fs->bind();
			VAO::none()->bind();
			fbo->texture(0)->bind(1);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		prg_phong->bind();
		set_uniform_value(0,proj);

		GLVec3 pl = Transfo::apply(view, light_position);

		set_uniform_value(3,pl);
		std::vector<int> bv_meshes;
		for (int i=0; i<scene_.nb(); ++i)
		{
			GLMat4 mv = view * scene_.transfo(i);
			set_uniform_value(1,mv);
			set_uniform_value(2,Transfo::inverse_transpose(mv));
			set_uniform_value(4, scene_.color(i));
			switch (scene_.type(i))
			{
			case 0:
				bv_meshes.push_back(i);
				break;
				case 1: sphere_->draw(GL_TRIANGLES);
					break;
				case 2: cube_->draw(GL_TRIANGLES);
					break;
				case 3: cylinder_->draw(GL_TRIANGLES);
					break;
			}
		}

		prg_mesh->bind();
		set_uniform_value(0, proj);
		set_uniform_value(3, pl);
		
		bvh_gpu_scene_.tex_tri_->bind(4);
		bvh_gpu_scene_.tex_p_->bind(5);
		bvh_gpu_scene_.tex_n_->bind(6);

		for (int i = 0; i < scene_.nb(); ++i)
		{
			const Mesh* m = scene_.dbg_buf_meshes_[i];
			if (m!= nullptr)
			{
				GLMat4 trf = scene_.transfo_mesh_bb(i);
				auto mv = view * trf;
				set_uniform_value(1, mv);
				set_uniform_value(2, Transfo::inverse_transpose(mv));
				set_uniform_value(4, scene_.color(i));
				set_uniform_value(6, scene_.dbg_buf_mline_[i]);
				VAO::none()->bind();
				glDrawArraysInstanced(GL_TRIANGLES, 0, 3, m->nb_triangles());
			}
			
		}


		prg_bb->bind();
		set_uniform_value(0, proj);
		set_uniform_value(1,view);
		int first = int(std::pow(2, depth_bb_draw)) - 1;
		int nb = int(std::pow(2, depth_bb_draw));
		set_uniform_value(2, first);
		set_uniform_value(3, 0);
		set_uniform_value(4,BLANC);
		bvh_gpu_scene_.tex_bb_->bind(1);
		vao_bb->bind();
		glDrawArraysInstanced(GL_LINES, 0, 24, nb);

		set_uniform_value(4, JAUNE);
		for (int i:bv_meshes)
		{ 
			GLMat4 mv = view * scene_.transfo_mesh_bb(i);
			set_uniform_value(1, mv);
			int j = scene_.mesh_line(i);
			int bl = bvh_gpu_scene_.mesh_bvh_info_[j].first;
			int d = bvh_gpu_scene_.mesh_bvh_info_[j].second;
			int dd = std::min(depth_mesh_draw, d);

			int first = int(std::pow(2, dd)) - 1;
			int nb = int(std::pow(2, dd));
			set_uniform_value(2, first);
			set_uniform_value(3, bl);
			glDrawArraysInstanced(GL_LINES, 0, 24, nb);
		}
	}

	//std::cout << glfwGetTime() << std::endl;
}


void RTViewer::interface_ogl()
{
	ImGui::GetIO().FontGlobalScale = 2.0f;
	ImGui::Begin("RayTracing GPU Demo",nullptr, ImGuiWindowFlags_NoSavedSettings);
	ImGui::SetWindowSize({0,0});
	ImGui::Text("FPS :(%2.2lf)", fps_);
	ImGui::Checkbox("Shader RT",&draw_rt_);
	if (ImGui::Button("Reload"))
	{
		prg_ray = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
								{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") +
								 loader.load() + load_shader_src_lib("main.frag") } }, "Raytracer");
	}
	if (draw_rt_)
	{
		if (ImGui::SliderInt("SubSampling", &sub_sampling_, 0, 5))
			resize_ogl(vp_w_, vp_h_);
		ImGui::SliderInt("Bounce", &nb_bounces_, 0, 20);
		ImGui::SliderFloat("LightPower", &light_power, 0.5, 1.5);
		ImGui::SliderFloat("K opa", &k_opa, 0.5, 2);
	}
	else
	{
		ImGui::SliderInt("depth BB draw", &depth_bb_draw, 0, bvh_gpu_scene_.depth(0));

	}
	ImGui::SliderFloat("Rotation speed", &speed_, 0.0f, 10.0f);
	ImGui::End();
}

void RTViewer::resize_ogl(int32_t w, int32_t h)
{
	if (sub_sampling_ != 0)
	{
		int k = int(std::pow(2, sub_sampling_));

		fbo->resize(w / k, h / k);
	}
}

void RTViewer::scene1()
{
	bvh_gpu_scene_.add_cube(Transfo::translate(0, 0, -9.2f) * Transfo::scale(9975, 9975, 1), Material(GRIS, 0.3f, 0.02f));

	menger(Transfo::translate(0, 0, 15) * Transfo::rotateZ(15) * Transfo::scale(25.0f), 1, 0.8f, Material(ROUGE));

	bvh_gpu_scene_.add_sphere(Transfo::translate(0, 0, 15) * Transfo::scale(10), Material(OPA(MAGENTA,0.7f), 0.7f, 0.99f));

	bvh_gpu_scene_.add_sphere(Transfo::translate(-50, -50, 10) * Transfo::scale(17), Material(OPA(BLANC, 0.9f), 0.5f,0.6f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(-50, 50, 10) * Transfo::scale(17), Material(OPA(CYAN, 0.6f), 0.4f, 0.9f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(50, 50, 10) * Transfo::scale(17), Material(OPA(JAUNE, 0.3f), 0.6f, 0.8f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(50, -50, 10) * Transfo::scale(17), Material(OPA(VERT, 0.1f), 0.3f, 0.5f));

	bvh_gpu_scene_.add_sphere(Transfo::translate(0, 0, 50) * Transfo::scale(10), Material(GRIS, 0.95f, 0.5f));
}



int main(int, char**)
{
	Eigen::initParallel();
	RTViewer v;
	v.set_size(1280,1000);
	return v.launch3d();
}

