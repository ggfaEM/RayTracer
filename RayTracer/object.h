#pragma once
#include <memory>
#include <glm.hpp>

#include "material.h"
#include "ray.h"

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

struct Object
{
	std::shared_ptr<Material> mat;

	Object() = default;

	virtual float intersect(const Ray &ray) const = 0;

	virtual glm::vec3 get_normal(glm::vec3 p) const = 0;
};

struct Sphere : public Object
{
	glm::vec3 origin;
	float r;
	glm::vec3 color;

	Sphere(glm::vec3 origin, float radius, glm::vec3 color, std::shared_ptr<Material> m)
	{
		this->origin = origin;
		this->r = radius;
		this->color = color;
		this->mat = m;
	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return glm::normalize(p - origin);
	}

	float intersect(const Ray &ray) const
	{
		float t1 = INFINITY, t2 = t1;
		float tmp;

		float term_1 = glm::dot(ray.rd, ray.rd);
		float term_2 = 2 * glm::dot(ray.rd, ray.ro - origin);
		float term_3 = glm::dot(ray.ro - origin, ray.ro - origin) - r * r;

		float disc = term_2 * term_2 - 4 * term_1*term_3;

		if (disc < 0)
		{
			return INFINITY;
		}

		t1 = (-term_2 + sqrt(disc)) / (2 * term_1);
		t2 = (-term_2 - sqrt(disc)) / (2 * term_1);

		tmp = fmin(t1, t2);
		tmp = tmp >= 0 ? tmp : fmax(t1, t2);
		return tmp >= 0 ? tmp : INFINITY;
	}
};

struct Plane : public Object
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	float k;

	Plane(glm::vec3 p, glm::vec3 n, glm::vec3 c, std::shared_ptr<Material> m) :
		pos(p), normal(glm::normalize(n)), color(c)
	{
		this->mat = m;
		k = glm::dot(normal, pos);
	}

	float intersect(const Ray &ray) const
	{
		float denom = glm::dot(normal, ray.rd);

		if (abs(denom) < 1e-6) return INFINITY;

		float num = k - glm::dot(normal, ray.ro);

		float t = num / denom;

		//if(t >= 0) std::cout << t << std::endl;
		return t >= 0 ? t : INFINITY;
	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return normal;
	}
};

struct Rectangle : public Object
{
	glm::vec3 center;
	glm::vec3 v1;
	glm::vec3 v2;
	// normal of the plane the rectangle resides in
	glm::vec3 normal;

	float v1_dot;
	float v2_dot;

	Rectangle() {};

	Rectangle(glm::vec3 center, glm::vec3 u, glm::vec3 v, std::shared_ptr<Material> m) :
		center(center - 0.5f * (u + v))
	{
		this->mat = m;
		this->v1 = u;
		this->v2 = v;
		this->normal = glm::normalize(glm::cross(u, v));
		v1_dot = glm::dot(v1, v1);
		v2_dot = glm::dot(v2, v2);
	}

	float intersect(const Ray &ray) const
	{
		float denom = glm::dot(ray.rd, normal);

		if (abs(denom) < 1e-6) return INFINITY;

		float num = glm::dot(normal, center - ray.ro);

		float t = num / denom;

		if (t < 0) return INFINITY;

		glm::vec3 isec_p = ray.ro + t * ray.rd;

		float inside_1 = glm::dot(isec_p - center, v1) / v1_dot;
		float inside_2 = glm::dot(isec_p - center, v2) / v2_dot;

		bool test = (0 <= inside_1) && (inside_1 <= 1) &&
			(0 <= inside_2) && (inside_2 <= 1);

		return test ? t : INFINITY;
	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return normal;
	}

	glm::vec3 get_normal() const
	{
		return normal;
	}
};

