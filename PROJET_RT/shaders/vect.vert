#version 430
layout(location=1) in vec3 position_vertex;
layout(location=1) uniform mat4 pmvMatrix;
void main()
{
	gl_Position = pmvMatrix*vec4(position_vertex,1);
}
