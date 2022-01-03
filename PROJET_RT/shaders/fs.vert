#version 430
void main()
{
	vec2 tc = vec2(gl_VertexID%2,gl_VertexID/2);
	gl_Position = vec4(2.0*tc-1.0,0,1);
}
