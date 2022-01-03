#include <thread>
#include <fstream>
#include<iterator>

#include <shader_program.h>
#include <vao.h>
#include <mesh.h>
#include <texture2d.h>
#include <fbo.h>
#include <gl_viewer.h>
#include<GLFW/glfw3.h>
#include "gpu_bvh_scene.h"
#include <chrono>
#include <random>

#ifdef _WIN32
#include <Windows.h>
inline void msleep(int ms) { Sleep(ms); }
#pragma warning( disable : 4244)
#else
#include <unistd.h>
inline void msleep(int ms) { usleep(ms * 1000);}
#endif


// for easy shader switching (see key_press_ogl)
SrcLoader loader({ "montecarlo.frag", "montecarlo_mat.frag","montecarlo_mat_tr.frag" }, TP_SHADER_PATH); 
					//"montecarlo_light_sampling.frag","montecarlo_light_sampling_mat.frag" }, TP_SHADER_PATH);


using namespace EZCOGL;

const GLVec4 ROUGE   = {0.9f,0,0,1};
const GLVec4 VERT    = {0,0.9f,0,1};
const GLVec4 BLEU    = {0,0,0.9f,1};
const GLVec4 JAUNE   = {0.9f,0.9f,0,1};
const GLVec4 CYAN    = {0,0.9f,0.9f,1};
const GLVec4 MAGENTA = {0.9f,0,0.9f,1};
const GLVec4 BLANC   = {0.9f,0.9f,0.9f,1};
const GLVec4 GRIS    = {0.45f,0.45f,0.45f,1};
const GLVec4 NOIR    = {0,0,0,1};
const GLVec4 ROUGE2   = {0.45f,0,0,1};
const GLVec4 VERT2    = {0,0.45f,0,1};
const GLVec4 ORANGE   = {0.9f,0.45f,0,1};

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
layout(location=1) uniform float nb;

void main()
{
	frag_out =  texture(TU,tc).rgb / nb;
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
	
	bool draw_rt_;
	GLint nb_bounces_;
	GLint nb_paths_;
	GLint sub_sampling_;
	GLint depth_bb_draw;
	GLint depth_mesh_draw;

	ScenePrimitives scene_;
	BVH_GPU_Scene bvh_gpu_scene_;

	SP_VAO vao_bb;
	GLVec4 Colors[5];

	std::chrono::high_resolution_clock::time_point last_time_;
	bool locked_;
	int nb_locked_;
	bool freezed_;
	float total_seconds_;
	float k_op_;

	void menger(const GLMat4& m, int d, float sc, const Material& mater);
	void menger_sphere(const GLMat4& m, int d, float sc, const Material& mater);

public:
	RTViewer();
	void scene_4boules();
	void scene_colonnes();
	void scene_box_diffuse();
	void scene_box_balls();
	void scene_materials();
	void scene_menger();
	void scene_menger_lights();
	void scene_box_no_top();
	void init_ogl() override;
	void draw_ogl() override;
	void interface_ogl() override;
	void key_press_ogl(int32_t /*key_code*/) override;
	void resize_ogl(int32_t w, int32_t h) override;
};
 
RTViewer::RTViewer() :
	draw_rt_(true),
	nb_bounces_(3),
	nb_paths_(1),
	sub_sampling_(0),
	depth_bb_draw(0),
	depth_mesh_draw(0),
	bvh_gpu_scene_(scene_),
	locked_(false),
	nb_locked_(-1),
	total_seconds_(0.0f),
	freezed_(false),
	k_op_(1.2f)
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
//
//inline std::string load_shader_src_tp(const std::string& fname)
//{
//	auto ifs =  std::ifstream(TP_SHADER_PATH + fname);
//	auto s = std::string(std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>() );
//	return s;
//}

