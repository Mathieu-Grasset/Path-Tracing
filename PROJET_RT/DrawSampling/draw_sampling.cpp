#include <thread>
#include <fstream>
#include<iterator>

#include <shader_program.h>
#include <vao.h>
#include <mesh.h>
#include <texture2d.h>
#include <fbo.h>
#include <gl_viewer.h>

#include <chrono>
#include <cstdlib>
#include <vector>
#include <string>

using namespace EZCOGL;

// for easy shader switching (see key_press_ogl)
SrcLoader loader({ "hsphere_wrong_sampling.vert","hsphere_wrong2_sampling.vert"}, TP_SHADER_PATH);

static const std::string frame_vert = R"(
#version 430
layout(location=0) uniform mat4 pvMatrix;
flat out vec3 color;
void main()
{
	color = vec3(0.2);
	vec3 P = vec3(0);
	int idir = gl_VertexID/2;
	color[idir] = 1.0;
	P[idir] = 1.5*float(gl_VertexID%2);
	gl_Position = pvMatrix * vec4(P,1);
}
)";

static const std::string N_vert = R"(
#version 430
layout(location=0) uniform mat4 pvMatrix;
layout(location=1) uniform vec3 N;
layout(location=2) uniform vec3 col;
layout(location=3) uniform int nb;
flat out vec3 color;
void main()
{
	color = col;
	vec3 P = 2.0*N*float(gl_VertexID)/float(nb);
	gl_Position = pvMatrix * vec4(P,1);
}
)";

static const std::string frame_frag = R"(
#version 430
out vec3 frag_out;
flat in vec3 color;

void main()
{
	frag_out = color;
}
)";


class SamplingViewer: public GLViewer
{
	SP_ShaderProgram prg;
	SP_ShaderProgram prg_fr;
	SP_ShaderProgram prg_N;

	GLVec3 Normal_;
	int NBS_;
	float p_;

public:
	SamplingViewer();
	void init_ogl() override;
	void draw_ogl() override;
	void interface_ogl() override;
	void key_press_ogl(int32_t k) override;
};
 
SamplingViewer::SamplingViewer() :
	Normal_(1, 1, 1),
	NBS_(10),
	p_(0.5f)
{
}

void SamplingViewer::key_press_ogl(int32_t k)
{
	 // Warning Layout toujours en QWERTY !!
	switch (k)
	{
	case 'O': //
		loader.prev();
		prg = ShaderProgram::create({ {GL_VERTEX_SHADER,loader.load("sampling_base.vert") + loader.load() },
										{GL_FRAGMENT_SHADER,loader.load("sampling.frag") } }, "Sampling");
		break;
	case 'P':
		loader.next();
		prg = ShaderProgram::create({ {GL_VERTEX_SHADER,loader.load("sampling_base.vert") + loader.load() },
										{GL_FRAGMENT_SHADER,loader.load("sampling.frag") } }, "Sampling");
		break;
	}

}

void SamplingViewer::init_ogl()
{
	prg_fr = ShaderProgram::create({ {GL_VERTEX_SHADER, frame_vert},
										{GL_FRAGMENT_SHADER, frame_frag } }, "Frame");
	prg_N = ShaderProgram::create({ {GL_VERTEX_SHADER, N_vert},
										{GL_FRAGMENT_SHADER, frame_frag } }, "Frame");
	prg = ShaderProgram::create({ {GL_VERTEX_SHADER, loader.load("sampling_base.vert") + loader.load()},
									{GL_FRAGMENT_SHADER, loader.load("sampling.frag") } }, "Sampling");
	
	set_scene_center(GLVec3(0,0,0));
	set_scene_radius(2);

}

void SamplingViewer::draw_ogl()
{
	GLMat4 pvm = this->get_projection_matrix() * this->get_modelview_matrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	VAO::none()->bind();
	prg_fr->bind();
	set_uniform_value(0, pvm);
	glDrawArrays(GL_LINES, 0, 6);

	auto normal1 = Normal_.normalized();

	prg_N->bind();
	set_uniform_value(0, pvm);
	set_uniform_value(1, normal1);
	set_uniform_value(2, GLVec3(0, 1, 1));
	set_uniform_value(3, 32);
	glDrawArrays(GL_LINES, 0, 32);

	glPointSize(1.0);
	prg->bind();
	set_uniform_value(0, pvm);
	set_uniform_value(2, normal1);
	set_uniform_value(3,GLVec3(float(rand())/RAND_MAX, float(rand()) / RAND_MAX,float(rand()) / RAND_MAX));
	set_uniform_value(4, GLVec3(1,1,0));
	glDrawArrays(GL_POINTS, 0, 1000*NBS_);


}


void SamplingViewer::interface_ogl()
{
// Pour grossir l'interace si trop petit
//	ImGui::GetIO().FontGlobalScale = 2.0f;
	ImGui::Begin("Sampling",nullptr, ImGuiWindowFlags_NoSavedSettings);
	ImGui::SetWindowSize({500,0});
	ImGui::SliderInt("NB", &NBS_, 0, 100);
	ImGui::SliderFloat3("N", &(Normal_[0]), -1.0, 1.0);
	ImGui::SliderFloat("Param", &p_, 0, 1.0);
	ImGui::End();
}



int main(int, char**)
{
	Eigen::initParallel();
	SamplingViewer v;
	v.set_size(1280,1000);
	return v.launch3d();
}

