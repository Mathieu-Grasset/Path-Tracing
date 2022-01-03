#version 430

in vec3 Ori_in;
in vec3 Dir_in;
in vec2 screen_tc;
flat in int instID;

#define FLT_MAX 3.402823e38
const float PI = 2.0*acos(0.0);
const float DPI = 2.0*PI;
const float HPI = 0.5*PI;

const float EPSILON = 1e-10;
const float BIAS = 1e-2;

const int TEX_WIDTH = 1024;
const int PRIM_WIDTH = 16;

const int HALF_TEX_WIDTH = TEX_WIDTH/2;
const int PRIMS_PER_LINE = TEX_WIDTH/PRIM_WIDTH;


layout(location=1) uniform int nb_prims;
layout(location=2) uniform int bvh_depth;
layout(location=3) uniform bool flat_face;
layout(location=4) uniform int numero_pass;
layout(location=5) uniform float date;

layout(binding=1) uniform sampler2D primsData;
layout(binding=2) uniform sampler2D bvh_bb;
layout(binding=3) uniform isampler2D bvh_ind;

layout(binding=4) uniform isampler2D tri_ind;
layout(binding=5) uniform sampler2D tri_pos;
layout(binding=6) uniform sampler2D tri_norm;


#define CODE_MESH 0
#define CODE_SPHERE 1
#define CODE_CUBE 2
#define CODE_CYLINDER 3
#define CODE_CONE 4
#define CODE_ORIENTED_QUAD 5

// float random_seed;
// float random_inc;

// void srand()
// {
// 	// random_seed = (float(numero_pass+instID)+date)*dot(screen_tc,vec2(12.9898,78.233));
// 	// random_inc = PI/37.0;
// }

// float random_float()
// {
// 	float r = fract(sin(random_seed)*43758.5453123);
// 	random_seed += random_inc;
// 	random_inc *= (1.5-r);
// 	return r;
// }


uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

uvec3 pcg3d(uvec3 v) {

    v = v * 1664525u + 1013904223u;

    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;

    v ^= v >> 16u;

    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;

    return v;
}
uint hash( uvec3 v){ return hash( v.x ^ hash(v.y ^ hash(v.z))); }

uint xxhash32(uvec3 p)
{
    const uint PRIME32_2 = 2246822519U, PRIME32_3 = 3266489917U;
	const uint PRIME32_4 = 668265263U, PRIME32_5 = 374761393U;
	uint h32 =  p.z + PRIME32_5 + p.x*PRIME32_3;
	h32 = PRIME32_4*((h32 << 17) | (h32 >> (32 - 17)));
	h32 += p.y * PRIME32_3;
	h32 = PRIME32_4*((h32 << 17) | (h32 >> (32 - 17)));
    h32 = PRIME32_2*(h32^(h32 >> 15));
    h32 = PRIME32_3*(h32^(h32 >> 13));
    return h32^(h32 >> 16);
}

uvec3 random_seed;

void srand()
{
	vec3 w = vec3(screen_tc.x,+float(numero_pass)*3.14+date,screen_tc.y) * float(1+numero_pass)*1.125;
	random_seed = floatBitsToUint(w);

}

float random_float()
{
	uint m = xxhash32(random_seed);
	const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0
    float  f = uintBitsToFloat( m );       // Range [1:2]
	random_seed += uvec3(11u,43u,67u);
	// random_seed_inc = uvec3(~random_seed_inc.z,~random_seed_inc.x,~random_seed_inc.y);
    return f - 1.0; 
}


vec2 random_vec2()
{
	return vec2(random_float(),random_float());
}

vec3 random_vec3()
{
	return vec3(random_float(),random_float(),random_float());
}


ivec3 read_mesh_info(in int mesh_line)
{
	return texelFetch(tri_ind,ivec2(0,mesh_line),0).rgb;
}


void read_triangle_pos(in int mesh_line, in int tri, out vec3 Pa, out vec3 Pb, out vec3 Pc)
{
	tri++; // skip info
	ivec3 indices = texelFetch(tri_ind,ivec2(tri%TEX_WIDTH,mesh_line+tri/TEX_WIDTH),0).rgb;

	Pa = texelFetch(tri_pos, ivec2(indices.x%TEX_WIDTH,indices.x/TEX_WIDTH),0).xyz;
	Pb = texelFetch(tri_pos,ivec2(indices.y%TEX_WIDTH,indices.y/TEX_WIDTH),0).xyz;
	Pc = texelFetch(tri_pos,ivec2(indices.z%TEX_WIDTH,indices.z/TEX_WIDTH),0).xyz;
}