void RTViewer::key_press_ogl(int32_t k)
{
	auto update = [this] ()
	{
		freezed_ = false;
		locked_ = false;
		nb_locked_ = 0;
		unlock();
		bvh_gpu_scene_.finalize();
		total_seconds_=0.0f;
	};
	
	// Warning Layout toujours en QWERTY !!
	switch (k)
	{
	case 'Q':
		bvh_gpu_scene_.clear();
		scene_box_diffuse();
		update();
		break;
	case 'W':
		bvh_gpu_scene_.clear();
		scene_box_balls();
		update();
		break;
	case 'E':
		bvh_gpu_scene_.clear();
		scene_menger();
		update();
		break;
	case 'R':
		bvh_gpu_scene_.clear();
		scene_box_no_top();
		update();
		break;
	case 'T':
		bvh_gpu_scene_.clear();
		scene_materials();
		update();
		break;
	case 'Y':
		bvh_gpu_scene_.clear();
		scene_4boules();
		update();
		break;
	case 'U':
		bvh_gpu_scene_.clear();
		scene_menger_lights();
		update();
		break;
	case 'I':
		bvh_gpu_scene_.clear();
		scene_colonnes();
		update();
		break;

	case 'O':
		loader.prev();
		total_seconds_ = 0.0f;
		prg_ray = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
		{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") + loader.load() + load_shader_src_lib("main.frag") } }, "Raytracer");
		break;

	case 'P':
		loader.next();
		total_seconds_ = 0.0f;
		prg_ray = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
		{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") + loader.load() + load_shader_src_lib("main.frag") } }, "Raytracer");
		break;

	//case 'A':
	//	tp_fname =  &tp_fname1;
	//	prg_ray = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
	//	{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") + load_shader_src_tp(*tp_fname) + load_shader_src_lib("main.frag") } }, "Raytracer");
	//	break;
	//case 'S':
	//	tp_fname =  &tp_fname2;
	//	prg_ray = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
	//	{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") + load_shader_src_tp(*tp_fname) + load_shader_src_lib("main.frag") } }, "Raytracer");
	//	break;
	//case 'D':
	//	tp_fname =  &tp_fname3;
	//	prg_ray = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
	//	{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") + load_shader_src_tp(*tp_fname) + load_shader_src_lib("main.frag") } }, "Raytracer");
	//	break;
	//case 'F':
	//	tp_fname =  &tp_fname4;
	//	prg_ray = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
	//	{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") + load_shader_src_tp(*tp_fname) + load_shader_src_lib("main.frag") } }, "Raytracer");
	//	break;
	//case 'G':
	//	tp_fname =  &tp_fname5;
	//	prg_ray = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
	//	{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") + load_shader_src_tp(*tp_fname) + load_shader_src_lib("main.frag") } }, "Raytracer");
	//	break;

	default:
		
		break;
	}
	
}

void RTViewer::init_ogl()
{
	glfwSwapInterval(0);
	prg_mesh = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("mesh_phong.vert")},{GL_FRAGMENT_SHADER,load_shader_src_lib("phong.frag")} }, "mesh");
	prg_bb = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("bb.vert")},{GL_FRAGMENT_SHADER,load_shader_src_lib("bb.frag")} }, "bb");
	prg_phong = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("phong.vert")},{GL_FRAGMENT_SHADER,load_shader_src_lib("phong.frag")} }, "phong");
	prg_ray = ShaderProgram::create({{GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
									{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") +
									loader.load() + load_shader_src_lib("main.frag"	) } }, "Raytracer");
	prg_fs = ShaderProgram::create({ {GL_VERTEX_SHADER,fs_vert},{GL_FRAGMENT_SHADER,fs_frag} }, "FS");

	cube_ = Mesh::Cube()->renderer(1,2,-1,-1);
	sphere_ = Mesh::Sphere(50)->renderer(1,2,-1,-1);
	cylinder_ = Mesh::ClosedCylinder(32,1.0)->renderer(1, 2, -1, -1);

	std::cout << "compute scene in " << std::endl;
	auto start = std::chrono::high_resolution_clock::now();

	scene_box_diffuse();
	
	bvh_gpu_scene_.finalize();

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed_seconds = end - start;
	std::cout << " compute bvh in : " << 1000.0f * elapsed_seconds.count() << "ms" << std::endl;

	last_time_ = std::chrono::high_resolution_clock::now();
	
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

	auto t = Texture2D::create({ GL_NEAREST });
	t->init(GL_RGB32F);
	fbo = FBO::create({ t });

	set_scene_center(GLVec3(0,0,0));
	set_scene_radius(145);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
}

