#version 430
precision highp float;
in vec3 Po;
in vec3 No;
out vec4 frag_out;
layout(location=3) uniform vec3 light_pos;
layout(location=4) uniform vec3 color;
const float specness = 150.0f;
const float shininess = 0.7;

void main()
{
	vec3 N = normalize(No);
	vec3 L = normalize(light_pos-Po);
	if (gl_FrontFacing==false)
		N *= -1.0f;

	float lamb = 0.2+0.8*max(0.0,dot(N,L));
	frag_out = vec4(color*lamb,1);
}