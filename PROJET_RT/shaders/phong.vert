#version 430
layout(location=0) uniform mat4 projectionMatrix;
layout(location=1) uniform mat4 viewMatrix;
layout(location=2) uniform mat3 normalMatrix;

layout(location=1) in vec3 position_in;
layout(location=2) in vec3 normal_in;
out vec3 Po;
out vec3 No;
void main()
{
	No = normalMatrix * normal_in;
	vec4 Po4 = viewMatrix * vec4(position_in,1);
	Po = Po4.xyz;
	gl_Position = projectionMatrix * Po4;
}