void read_triangle_PN(in int mesh_line, in int tri, out vec3 Pa, out vec3 Na, out vec3 Pb, out vec3 Nb, out vec3 Pc, out vec3 Nc)
{
	tri++; // skip info
	ivec3 indices = texelFetch(tri_ind,ivec2(tri%TEX_WIDTH,mesh_line+tri/TEX_WIDTH),0).xyz;
	ivec2 coord = ivec2(indices.x%TEX_WIDTH,indices.x/TEX_WIDTH);
	Pa = texelFetch(tri_pos,coord,0).xyz;
	Na = texelFetch(tri_norm,coord,0).xyz;
	coord = ivec2(indices.y%TEX_WIDTH,indices.y/TEX_WIDTH);
	Pb = texelFetch(tri_pos,coord,0).xyz;
	Nb = texelFetch(tri_norm,coord,0).xyz;
	coord = ivec2(indices.z%TEX_WIDTH,indices.z/TEX_WIDTH);
	Pc = texelFetch(tri_pos,coord,0).xyz;
	Nc = texelFetch(tri_norm,coord,0).xyz;
}


int read_primitive_type(in int i, out int mesh_line)
{
	int l = i/PRIMS_PER_LINE;
	int c= (i%PRIMS_PER_LINE)*PRIM_WIDTH;
	vec4 texel = texelFetch(primsData,ivec2(c+12,l),0);
	mesh_line = int(texel.y);
	return int(texel.x);
}

int read_primitive_type_simple(in int i)
{
	int l = i/PRIMS_PER_LINE;
	int c= (i%PRIMS_PER_LINE)*PRIM_WIDTH;
	vec4 texel = texelFetch(primsData,ivec2(c+12,l),0);
	return int(texel.x);
}

mat4 read_transfo(in int i)
{
	int l = i/PRIMS_PER_LINE;
	int c= (i%PRIMS_PER_LINE)*PRIM_WIDTH;
	return mat4(texelFetch(primsData,ivec2(c,l),0),
		texelFetch(primsData,ivec2(c+1,l),0),
		texelFetch(primsData,ivec2(c+2,l),0),
		texelFetch(primsData,ivec2(c+3,l),0));
}


mat4 read_inv_transfo(in int i)
{
	int l = i/PRIMS_PER_LINE;
	int c= (i%PRIMS_PER_LINE)*PRIM_WIDTH;
	return mat4(texelFetch(primsData,ivec2(c+4,l),0),
		texelFetch(primsData,ivec2(c+5,l),0),
		texelFetch(primsData,ivec2(c+6,l),0),
		texelFetch(primsData,ivec2(c+7,l),0));
}


mat4 read_mesh_transfo(in int i)
{
	int l = i/PRIMS_PER_LINE;
	int c= (i%PRIMS_PER_LINE)*PRIM_WIDTH;
	return mat4(texelFetch(primsData,ivec2(c+8,l),0),
		texelFetch(primsData,ivec2(c+9,l),0),
		texelFetch(primsData,ivec2(c+10,l),0),
		texelFetch(primsData,ivec2(c+11,l),0));
}


vec4 read_color(in int i)
{
	int l = i/PRIMS_PER_LINE;
	int c= (i%PRIMS_PER_LINE)*PRIM_WIDTH;
	return texelFetch(primsData,ivec2(c+13,l),0);
}

vec4 read_material(in int i)
{
	int l = i/PRIMS_PER_LINE;
	int c= (i%PRIMS_PER_LINE)*PRIM_WIDTH;
	return texelFetch(primsData,ivec2(c+14,l),0);
}
/*
float read_index(in int i)
{
	int l = i/PRIMS_PER_LINE;
	int c= (i%PRIMS_PER_LINE)*PRIM_WIDTH;
	return int(texelFetch(primsData,ivec2(c+15,l),0).x);
}
*/

int read_bvh_ind(in int bl, in int i)
{
	int v = texelFetch(bvh_ind,ivec2(i%TEX_WIDTH,i/TEX_WIDTH + bl),0).x;
	return v;
}