void RTViewer::draw_ogl()
{
	
	auto now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed_seconds = now - last_time_;
	last_time_ = now;
	total_seconds_ += float(elapsed_seconds.count());


	const GLMat4& proj = this->get_projection_matrix();
	GLMat4 view = this->get_modelview_matrix() * Transfo::rotateX(-80);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (draw_rt_)
	{
//		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_DEPTH_TEST);

		if (freezed_)
		{
			if (elapsed_seconds.count() < 0.01)
				msleep(int(16 - elapsed_seconds.count() * 1000));
		}
		else
		{
			FBO::push();
			fbo->bind();
			glDisable(GL_DEPTH_TEST);
			if (!locked_)
				glClear(GL_COLOR_BUFFER_BIT);

			++nb_locked_;

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

			set_uniform_value(20, bvh_gpu_scene_.nb_emissives());
			set_uniform_value(21, nb_bounces_);
			set_uniform_value(22, locked_ ? 1 : nb_paths_);
			set_uniform_value(23, k_op_);
			VAO::none()->bind();
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			set_uniform_value(BVH_GPU::uniform_date, elapsed_seconds.count());
			if (locked_)
			{
				set_uniform_value(BVH_GPU::uniform_nbp, nb_locked_ );
				//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
			else
			{
				for (int j = 0; j < nb_paths_; ++j)
				{
					set_uniform_value(BVH_GPU::uniform_nbp,nb_locked_+j);
					//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				}
			}
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glDisable(GL_BLEND);
		}

		FBO::pop();
		glDisable(GL_DEPTH_TEST);
		prg_fs->bind();
		set_uniform_value(1, float(locked_ ? nb_locked_ : nb_paths_));
		VAO::none()->bind();
		fbo->texture(0)->bind(1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		prg_phong->bind();
		set_uniform_value(0,proj);

		GLVec3 pl = Transfo::apply(view, GLVec3(100,100,1000));

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
	ImGui::Begin(loader.name().c_str(),nullptr, ImGuiWindowFlags_NoSavedSettings);
	ImGui::SetWindowSize({0,0});
	ImGui::Text("FPS :(%2.2lf)", fps_);
	ImGui::Checkbox("Shader RT",&draw_rt_);
	if (ImGui::Button("Reload"))
	{
		prg_ray = ShaderProgram::create({ {GL_VERTEX_SHADER,load_shader_src_lib("raytracer.vert")},
								{GL_FRAGMENT_SHADER,load_shader_src_lib("raytracer_func.frag") + 
								loader.load() + load_shader_src_lib("main.frag") } }, "Raytracer");
		total_seconds_ = 0.0f;
	}
	if (draw_rt_)
	{
		if (ImGui::SliderInt("SubSampling", &sub_sampling_, 0, 5))
			resize_ogl(vp_w_, vp_h_);
		ImGui::SliderInt("Bounces", &nb_bounces_, 0, 9);
		ImGui::SliderInt("Paths", &nb_paths_, 1, 8);
		ImGui::SliderFloat("K op", &k_op_, 0.2f, 2.0f);
		if (ImGui::Checkbox("lock", &locked_))
		{
			if (locked_)
			{
				nb_locked_ = 0;
				lock();
			}
			else
				unlock();
			freezed_ = false;
			total_seconds_ = 0.0f;
		}
		if (locked_)
		{
			ImGui::Text("nb path :(%d)", nb_locked_);
			ImGui::Checkbox("freeze", &freezed_);
		}
	}
	else
	{
		ImGui::SliderInt("depth BB draw", &depth_bb_draw, 0, bvh_gpu_scene_.depth(0));

	}
	ImGui::End();
}

void RTViewer::resize_ogl(int32_t w, int32_t h)
{
		int k = int(std::pow(2, sub_sampling_));
		fbo->resize(w / k, h / k);
		freezed_ = false;
		locked_ = false;
		nb_locked_ = 0;
		unlock();
		total_seconds_ = 0.0f;

}


void RTViewer::scene_box_no_top()
{
	//boite
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, -100) * Transfo::scale(100, 100, 1), Material(BLANC));
//	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, 100) * Transfo::rotateX(180) * Transfo::scale(100, 100, 1), Material(BLANC));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 100, 0) * Transfo::rotateX(90) * Transfo::scale(100, 100, 1), Material(CYAN));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 99, 0) * Transfo::rotateX(90) * Transfo::scale(40, 60, 1), Material(BLANC, 1, 1));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, -100, 0) * Transfo::rotateX(-90) * Transfo::scale(100, 100, 1), Material(BLANC));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(-100, 0, 0) * Transfo::rotateY(90) * Transfo::scale(100, 100, 1), Material(BLANC));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(100, 0, 0) * Transfo::rotateY(-90) * Transfo::scale(100, 100, 1), Material(BLANC));

	//objet
	bvh_gpu_scene_.add_cube(Transfo::translate(70, 20, -60) * Transfo::rotateZ(20) * Transfo::scale(20, 20, 40), Material(ROUGE));
	bvh_gpu_scene_.add_cube(Transfo::translate(-70, 40, -60) * Transfo::rotateZ(-20) * Transfo::scale(20, 20, 40), Material(VERT));
	bvh_gpu_scene_.add_sphere(Transfo::translate(0, 50, -80) * Transfo::scale(20), Material(MAGENTA, 0.8f, 0.995f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(-0, -30, 0) * Transfo::scale(40), Material(OPA(JAUNE,0.1f), 0.65f, 1));
	bvh_gpu_scene_.add_sphere(Transfo::translate(70, 20, 5) * Transfo::scale(20), Material(ROUGE, 0.8f, 0.95f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(-70, 40, 5) * Transfo::scale(20), Material(VERT, 0.7f, 0.9f));

	//lumieres
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(99, -10, -40) * Transfo::rotateY(-90) * Transfo::scale(60, 5, 1), Material::light(BLANC, 10));


}


void RTViewer::scene_menger_lights()
{
	bvh_gpu_scene_.add_cube(Transfo::translate(0, 0, -10) * Transfo::scale(9975, 9975, 1), Material(BLANC, 0.5f, 0.9f));
	
	menger(Transfo::translate(0, 0, 42) * Transfo::rotateZ(15) * Transfo::scale(50.0f), 1, 0.9f, Material(ROUGE));

	menger(Transfo::translate(-105, 0, 11) * Transfo::scale(20.0f), 0, 0.7f, Material(BLEU));
	menger(Transfo::translate(0, -105, 11) * Transfo::scale(20.0f), 0, 0.7f, Material(CYAN));
	menger(Transfo::translate(0, 105, 11) * Transfo::scale(20.0f), 0, 0.7f, Material(MAGENTA));
	menger(Transfo::translate(105, 0, 11) * Transfo::scale(20.0f), 0, 0.7f, Material(JAUNE));
	
	
	bvh_gpu_scene_.add_sphere(Transfo::translate(-100, -100, 5) * Transfo::scale(15), Material(GLVec4(1, 1, 1, 0.3f), 0.99f, 0.6f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(-100, 100, 5) * Transfo::scale(15), Material(GLVec4(1, 0, 1, 0.2f), 0.8f, 0.4f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(100, 100, 5) * Transfo::scale(15), Material(GLVec4(1, 1, 0, 0.4f), 0.6f, 0.2f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(100, -100, 5) * Transfo::scale(15), Material(GLVec4(0, 1, 0, 0.1f), 0.4f, 0.1f));

	// toit noir pour assombrir
	bvh_gpu_scene_.add_cube(Transfo::translate(0, 0, 500) * Transfo::scale(1000, 1000, 1), Material(NOIR));

	bvh_gpu_scene_.add_sphere(Transfo::translate(0, 0, 42) * Transfo::scale(10), Material::light(BLANC, 10));
	bvh_gpu_scene_.add_sphere(Transfo::translate(-105, 0, 11) * Transfo::scale(5), Material::light(BLANC,10));
	bvh_gpu_scene_.add_sphere(Transfo::translate(105, 0, 11) * Transfo::scale(5), Material::light(BLANC, 10));
	bvh_gpu_scene_.add_sphere(Transfo::translate(0, 105, 11) * Transfo::scale(5), Material::light(BLANC, 10));
	bvh_gpu_scene_.add_sphere(Transfo::translate(0, -105, 11) * Transfo::scale(5), Material::light(BLANC, 10));

}

void RTViewer::scene_menger()
{
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, -100) * Transfo::scale(9000, 9000, 1), Material(BLANC,0.8f,0.999f));

	menger(Transfo::translate(0, 0, -50) * Transfo::rotateZ(15) * Transfo::scale(50), 1, 0.9f, Material(MAGENTA));

	bvh_gpu_scene_.add_cylinder(Transfo::translate(80, 80, -75) * Transfo::scale(15,15,25), Material(BLEU));
	bvh_gpu_scene_.add_cylinder(Transfo::translate(-80, 80, -75) * Transfo::scale(15, 15, 25), Material(VERT));
	bvh_gpu_scene_.add_cylinder(Transfo::translate(-80, -80, -75) * Transfo::scale(15, 15, 25), Material(ROUGE));
	bvh_gpu_scene_.add_cylinder(Transfo::translate(80, -80, -75) * Transfo::scale(15, 15, 25), Material(JAUNE));
	bvh_gpu_scene_.add_sphere(Transfo::translate(80, 80, -30) * Transfo::scale(20), Material(CYAN,0.6f,0.998f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(-80, 80, -30) * Transfo::scale(20), Material(OPA(VERT,0.1f), 0.7f, 0.5f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(-80, -80, -30) * Transfo::scale(20), Material(ROUGE, 0.95f, 0.97f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(80, -80, -30) * Transfo::scale(20), Material(OPA(JAUNE,0.25f), 0.5f, 0.999f));

	bvh_gpu_scene_.add_sphere(Transfo::translate(0, 00, -50) * Transfo::scale(20), Material(BLANC, 1, 1));
}

void RTViewer::scene_box_diffuse()
{
	//boite
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, -100) * Transfo::scale(100, 100, 1), Material(BLANC));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, 100) * Transfo::rotateX(180) * Transfo::scale(100, 100, 1), Material(BLANC));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 100, 0) * Transfo::rotateX(90) * Transfo::scale(100, 100, 1), Material(CYAN));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, -100, 0) * Transfo::rotateX(-90) * Transfo::scale(100, 100, 1), Material(JAUNE));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(-100, 0, 0) * Transfo::rotateY(90) * Transfo::scale(100, 100, 1), Material(ROUGE));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(100, 0, 0) * Transfo::rotateY(-90) * Transfo::scale(100, 100, 1), Material(VERT));

	//objet
	bvh_gpu_scene_.add_cube(Transfo::translate(70, 20, -40) * Transfo::rotateZ(20) * Transfo::scale(20, 20, 60), Material(BLANC));
	bvh_gpu_scene_.add_cube(Transfo::translate(-70, 40, -40) * Transfo::rotateZ(-20) * Transfo::scale(20, 20, 60), Material(BLANC));

	//lumiere
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, 99) * Transfo::rotateX(180) * Transfo::scale(40, 40, 1), Material::light(BLANC, 20));
}


