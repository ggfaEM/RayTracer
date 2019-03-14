#pragma once

#include "rt.h"
#include "shape.h"

namespace rt
{
class Quadric : public Shape
{
	static bool solveQuadraticEq();
};

struct Sphere : public Quadric
{
	float r;
	glm::vec3 origin;
	glm::vec3 color;
	std::shared_ptr<Texture> tex;


	Sphere(glm::vec3 origin, float radius, glm::vec3 color, std::shared_ptr<Material> m)
	{
		this->origin = origin;
		this->r = radius;
		this->color = color;
		this->mat = m;
		tex = nullptr;
	}

	Sphere(glm::vec3 origin,
		float radius,
		glm::vec3 color,
		std::shared_ptr<Material> m,
		std::shared_ptr<Texture> texture)
	{
		this->origin = origin;
		this->r = radius;
		this->color = color;
		this->mat = m;
		this->tex = texture;
	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return glm::normalize(p - origin);
	}

	float intersect(const Ray &ray, SurfaceInteraction *isect);

	glm::vec2 getTextureCoordinates(glm::vec3 pos)
	{
		glm::vec3 pos_shift = glm::normalize(pos - origin);
		//float radius = glm::length(pos_shift);
		float u = (1 + atan2f(pos_shift.z, pos_shift.x) / (float)M_PI) * 0.5f;
		float v = acosf(pos_shift.y) / (float)M_PI;

		return glm::vec2(u, v);
	}
};

struct Cylinder : public Quadric
{
	float height, radius;
	glm::vec3 pos, dir;
	glm::mat4 objToWorld;
	glm::mat4 worldToObj;
	glm::mat4 tr_worldToObj;

	Cylinder(glm::vec3 pos,
		glm::vec3 dir,
		float radius,
		float height,
		std::shared_ptr<Material> mat) :
		pos(pos), dir(glm::normalize(dir)), radius(radius), height(height)
	{
		this->mat = mat;

		glm::vec3 tangent_v = glm::normalize(Plane::getTangentVector(dir));

		//objToWorld = glm::lookAt(pos, pos + tangent_v, dir);
		// transform axis of the cylinder to the axis given by dir
		objToWorld[0] = glm::vec4(glm::cross(dir, tangent_v), 0.f);
		objToWorld[1] = glm::vec4(dir, 0.f);
		objToWorld[2] = glm::vec4(tangent_v, 0.f);
		objToWorld[3] = glm::vec4(pos, 1.f);

		worldToObj = glm::inverse(objToWorld);
		tr_worldToObj = glm::transpose(worldToObj);
	}

	float intersect(const Ray &ray, SurfaceInteraction *isect);

	glm::vec3 get_normal(glm::vec3 p) const
	{
		// TODO: Inside or outside? How to figure out which side the ray hit?
		return glm::vec3(p.x, 0.f, p.z);
	}

	glm::vec3 get_normal(glm::vec3 p, int hit_cnt) const
	{
		if (hit_cnt == 2)
			return tr_worldToObj * glm::vec4(p.x, 0.f, p.z, 0.f);
		else
			return -(tr_worldToObj * glm::vec4(p.x, 0.f, p.z, 0.f));
	}
};

} // namespace rt