void read_bvh_bb(in int bl, in int i, out vec3 bbmin, out vec3 bbmax)
{
	int l = i/HALF_TEX_WIDTH + bl;
	int c = (i%HALF_TEX_WIDTH)*2;
	bbmin = texelFetch(bvh_bb,ivec2(c,l),0).xyz;
	bbmax = texelFetch(bvh_bb,ivec2(c+1,l),0).xyz;	
}

struct sInter
{
	vec3 pl;
	vec3 pg;
	float dist;
	int dir;
	int index;
	int tri_index;
	int mesh_line;
	int shape;
} closest_intersection = { vec3(0),vec3(0),FLT_MAX,-1,-1,-1,-1,-1};

bool hit_only=false;

bool hit() {return closest_intersection.shape >= 0;}

bool intersect_bvm(int bl, in int i, in vec3 O, in vec3 D, in vec3 Ol, in mat4 trf)
{
	vec3 bbmin;
	vec3 bbmax;
	read_bvh_bb(bl,i,bbmin,bbmax);
	vec3 center = (bbmin+bbmax)/2.0;
	vec3 width = 0.5*(bbmax- bbmin);
	
	vec3 Oi = (O-center)/width;
	vec3 Di = D/width;

	if (all(lessThan(abs(Oi),vec3(1))))
		return true;


	float al = FLT_MAX;
	for(int c=0; c<6; ++c)
	{
		int c0 = c/2;
		if (abs(Di[c0])> EPSILON)
		{
			int c1 =(c0+1)%3;
			int c2 =(c0+2)%3;
			float cd = -1.0+2.0*float(c%2);
			float a = (cd - Oi[c0])/Di[c0];
			if ((a>EPSILON) && (abs(Oi[c1]+a*Di[c1]) <= 1.0) && (abs(Oi[c2]+a*Di[c2]) <= 1.0))
				if (a <al)
					al = a;
		}
	}
	if (al<FLT_MAX)
	{
		vec3 Pl = Oi + al * Di;
		vec3 Pg = (trf*vec4(Pl*width + center,1)).xyz;
		float dist = distance(Ol,Pg);
		return (dist <= closest_intersection.dist);
	}
	return false;
}


bool intersect_bv(in int bl, in int i, in vec3 O, in vec3 D, in vec3 Ol)
{
	vec3 bbmin;
	vec3 bbmax;
	read_bvh_bb(bl,i,bbmin,bbmax);
	vec3 center = (bbmin+bbmax)/2.0;
	vec3 width = 0.5*(bbmax- bbmin);
	
	vec3 Oi = (O-center)/width;
	vec3 Di = D/width;

	if (all(lessThan(abs(Oi),vec3(1))))
		return true;

	bool res=false;
	float al = FLT_MAX;
	for(int c=0; c<6; ++c)
	{
		int c0 = c/2;
		if (abs(Di[c0])> EPSILON)
		{
			int c1 =(c0+1)%3;
			int c2 =(c0+2)%3;
			float cd = -1.0+2.0*float(c%2);
			float a = (cd - Oi[c0])/Di[c0];
			if ((a>EPSILON) && (abs(Oi[c1]+a*Di[c1]) <= 1.0) && (abs(Oi[c2]+a*Di[c2]) <= 1.0))
				if (a<al)
					al = a;
		}
	}

	if (al<FLT_MAX)
	{
		vec3 Pg = (al*Di+Oi)*width + center;
		float dist = distance(Ol,Pg);
		return (dist <= closest_intersection.dist);
	}
	return false;
}

void Triangle_intersect(in int mesh_line, in int tri_index, in int bb_index, in vec3 O, in vec3 D, in vec3 Ol, in mat4 trf) 
{
	vec3 vA;
	vec3 vB;
	vec3 vC;
	read_triangle_pos( mesh_line, tri_index, vA,vB,vC);

	vec3 edge1 = vB - vA;
	vec3 edge2 = vC - vA;
	vec3 h = cross(D,edge2);
	float det = dot(edge1,h);
	if (abs(det) < EPSILON)
		return;
	float invdet = 1.0/det;
	vec3 s = O - vA;
	float u = dot(s,h)*invdet;
	if (u < 0.0 || u > 1.0)
		return;
	vec3 q = cross(s,edge1);
	float v = dot(D,q)*invdet;
	if (v < 0.0 || (u + v) > 1.0)
		return;
	float a = dot(edge2,q) * invdet;

    if 	( a > EPSILON)
	{
		vec3 Pl = O+a*D;
		vec3 Pg = (trf*vec4(Pl,1)).xyz;

		float dist = distance( Ol, Pg);
		if (dist<closest_intersection.dist)
    	{
    		closest_intersection.pl = Pl;
    		closest_intersection.pg = Pg;
    		closest_intersection.dist = dist;
			closest_intersection.tri_index = tri_index;
			closest_intersection.index = bb_index;
			closest_intersection.mesh_line = mesh_line;
			closest_intersection.shape = CODE_MESH;
			closest_intersection.dir = 0;
		}
    }
}