void RTViewer::scene_box_balls()
{
	//boite
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, -100) * Transfo::scale(100, 100, 1), Material(BLANC));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, 100) * Transfo::rotateX(180) * Transfo::scale(100, 100, 1) , Material(BLANC));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 100, 0) * Transfo::rotateX(90) * Transfo::scale(100, 100, 1), Material(CYAN));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 99, 0) * Transfo::rotateX(90) * Transfo::scale(40, 60, 1), Material(BLANC, 1, 1));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, -100, 0) * Transfo::rotateX(-90) * Transfo::scale(100, 100, 1), Material(BLANC));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(-100, 0, 0) * Transfo::rotateY(90) * Transfo::scale(100, 100, 1), Material(BLANC));
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(100, 0, 0) * Transfo::rotateY(-90) * Transfo::scale(100, 100, 1), Material(BLANC));

	//objet
	bvh_gpu_scene_.add_cube(Transfo::translate(70, 20, -60) * Transfo::rotateZ(20) * Transfo::scale(20, 20, 40), Material(ROUGE));
	bvh_gpu_scene_.add_cube(Transfo::translate(-70, 40, -60) * Transfo::rotateZ(-20) * Transfo::scale(20, 20, 40), Material(VERT));
	bvh_gpu_scene_.add_sphere(Transfo::translate(0, 50, -80) * Transfo::scale(20), Material(MAGENTA, 0.8f, 0.995f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(-0, -30, 0) * Transfo::scale(40), Material(OPA(JAUNE,0.5f), 0.65f, 1));
	bvh_gpu_scene_.add_sphere(Transfo::translate(70, 20, 5) * Transfo::scale(20), Material(OPA(ROUGE,0.2f), 0.8f, 0.95f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(-70, 40, 5) * Transfo::scale(20), Material(VERT, 0.7f, 0.9f));

	//lumieres
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, 99) * Transfo::rotateX(180) * Transfo::scale(40, 40, 1), Material::light(BLANC, 12.0f));
}

