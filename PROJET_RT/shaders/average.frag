#version 430
out vec3 frag_out;
layout(binding=1) uniform sampler2D TU_rt;
layout(location=4) uniform float subsampling;
layout(location=5) uniform float k;

void main()
{
	vec3 c = texelFetch(TU_rt,ivec2(gl_FragCoord.xy/subsampling),0).rgb * k;
	frag_out = c;
}