void Sphere_intersect(in int index, in vec3 O, in vec3 D, in vec3 Ol)
{
	float OO = dot(O,O);
	float OD = dot(O,D);
	float D2 = dot(D,D);
	float delta4 = OD*OD - D2*(OO-1.0);
	if (delta4>0)
	{
		float a = -(OD+sqrt(delta4))/D2;
		if (a>EPSILON) 
		{
			mat4 trf = read_transfo(index);
			vec3 Pl = O+a*D;
			vec3 Pg = (trf*vec4(Pl,1)).xyz;
			float dist = distance(Ol,Pg);
			if ( dist < closest_intersection.dist)
			{
				closest_intersection.dist = dist;
				closest_intersection.pl = Pl;
				closest_intersection.pg = Pg;
				closest_intersection.index = index;
				closest_intersection.shape = CODE_SPHERE;
				closest_intersection.dir = 0;
			}
		}
		a = -(OD-sqrt(delta4))/D2;
		if (a>EPSILON) 
		{
			mat4 trf = read_transfo(index);
			vec3 Pl = O+a*D;
			vec3 Pg = (trf*vec4(Pl,1)).xyz;
			float dist = distance(Ol,Pg);
			if ( dist < closest_intersection.dist)
			{
				closest_intersection.dist = dist;
				closest_intersection.pl = Pl;
				closest_intersection.pg = Pg;
				closest_intersection.index = index;
				closest_intersection.shape = 1;
				closest_intersection.dir = 0;
			}
		}
	}
}

void OrientedQuad_intersect(in int index, in vec3 O, in vec3 D, in vec3 Ol)
{
 	if (D.z > -EPSILON)
 		return;

//	P.z = 0 = O.z +a . D.z
// 	-O.z = a . D.z
//  -O.z/D.z = a
 	float a = -O.z/D.z;
 	vec3 Pl = O+a*D;

 	if (abs(Pl.x)>1 || abs(Pl.y)>1)
 		return;

	mat4 trf = read_transfo(index);
	
	vec3 Pg = (trf*vec4(Pl,1)).xyz;
	float dist = distance(Ol,Pg);
	if ( dist < closest_intersection.dist)
	{
		closest_intersection.pl = Pl;
		closest_intersection.pg = Pg;
		closest_intersection.dist = dist;
		closest_intersection.index = index;
		closest_intersection.shape = CODE_ORIENTED_QUAD;
		closest_intersection.dir = 0;
	}
}

void Cube_intersect(in int index, in vec3 O, in vec3 D, in vec3 Ol)
{
	float al = FLT_MAX;
	int cl =0;
	for(int c=0; c<6; ++c)
	{
		int c0 = c/2;
		if (abs(D[c0])> EPSILON)
		{
			int c1 =(c0+1)%3;
			int c2 =(c0+2)%3;
			float cd = -1.0+2.0*float(c%2);

			float a = (cd - O[c0])/D[c0];
			if ((a>EPSILON) && (abs(O[c1]+a*D[c1]) <= 1.0) && (abs(O[c2]+a*D[c2]) <= 1.0))
			{
				if (a < al)
				{
					al = a;
					cl =c;
				}
			}
		}
	}
	if (al < FLT_MAX)
	{
		mat4 trf = read_transfo(index);
		vec3 Pl = O+al*D;
		vec3 Pg = (trf*vec4(Pl,1)).xyz;
		float dist = distance(Ol,Pg);
		if ( dist < closest_intersection.dist)
		{
			closest_intersection.pl = Pl;
			closest_intersection.pg = Pg;
			closest_intersection.dist = dist;
			closest_intersection.dir = cl;
			closest_intersection.index = index;
			closest_intersection.shape = CODE_CUBE;
		}
	}
}


