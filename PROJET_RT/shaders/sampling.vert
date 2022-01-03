#version 430
layout(location=0) uniform mat4 projectionMatrix;
layout(location=1) uniform mat4 viewMatrix;
layout(location=2) uniform vec3 N;

vec3 random_seed =  vec3(0.707,0.0,0.666);;


uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

uint hash( uvec3 v) { return hash( v.x ^ hash(v.y ^ hash(v.z))); }


float random_float()
{
	uint m = hash(floatBitsToUint(random_seed));
	const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0
    float  f = uintBitsToFloat( m );       // Range [1:2]
	random_seed.y += 153.74;
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



void main()
{
	random_seed.y = 153.74*float(10*gl_VertexID);	
	vec3 P = random_vec3();
	vec4 Po4 = viewMatrix * vec4(P,1);
	gl_Position = projectionMatrix * Po4;
}