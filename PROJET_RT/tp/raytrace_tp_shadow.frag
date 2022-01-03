
layout(location=20) uniform vec4 light;
layout(location=21) uniform int NB_BOUNCES;
layout(location=22) uniform float k;


vec3 compute_lighting(in vec3 P, in vec3 N, in vec3 D, in vec3 color,float spec_exp, float shin)
{
	vec3 L = (light.xyz - P);
	float d2 = dot(L,L);
	L /= sqrt(d2); // normalise L
	float lamb = 0.1; // un peu d'ambiant 
	float spec = 0.0; // pas de speculaire si on est dans l'ombre
	just_hit_bvh(P+BIAS*N,L);
	vec3 R = reflect(-L,N);
	if (!hit()) // pas dans l'ombre ?
	{
		float kl = light.w/d2; // divise par d² pour plus de réalisme physique (et compense en x par light.w)
		lamb += 0.9*max(0.0,dot(L,N))*kl;
		spec = pow(max(-dot(D,R),0.0),spec_exp); // phong speculaire
	}
	return color*lamb + vec3(spec)*shin;
}

vec3 raytrace(in vec3 Dir, in vec3 Orig)   	
{
	const vec3 PosLum = vec3(30,30,100);
	vec3 D = normalize(Dir);
	vec3 O = Orig;				

	traverse_all_bvh(O,D);
	if (!hit())
		return vec3(0);
	vec3 N;
	vec3 P;
	intersection_info(N,P);
	vec4 color = intersection_color_info();
	vec4 mat = intersection_mat_info();
	return compute_lighting(P,N,D,color.rgb,mix(10,1000,mat.g),mat.r);
}
