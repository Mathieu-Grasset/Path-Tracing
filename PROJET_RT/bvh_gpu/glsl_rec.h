#ifndef __GLSL_REC__
#define __GLSL_REC__

#include <vector>
#include <string>

	
class RecursiveGLSL
{
	std::string tmplte_;
	std::vector<std::size_t> placesXX_;
	std::vector<std::size_t> placesYY_;

	std::string generate_one(int vX, int vY) const;

public:
	RecursiveGLSL(const std::string& src);
	std::string generate(int nb) const;
};

#endif
