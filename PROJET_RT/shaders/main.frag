out vec4 frag_out;


void main()   
{
	frag_out = vec4(raytrace(Dir_in,Ori_in),1);
}
