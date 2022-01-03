
layout(location=20) uniform int nb_emissives;
layout(location=21) uniform int NB_BOUNCES;



//-------------------------------------------------------Hémisphère---------------------------------------------
vec3 sample_hemisphere()
{
    float Z = (random_float());
    float beta = acos(Z);    
    float alpha = random_float() * PI*2;

    float x = sin(beta) * cos(alpha);
    float y = sin(beta) * sin(alpha);
    float z = cos(beta);

    return normalize(vec3(x,y,z));
}

//-------------------------------------------------------Rayon aléatoire----------------------------------------
vec3 random_ray(in vec3 D)
{

	vec3 W = random_vec3();
	W = normalize(W);
	while (abs(dot(W,D))<=0.000001 )
        W = normalize(random_vec3());
	//produit vectoriel entre D et W pour avoir U
	vec3 U = vec3(cross(D,W));
	U = normalize(U);
	//Produit vectoriel entre U et D pour avoir V
	vec3 V = vec3(cross(U,D));
	normalize(V);
	mat3 M;
	M[0]=U;
	M[1]=V;
	M[2]=D;

	vec3 echantillon = sample_hemisphere();
	echantillon = M*echantillon;
	return echantillon;
}

//---------------------------------------Chemin du Rayon + calcul couleur--------------------------------

vec3 random_path(in vec3 D, in vec3 O)
{
	traverse_all_bvh(O,D);

	if (!hit())
		return vec3(0,0,0.2) ;
	
	vec3 N;
	vec3 P;
	intersection_info(N,P);
	vec4 mat = intersection_mat_info();
	vec4 col = intersection_color_info();
	if(mat[2]>0.0){
		return mat[2]*col.rgb;
	}
	vec3 color = vec3(0.0,0.0,0.0);
	float angRay;
	float attenuation=1.0;
	for(int i=0;(i<NB_BOUNCES)&&(attenuation>0.01);i++){
		traverse_all_bvh(O,D);
		
		if (!hit())
			return color*attenuation* mix (vec3(0.25,0.25,0.45),vec3(0.1,0.1,0.0),max(0.0,D.z));
		intersection_info(N,P);
		mat = intersection_mat_info();
		col = intersection_color_info();

		
		//coeff d'aténuation 1 au premier tour
		color += attenuation*col.rgb;

		if(mat[2]>0.0){
			return mat[2]*color;
		}

		//calcul du cos entre angle rayon
		normalize(N);
		angRay = dot(D,N);
		
		//rebond -> attenuation plus forte
		attenuation *= 0.1+0.9*max(0.0,angRay);
		D = random_ray(N);
		O = P+0.0001*N;
	}
	return vec3(0);
}

vec3 raytrace(in vec3 Dir, in vec3 Orig)   
{
	// init de la graine du random
	srand();
	// calcul de la lumière captée par un chemin aléatoire
	return random_path(normalize(Dir),Orig);
}

