#version 430
precision highp float;
layout(location=4) uniform vec4 color;
out vec3 frag_out;
void main()
{
	frag_out = color.rgb;
}