void Cylinder_intersect(in int index, in vec3 O, in vec3 D, in vec3 Ol)
{
	int cl = -1;
	float al = FLT_MAX;
	if (abs(D.z)> EPSILON)
	{
		float a = (-1.0 - O.z)/D.z;
		if (a>EPSILON)
		{
			vec2 R = O.xy +a*D.xy;
			if ((dot(R,R)< 1.0) && (a<al))
			{
				cl = 0;
				al = a;
			}
		}
		a = (1.0 - O.z)/D.z;
		if (a>EPSILON)
		{
			vec2 R = O.xy +a*D.xy;
			if ((dot(R,R)< 1.0) && (a<al))
			{
				cl = 1;
				al = a;
			}
		}
	}

	float O2 = dot(O.xy,O.xy);
	float OD = dot(O.xy,D.xy);
	float D2 = dot(D.xy,D.xy);
	float delta4 = OD*OD - D2*(O2-1);
	if (delta4 > 0.0)
	{
		float a = -(OD+sqrt(delta4))/D2;
		if ((a > EPSILON) && (a<al))
		{
			float z = O.z + a* D.z;
			if (abs(z) < 1.0)
			{
				cl = 2;
				al = a;
			}
		}
	}

	if (al < FLT_MAX)
	{
		mat4 trf = read_transfo(index);
		vec3 Pl = O+al*D;
		vec3 Pg = (trf*vec4(Pl,1)).xyz;
		float dist = distance(Ol,Pg);
		if ( dist < closest_intersection.dist)
		{
			closest_intersection.pl = Pl;
			closest_intersection.pg = Pg;
			closest_intersection.dist = dist;
			closest_intersection.dir = cl;
			closest_intersection.index = index;
			closest_intersection.shape = CODE_CYLINDER;
		}
	}
}

void Cone_intersect(in int index, in vec3 O, in vec3 D, in vec3 Ol)
{

	int cl = -1;
	float tl = FLT_MAX;

	if (abs(D.z)> EPSILON)
	{
		float t0 = (-1.0 - O.z)/D.z;
		if (t0>EPSILON)
		{
			vec2 R = O.xy +t0*D.xy;
			if ((dot(R,R)< 1.0) && (t0<tl))
			{
				cl = 0;
				tl = t0;
			}
		}
	}

	vec3 co = O;
	co.z -= 1.0;
	float a = D.z*D.z - 0.8;
	float b = 2.0 * (D.z*co.z - dot(D,co)*0.8); 
	float c = co.z*co.z - dot(co,co)*0.8;

	float det = b*b - 4.0*a*c;
	if (det > 0.0)
	{
		det = sqrt(det);
		float t1 = (-b - det) / (2.0 * a);
		if (abs(O.z + t1*D.z)>1)
			t1 = FLT_MAX;
		float t2 = (-b + det) / (2.0 * a);
		if (abs(O.z + t2*D.z)>1)
			t2 = FLT_MAX;
		float t = min(t1,t2);	
    	if (t<tl)
    	{
			cl = 2;
			tl = t;
		}
	}
	
	if (tl< FLT_MAX)
	{
		vec3 Pl = O + tl*D;
		mat4 trf = read_transfo(index);
		vec3 Pg = (trf * vec4(Pl,1)).xyz;
		float dist = distance(Ol,Pg);
		if ( dist < closest_intersection.dist)
		{
			closest_intersection.pl = Pl;
			closest_intersection.pg = Pg;
			closest_intersection.dist = dist;
			closest_intersection.dir = cl;
			closest_intersection.index = index;
			closest_intersection.shape = CODE_CONE;
		}
	}
	
}

void Mesh_intersect(in int mesh_line, in int bb_index, in vec3 O, in vec3 D, in vec3 Ol)
{
	int stack_ind[29];
	int head=1;
	stack_ind[0]=0;

	ivec3 mesh_info = read_mesh_info(mesh_line);
	int max_line_depth = mesh_info.z;

	mat4 trfm = read_mesh_transfo(bb_index);
	mat4 trfp = read_transfo(bb_index);

	while (head>0)
	{
		head--;
		int i = stack_ind[head];
		if (i >= max_line_depth)
		{
			int p = read_bvh_ind(mesh_info.y,i-max_line_depth);
			if (p>=0)
			{
				Triangle_intersect(mesh_line,p,bb_index,O,D,Ol,trfm);
				if (hit_only && closest_intersection.shape>=0)
						return;
			}
		}
		else 
		{
			int j = 2*i+1;
			if (intersect_bvm(mesh_info.x,j,O,D,Ol,trfm))
				stack_ind[head++] = j;
			j++;
			if (intersect_bvm(mesh_info.x,j,O,D,Ol,trfm))
				stack_ind[head++] = j;
		}
	} 
}


