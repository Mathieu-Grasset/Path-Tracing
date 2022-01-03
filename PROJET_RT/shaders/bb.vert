#version 430
#define HALF_TEX_WIDTH 512
layout(location=0) uniform mat4 projectionMatrix;
layout(location=1) uniform mat4 viewMatrix;
layout(location=2) uniform int first;
layout(location=3) uniform int bl;

layout(binding=1) uniform sampler2D bvh_bb;
layout(location=1) in vec3 position_in;

void read_bvh_bb(in int bl, in int i, out vec3 bbmin, out vec3 bbmax)
{
	int l = bl + i/HALF_TEX_WIDTH;
	int c = (i%HALF_TEX_WIDTH)*2;
	bbmin = texelFetch(bvh_bb,ivec2(c,l),0).xyz;
	bbmax = texelFetch(bvh_bb,ivec2(c+1,l),0).xyz;
}

void main()
{
	vec3 B;
	vec3 A;
	read_bvh_bb(bl, first + gl_InstanceID, B,A);
	vec3 C = (A+B)/2.0;
	vec3 D = (B-A)/2;
	vec4 Po4 = viewMatrix * vec4(position_in*D+C,1);
	gl_Position = projectionMatrix * Po4;
}