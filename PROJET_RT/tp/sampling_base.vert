#version 430
layout(location=0) uniform mat4 pvMatrix;
layout(location=2) uniform vec3 normal;
layout(location=3) uniform vec3 fseed;

const float PI = acos(-1.0);

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

void srand(uint nb_used)
{
	random_seed = floatBitsToUint(fseed) + uint(gl_VertexID)* nb_used * uvec3(11u,43u,67u);
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

