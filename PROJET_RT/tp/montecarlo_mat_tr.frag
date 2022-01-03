
layout(location=20) uniform int nb_emissives;
layout(location=21) uniform int NB_BOUNCES;
layout(location=23) uniform float k_op;

uniform float d_air = 1.2f;

vec3 sample_hemisphere(in float rug)
{
    float Z = (random_float());
    float beta = acos(Z)*(1-rug);    
    float alpha = random_float() * PI*2;

    float x = sin(beta) * cos(alpha);
    float y = sin(beta) * sin(alpha);
    float z = cos(beta);

    return normalize(vec3(x,y,z));
}

vec3 random_ray(in vec3 D,in float rug)
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

	vec3 echantillon = sample_hemisphere(rug);
	echantillon = M*echantillon;
	return echantillon;
}

vec3 random_path(in vec3 D, in vec3 O)
{	
	vec3 N;
	vec3 P;
	vec4 mat ;
	vec4 col ;
	float angRay;
	vec3 attenuation=vec3(1.0f);
	
	for(int i=0;i<NB_BOUNCES;i++){
		traverse_all_bvh(O,D);
		
		//Partie hors de la scene
		if (i==0 && !hit()){
			return vec3(0,0,0.2);
		}

		if (!hit()){
			return  attenuation * mix (vec3(0.5,0.5,0.9),vec3(1.0,1.0,0.8),max(0.0,D.z));
		}
		
		intersection_info(N,P);
		mat = intersection_mat_info();
		col = intersection_color_info();

		//initialisation de l'atténuation
		if(i==0){
			attenuation = mix(col.rgb , vec3(1.0f) , mat.r);
		}
	
		//hit lumière
		if(mat[2]>0.0){
			return mat[2]*attenuation;
		}
		
		//le rayon peut traverser les objets avec une proba dépendant de la transparence
		if(random_float() < col.a ){
			//on se décale un peu par rapport à l'objet transpa
			O = P+BIAS*N;
			D = random_ray( reflect(D,N) , mat.g);
			angRay = dot(D,N);
			attenuation *= mix( col.rgb*max(0.0,angRay) , vec3(1.0f), mat.r );
		}else{
			O = P-BIAS*N;
			D = normalize( refract(D,N,d_air/k_op) );
			traverse_all_bvh(O,D);
			intersection_info(N,P);

			
			O = P+BIAS*N;
			D = normalize( refract(D,-N,k_op/d_air) );
			angRay = dot(D,N);
			attenuation *= mix( col.rgb*max(0.0,angRay) , vec3(1.0f) , mat.r);
		}
		


		

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

