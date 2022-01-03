
layout(location=20) uniform vec4 light;
layout(location=21) uniform int NB_BOUNCES;
layout(location=22) uniform float k;

vec3 compute_lambert(in vec3 P, in vec3 N, in vec3 color)
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

// approximal frenel 
float rSchlick(in vec3 I, in vec3 N)
{
    float r0 = (k-1) / (k+1);
    r0 *= r0;
        
    // requires n1 <= n2.
    float x  = 1.0 + dot(N,I);
    return clamp(r0 + (1.0 - r0) * x * x * x * x * x, 0.0,1.0);
}

// PILE

vec3 stO[4];
vec3 stD[4];
vec3 stAttenu[4];
int stHead;

void empile(in vec3 o, in vec3 d, in vec3 a)
{
	++stHead;
	stO[stHead] = o; 
	stD[stHead] = d;
	stAttenu[stHead] = a;
}

void depile(out vec3 o, out vec3 d, out vec3 a)
{
	o = stO[stHead]; 
	d = stD[stHead];
	a = stAttenu[stHead];
	stHead--;
}

bool pile_vide()
{
	return stHead<0;
}

void init_pile()
{
	stHead=-1;
}

vec3 raytrace(in vec3 Dir, in vec3 Orig)   
{
	vec3 D = normalize(Dir);
	vec3 O = Orig;

	vec3 total = vec3(0);
	init_pile();
	empile(O,D,vec3(1));
	for(int i=0; (i<NB_BOUNCES)&&(!pile_vide()); ++i)
	{
		//depile
		vec3 attenu;
		depile(O,D,attenu);
		traverse_all_bvh(O,D);
		if (!hit())
			return total + attenu * mix (vec3(0.5,0.5,0.9),vec3(1.0,1.0,0.8),max(0.0,D.z)); //faux ciel
		//sinon
		vec3 N;
		vec3 P;
		intersection_info(N,P);
		vec4 color = intersection_color_info();
		vec4 mat = intersection_mat_info();
		//illumination sans le rebond
		vec3 local = compute_lambert(P,N,color.rgb);
		total += attenu* local * color.a * (1.0-mat.r);

		float rs = rSchlick(D,N);
		// part reflete
		if ((rs>0.0) &&  (mat.r > 0) && (stHead<4))
			empile(P+BIAS*N, reflect(D,N), attenu * mat.r * rs);
		// part traverse	
		if ((rs<1.0) && (color.a<1.0) && (stHead<4))
		{
			O = P-BIAS*N; // Attention au sens
			D = refract(D,N,1.0/k);
			traverse_all_bvh(O,D);
			intersection_info(N,P);
			empile(P+BIAS*N, refract(D,-N, k), attenu * (1.0-color.a) * (1.0-rs));
		}

	}
	return total;
}

