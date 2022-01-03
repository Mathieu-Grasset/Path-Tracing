#version 430
#define TEX_WIDTH 1024
layout(location=0) uniform mat4 projectionMatrix;
layout(location=1) uniform mat4 viewMatrix;
layout(location=2) uniform mat3 normalMatrix;

layout(location=6) uniform int mesh_line;

layout(binding=4) uniform isampler2D tri_ind;
layout(binding=5) uniform sampler2D tri_pos;
layout(binding=6) uniform sampler2D tri_norm;
out vec3 Po;
out vec3 No;

void read_triangle_PN(in int tri, out vec3 Pa, out vec3 Na, out vec3 Pb, out vec3 Nb, out vec3 Pc, out vec3 Nc)
{
	tri++;
	ivec3 indices = texelFetch(tri_ind,ivec2(tri%TEX_WIDTH,mesh_line+tri/TEX_WIDTH),0).xyz;
	ivec2 coord = ivec2(indices.x%TEX_WIDTH,indices.x/TEX_WIDTH);
	Pa = texelFetch(tri_pos,coord,0).xyz;
	Na = texelFetch(tri_norm,coord,0).xyz;
	coord = ivec2(indices.y%TEX_WIDTH,indices.y/TEX_WIDTH);
	Pb = texelFetch(tri_pos,coord,0).xyz;
	Nb = texelFetch(tri_norm,coord,0).xyz;
	coord = ivec2(indices.z%TEX_WIDTH,indices.z/TEX_WIDTH);
	Pc = texelFetch(tri_pos,coord,0).xyz;
	Nc = texelFetch(tri_norm,coord,0).xyz;
}


void main()
{
	vec3 P[3];
	vec3 N[3];
	read_triangle_PN(gl_InstanceID,P[0],N[0],P[1],N[1],P[2],N[2]);

	No = normalMatrix * N[gl_VertexID];
	vec4 Po4 = viewMatrix * vec4(P[gl_VertexID],1);
	Po = Po4.xyz;
	gl_Position = projectionMatrix * Po4;
}