void intersect_prim(in int i, in vec3 O, in vec3 D)
{
	int mesh_line = -1;
	int prim_t = read_primitive_type(i,mesh_line);
	if (prim_t <0) return;
	mat4 inv = read_inv_transfo(i);
	vec3 Oi = (inv*vec4(O,1)).xyz;
	vec3 Di = normalize(inv*vec4(D,0)).xyz;

	switch(prim_t)
	{
		case CODE_MESH: Mesh_intersect(mesh_line,i,Oi,Di,O);
				break;
		case CODE_SPHERE: Sphere_intersect(i,Oi,Di,O);
				break;
		case CODE_CUBE: Cube_intersect(i,Oi,Di,O);
				break;
		case CODE_CYLINDER: Cylinder_intersect(i,Oi,Di,O);
				break;
		case CODE_CONE: Cone_intersect(i,Oi,Di,O);
				break;	
		case CODE_ORIENTED_QUAD: OrientedQuad_intersect(i,Oi,Di,O);
				break;
	}
}


void reset_inter()
{
	closest_intersection.index = -1;
/*	closest_intersection.bb_index = -1*/;
	closest_intersection.dist = FLT_MAX;
	closest_intersection.tri_index = -1;
	closest_intersection.shape = -1;
	closest_intersection.dir = -1;
}

void intersect_one_prim(in int i, in vec3 O, in vec3 D)
{
	reset_inter();
	hit_only = false;
	intersect_prim(i,O,D);
}


void hit_one_prim(in int i, in vec3 O, in vec3 D)
{
	reset_inter();
	hit_only = true;
	intersect_prim(i,O,D);
}


void intersect_bvh(in vec3 O,in vec3 D)
{
	int stack_ind[29];
	int head=1;
	stack_ind[0]=0;

	reset_inter();

// TODO REPLACE DEPTH_BVH BY MAX_LINE_DEPTH ?
	int max_line_depth = int(pow(2.0,float(bvh_depth))-1.0);
	
	while (head>0)
	{
		head--;
		int i = stack_ind[head];
		if (i >= max_line_depth)
		{
			int p = read_bvh_ind(0,i-max_line_depth);
			if (p>=0)
			{
				intersect_prim(p,O,D);
				if (hit_only && closest_intersection.shape>=0)
						return;
			}
		}
		else 
		{
			int j = 2*i+1;
			if (intersect_bv(0,j,O,D,O))
				stack_ind[head++] = j;
			j++;
			if (intersect_bv(0,j,O,D,O))
				stack_ind[head++] = j;
		}
	} 
}

void just_hit_bvh(in vec3 O,in vec3 D)
{
	hit_only = true;
	intersect_bvh(O,D);
}

void traverse_all_bvh(in vec3 O,in vec3 D)
{
	hit_only = false;
	intersect_bvh(O,D);
}

void mesh_inter_geom_info(out vec3 N, out vec3 Pg)
{
	vec3 A;
	vec3 B;
	vec3 C; 

	vec3 nA,nB,nC;
	read_triangle_PN(closest_intersection.mesh_line, closest_intersection.tri_index, A,nA, B,nB, C,nC);

	mat4 transfo = read_mesh_transfo(closest_intersection.index);
	Pg = closest_intersection.pg;

	if (flat_face)
		N = normalize((transfo*vec4(closest_intersection.pl+cross(B-A,C-A),1)).xyz - Pg);
	else
	{
		// compute triangle areas
		vec3 PA = A-closest_intersection.pl;
		vec3 PB = B-closest_intersection.pl;
		vec3 PC = C-closest_intersection.pl;
		float tA = length(cross(PB,PC));
		float tB = length(cross(PA,PC));
		float tC = length(cross(PA,PB));

		vec3 No = nA*tA + nB*tB + nC*tC;
		N = normalize((transfo*vec4(closest_intersection.pl+No,1)).xyz - Pg);
	}
}

void sphere_inter_geom_info(out vec3 N, out vec3 Pg)
{
	Pg = closest_intersection.pg;
	vec3 Pn = 2.0*closest_intersection.pl;
	mat4 transfo = read_transfo(closest_intersection.index);
	N = normalize((transfo*vec4(Pn,1)).xyz - Pg);
}

