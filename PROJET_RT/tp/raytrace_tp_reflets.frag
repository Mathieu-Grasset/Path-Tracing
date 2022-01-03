
layout(location=20) uniform vec4 light;
layout(location=21) uniform int NB_BOUNCES;
layout(location=22) uniform float k;

vec3 compute_lighting_diffuse(in vec3 P, in vec3 N, in vec3 color)
{
	vec3 L = (light.xyz - P);
	float d2 = dot(L,L);
	L /= sqrt(d2);
	float lamb = 0.1;
	just_hit_bvh(P+BIAS*N,L);
	if (!hit())
	{
		float kl = light.w/d2;
		lamb += 0.9*max(0.0,dot(L,N))*kl;
	}
	return color * lamb;
}

vec3 raytrace(in vec3 Dir, in vec3 Orig)   
{
	vec3 D = normalize(Dir);
	vec3 O = Orig;				

	vec3 total = vec3(0);
	float attenu = 1.0;
	for(int i=0; (i<NB_BOUNCES)&&(attenu>0.01); ++i)
	{
		traverse_all_bvh(O,D);
		if (!hit())
			return total + attenu * mix (vec3(0.5,0.5,0.9),vec3(1.0,1.0,0.8),max(0.0,D.z));//faux ciel
		//sinon
		vec3 N;
		vec3 P;
		intersection_info(N,P);
		vec4 color = intersection_color_info();
		vec4 mat = intersection_mat_info();
		//illumination sans le rebond
		vec3 local = compute_lighting_diffuse(P,N,color.rgb);

		total += (1.0-mat.r)*local*attenu;
		attenu *= mat.r;
		// on rebondit
		O = P+BIAS*N; // il faut se décaler un peu dans la direction de la normale pour eviter les auto-intersections
		D = reflect(D,N);
	}
	return total;
}