void RTViewer::scene_materials()
{
	bvh_gpu_scene_.add_cube(Transfo::translate(0, 0, -50) * Transfo::scale(9000, 9000, 1), Material(BLANC));

	//objets
	for (int j=-5;j<=5;j++)
		for (int i = -5; i <= 5; i++)
			bvh_gpu_scene_.add_sphere(Transfo::translate(30*i,30*j, -41) * Transfo::scale(8), Material(ROUGE, 1.0f-0.075*(i+5), 1.0f-0.01f*(j+5)));

	
}


void RTViewer::scene_4boules()
{
	bvh_gpu_scene_.add_cube(Transfo::translate(0, 0, -51) * Transfo::scale(9000, 9000, 1), Material(BLANC,0.2f,0.99999f));

	bvh_gpu_scene_.add_sphere(Transfo::translate(110,0,0) * Transfo::scale(50), Material(OPA(MAGENTA,0.01f),0.7f,0.99f));

	bvh_gpu_scene_.add_sphere(Transfo::translate(-110, 0, 0) * Transfo::scale(50), Material(OPA(ROUGE, 0.15f), 0.5f, 0.5f));

	bvh_gpu_scene_.add_sphere(Transfo::translate(0, 110, 0) * Transfo::scale(50), Material(OPA(CYAN, 0.05f), 0.8f, 0.7f));

	bvh_gpu_scene_.add_sphere(Transfo::translate(0, -110, 0) * Transfo::scale(50), Material(OPA(VERT, 0.25f), 0.7f, 0.9f));

	bvh_gpu_scene_.add_orientedQuad( Transfo::translate(200, 0, 100) * Transfo::rotateY(-110) * Transfo::scale(20, 20, 1), Material::light(BLANC, 20));

}

