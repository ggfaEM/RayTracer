#pragma once
#include <glm.hpp>

struct Material
{
	glm::vec3 ambient, diffuse, specular;

	Material() : ambient(0), diffuse(0), specular(0) {}

	Material(glm::vec3 amb, glm::vec3 dif, glm::vec3 spe)
		: ambient(amb), diffuse(dif), specular(spe) {}
};