class Cube : public Object
{
	glm::vec3 normal;
	glm::vec3 boundaries;
	glm::vec3 centers[6];
	glm::vec3 v1[3], v2[3];
	float v1_dots[3], v2_dots[3];

public:
	Cube(glm::vec3 b) : boundaries(b)
	{
		assert(fmin(fmin(b.x, b.y), b.z) > 0);

		for (int i = 0; i < 6; ++i)
		{
			// too lazey to write out the six centers
			int i_m = i % 3;
			centers[i] = glm::vec3(1.f) *
				glm::vec3(i_m == 0, i_m == 1, i_m == 2) *
				(-1.f * (i > 3 ? 1.f : -1.f));
		}

		// sides
		v1[0] = glm::vec3(0.f, 0.f, 2 * boundaries[2]);
		v2[0] = glm::vec3(0.f, 2 * boundaries[1], 0.f);

		v1[1] = glm::vec3(2 * boundaries[0], 0.f, 0.f);
		v2[1] = glm::vec3(0.f, 0.f, 2 * boundaries[2]);

		v1[2] = glm::vec3(2 * boundaries[0], 0.f, 0.f);
		v2[2] = glm::vec3(0.f, 2 * boundaries[1], 0.f);

		for (int i = 0; i < 3; ++i)
		{
			v1_dots[i] = glm::dot(v1[i], v1[i]);
			v2_dots[i] = glm::dot(v2[i], v2[i]);
		}
	}

	float intersect(const Ray &ray) const
	{
		assert(abs(length(ray.rd)) > 0);

		int nearest;
		float isec_t = INFINITY;
		glm::vec3 t[2];
		glm::vec3 m;
		glm::vec3 isec_p;


		// calculate the 6 ray-plane intersections
		t[0] = -(boundaries + ray.ro);
		t[1] = (boundaries - ray.ro);

		if (ray.rd.x != 0)
		{
			t[0].x /= ray.rd.x;
			t[1].x /= ray.rd.x;
		}
		else
		{
			t[0].x = (t[0].x == 0 ? 0.f : INFINITY);
			t[1].x = (t[1].x == 0 ? 0.f : INFINITY);
		}

		if (ray.rd.y != 0)
		{
			t[0].y /= ray.rd.y;
			t[1].y /= ray.rd.y;
		}
		else
		{
			t[0].y = (t[0].y == 0 ? 0.f : INFINITY);
			t[1].y = (t[1].y == 0 ? 0.f : INFINITY);
		}

		if (ray.rd.z != 0)
		{
			t[0].z /= ray.rd.z;
			t[1].z /= ray.rd.z;
		}
		else
		{
			t[0].z = (t[0].z == 0 ? 0.f : INFINITY);
			t[1].z = (t[1].z == 0 ? 0.f : INFINITY);
		}

		// get nearest plane
		nearest = (t[0].x < t[0].y ?
			(t[0].x < t[0].z ? 0 : 2) :
			(t[0].y < t[0].z ? 1 : 2));

		nearest = (t[1].x < t[1].y ?
			(t[1].x < t[1].z ? 3 : 5) :
			(t[1].y < t[1].z ? 4 : 5));

		// get nearest intersection
		m = glm::min(t[0], t[1]);

		isec_t = fmin(fmin(m.x, m.y), m.z);

		// no intersection found
		if (isec_t < 0.f)
		{
			return INFINITY;
		}

		isec_p = ray.ro + isec_t * ray.rd;

		// test if inside
		int tmp = nearest % 3;
		float inside_1 = glm::dot(isec_p - centers[nearest], v1[tmp]) /
			v1_dots[tmp];
		float inside_2 = glm::dot(isec_p - centers[nearest], v2[tmp]) /
			v2_dots[tmp];

		bool test = (0 <= inside_1) && (inside_1 <= 1)
			&& (0 <= inside_2) && (inside_2 <= 1);

		return (test ? isec_t : INFINITY);

	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		glm::vec3 a_p = glm::abs(p);
		return a_p.x > a_p.y ? 
			(a_p.x > a_p.z ? glm::vec3(sgn(p.x), 0.f, 0.f) :glm::vec3(0.f, 0.f, sgn(p.z))) :
			(a_p.y > a_p.z ? glm::vec3(0.f, sgn(p.y), 0.f) : glm::vec3(0.f, 0.f, sgn(p.z)));
	}
};