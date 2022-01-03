#version 430
layout(location=6) uniform mat4 invV;
layout(location=10) uniform mat4 invPV;

out vec3 Ori_in;
out vec3 Dir_in;
out vec2 screen_tc;
flat out int instID;
void main()
{
	instID = gl_InstanceID;
	screen_tc = vec2(gl_VertexID%2,gl_VertexID/2);
	vec2 c = 2.0 * screen_tc - 1.0;
	
	vec4 P4 = invV*vec4(0,0,0,1);
	vec4 Q4 = invPV*vec4(c,1,1);

	Ori_in = P4.xyz;
	Dir_in = normalize(Q4.xyz / Q4.w - P4.xyz);

	gl_Position = vec4(c,0,1);
}