void cube_inter_geom_info(out vec3 N, out vec3 Pg)
{
	Pg = closest_intersection.pg;
	vec3 No = vec3(0);
	No[closest_intersection.dir/2] = (closest_intersection.dir%2 != 0)?1.0:-1.0;
	mat4 transfo = read_transfo(closest_intersection.index);
	N = normalize((transfo*vec4(closest_intersection.pl+No,1)).xyz - Pg);
}

void cylinder_inter_geom_info(out vec3 N, out vec3 Pg)
{
	Pg = closest_intersection.pg;
	vec3 No = vec3(0);
	if (closest_intersection.dir<2)
		No.z = (closest_intersection.dir%2 != 0)?1.0:-1.0;
	else
		No = vec3(closest_intersection.pl.xy,0.0);
	mat4 transfo = read_transfo(closest_intersection.index);
	N = normalize((transfo*vec4(closest_intersection.pl+No,1)).xyz - Pg);
}

void cone_inter_geom_info(out vec3 N, out vec3 Pg)
{
	Pg = closest_intersection.pg;
	mat4 transfo = read_transfo(closest_intersection.index);
	switch(closest_intersection.dir)
	{
		case 0:
			N = normalize((transfo*vec4(closest_intersection.pl.x,closest_intersection.pl.y,closest_intersection.pl.z-1.0,1)).xyz - Pg);
			break;
		case 1:
			//N = normalize((transfo*vec4(closest_intersection.pl.x,closest_intersection.pl.y,closest_intersection.pl.z+1.0,1)).xyz - Pg);
			N =vec3(0,0,0);
			break;
		case 2:
			{
				vec3 No = vec3(closest_intersection.pl.xy,length(closest_intersection.pl.xy)/2.0);
				N = normalize((transfo*vec4(closest_intersection.pl+No,1)).xyz - Pg);
			}
			break;
	}

}

void oriented_quad_inter_geom_info(out vec3 N, out vec3 Pg)
{
	Pg = closest_intersection.pg;
	mat4 transfo = read_transfo(closest_intersection.index);
	vec3 No = vec3(0,0,1);
	N = normalize((transfo*vec4(closest_intersection.pl+No,1)).xyz - Pg);
}
			


void intersection_info(out vec3 N, out vec3 Pg)
{
	switch(closest_intersection.shape)
	{
		case CODE_MESH: 
			mesh_inter_geom_info(N,Pg);
			break;
		case CODE_SPHERE:
			sphere_inter_geom_info(N,Pg);
			break;
		case CODE_CUBE:
			cube_inter_geom_info(N,Pg);
			break;
		case CODE_CYLINDER:
			cylinder_inter_geom_info(N,Pg);
			break;
		case CODE_CONE:
			cone_inter_geom_info(N,Pg);
			break;
		case CODE_ORIENTED_QUAD:
			oriented_quad_inter_geom_info(N,Pg);
			break;
	}
}
                                                                                                                                                                                  
vec4 intersection_color_info()                                                                  
{
	return read_color(closest_intersection.index);
}

vec4 intersection_mat_info()
{
	return read_material(closest_intersection.index);
}
/*
vec3 random_sample_cube()
{
	return random_vec3()*2.0-1.0;
}

vec3 random_sample_oriented_quad()
{
	return vec3(random_vec2()*2.0-1.0,0.0);
}

vec3 random_sample_sphere()
{
	vec3 r = random_vec3();
	float h = fract(r.x)*2.0-1.0;
	float alpha = asin(h);
	float beta = DPI*r.y;
	float rad = cos(alpha)*r.z;
	return vec3(rad*cos(beta),rad*sin(beta),h*r.z);
}

vec3 random_sample_cylinder()
{
	vec3 r = random_vec3();
	float alpha = DPI*r.y;
	return vec3(r.x*cos(alpha),r.x*sin(alpha),r.z);
}


vec3 rand_sample_primitive(int p)
{
	switch(read_primitive_type_simple(p))
	{
		case CODE_SPHERE:
			return random_sample_sphere();
			break;
		case CODE_CUBE:
			return random_sample_cube();
			break;
		case CODE_ORIENTED_QUAD:
			return random_sample_oriented_quad();
			break;
		case CODE_CYLINDER:
			return random_sample_cylinder();
			break;
	}
	return vec3(0);
}
*/












