void RTViewer::scene_colonnes()
{
	bvh_gpu_scene_.add_orientedQuad(Transfo::translate(0, 0, -100) * Transfo::scale(90000, 90000, 1), Material(0.6f*BLANC+0.4f*VERT,0.7f,0.9999f));

	for (int i = -1000;i<=1000; i+= 250)
		for (int j = -1000; j <= 1000; j += 250)
		{
			bvh_gpu_scene_.add_cylinder(Transfo::translate(i, j, -98) * Transfo::scale(60, 60, 2), Material(BLANC));
			bvh_gpu_scene_.add_cylinder(Transfo::translate(i, j, -93) * Transfo::scale(50, 50, 3), Material(BLANC));
			bvh_gpu_scene_.add_cylinder(Transfo::translate(i, j, -85) * Transfo::scale(30, 30, 5), Material(BLANC));
			bvh_gpu_scene_.add_cylinder(Transfo::translate(i, j, 0) * Transfo::scale(20, 20, 80), Material(BLANC));
			bvh_gpu_scene_.add_cube(Transfo::translate(i, j, 90) * Transfo::scale(30, 30, 10), Material(BLANC));
			bvh_gpu_scene_.add_cube(Transfo::translate(i , j, 105) * Transfo::rotateZ( 45) * Transfo::translate(90, 0, 0) * Transfo::scale(80, 10, 5), Material(BLANC));
			bvh_gpu_scene_.add_cube(Transfo::translate(i , j, 105) * Transfo::rotateZ(135) * Transfo::translate(90, 0, 0) * Transfo::scale(80, 10, 5), Material(BLANC));
			bvh_gpu_scene_.add_cube(Transfo::translate(i , j, 105) * Transfo::rotateZ(225) * Transfo::translate(90, 0, 0) * Transfo::scale(80, 10, 5), Material(BLANC));
			bvh_gpu_scene_.add_cube(Transfo::translate(i , j, 105) * Transfo::rotateZ(315) * Transfo::translate(90, 0, 0) * Transfo::scale(80, 10, 5), Material(BLANC));
			bvh_gpu_scene_.add_cylinder(Transfo::translate(i+125, j+125, 115) * Transfo::scale(75, 75, 5), Material(BLANC));
			bvh_gpu_scene_.add_cylinder(Transfo::translate(i , j , 115) * Transfo::scale(65, 65, 5), Material(BLANC));
		}
	bvh_gpu_scene_.add_sphere(Transfo::translate(150, 375, -70) * Transfo::scale(30), Material(JAUNE,0.5f,0.999f));
	bvh_gpu_scene_.add_sphere(Transfo::translate(100, 125, -70) * Transfo::scale(30), Material(OPA(CYAN,0.2f),0.5f,0.9f));
	bvh_gpu_scene_.add_cube(Transfo::translate(125, -125, -80) * Transfo::rotateZ(45) * Transfo::scale(20), Material(ROUGE, 0.1f, 0.2f));

}

int main(int, char**)
{
	Eigen::initParallel();
	RTViewer v;
	v.set_size(1280,1000);
	return v.launch3d();
}

