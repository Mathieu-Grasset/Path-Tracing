#include "glsl_rec.h"
#include <sstream>
#include <iomanip>

std::string RecursiveGLSL::generate_one(int vX, int vY) const
{
	auto src_rec = tmplte_;
	std::stringstream sstrX;
	sstrX << "0x" <<std::setfill('0') << std::setw(2)<<std::hex<< vX;
	std::stringstream sstrY;
	sstrY << "0x" << std::setfill('0') << std::setw(2)<<std::hex << vY;
	std::string strX = sstrX.str();
	std::string strY = sstrY.str();

	for (auto p: placesXX_)
		src_rec.replace(p,4, strX);

	for (auto p: placesYY_)
		src_rec.replace(p,4,strY);

	return src_rec;
}


RecursiveGLSL::RecursiveGLSL(const std::string& src):
tmplte_(src)
{
	std::size_t pl = tmplte_.find("0xXX",0);
	while (pl != std::string::npos)
	{
		placesXX_.push_back(pl);
		pl += 2;
		pl = tmplte_.find("0xXX",pl + 2);
	} 

	pl = tmplte_.find("0xYY", 0);
	while (pl != std::string::npos)
	{
		placesYY_.push_back(pl);
		pl += 2;
		pl = tmplte_.find("0xYY", pl + 2);
	}
}

std::string RecursiveGLSL::generate(int nb) const
{
	auto src = generate_one(nb,nb+1);
	while (nb>0)
	{
		src += generate_one(nb-1,nb);
		nb--;
	}

	return src;
}


