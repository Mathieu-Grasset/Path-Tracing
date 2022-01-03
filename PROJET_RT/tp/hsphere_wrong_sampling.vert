
vec3 sample_hemisphere()
{
	// Algo echantillonnage uniforme hemisphere:
	// Z <- rand entre ? et ?    
	// beta <- Z : angle/plan_xy 	
	// alpha <- rand entre ? et ?
	// x,y,z <- alpha,beta : coord polaire -> cartesienne
	// fake !!!!!!
	//return normalize(random_vec3());
	
	/*
	float z = random_float();
	// alpha : norme de mon hémisphère doit être >= z
	float alpha = random_float()+z;
	//Angle calculé à partir de ma norme (alpha) et de ma hauteur (z)
	float beta = asin(z/alpha);
	float x = 1.0 * cos(beta);
	float y = 1.0 * sin(beta);
	*/
	
    float Z = (random_float());
    float beta = acos(Z)*(1-0.5);    
    float alpha = random_float() * PI*2;

    float x = sin(beta) * cos(alpha);
    float y = sin(beta) * sin(alpha);
    float z = cos(beta);

    return normalize(vec3(x,y,z));


}

// D direction principale de l'hemisphere, normalisée
vec3 random_ray(in vec3 D)
{
	// Algo orientation échantillon
	// choisir un W normalisé non colineaire à D
	// U orthogonal à D et W                    
	// V tq U,V,D repère ortho-normé direct
	// mettre  U,V,D dans une  matrice 3x3 de changement de repère M
	// multiplier votre echantillon par M pour bien l'orienter
	// ici par de matrice 4x4 car pas de translation
	//return sample_hemisphere();

	//On tire un vecteur W aléatoire jusqu'à qu'il respecte la contrainte.
	vec3 W = random_vec3();
	W = normalize(W);
	while (dot(W,D) == 0)
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


void main()
{
	// param de srand le nombre de random_float appelé dans le shader
	srand(3u);
	vec3 P = random_ray(normalize(normal));
	gl_Position = pvMatrix * vec4(P,1);
}