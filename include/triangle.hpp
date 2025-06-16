#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// TO: implement this class and add more fields as necessary,
class Triangle: public Object3D {

public:
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m), normal(Vector3f::cross(b - a, c - a).normalized()) {
		vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;
	}

	bool intersect( const Ray& ray,  Hit& hit , float tmin) override {
		Vector3f E1 = vertices[1] - vertices[0];
        Vector3f E2 = vertices[2] - vertices[0];
        Vector3f P = Vector3f::cross(ray.getDirection(), E2);
        float det = Vector3f::dot(E1, P);

        // 如果光线与三角形平行（行列式接近0），无交点
        if (fabs(det) < 1e-6) {
            return false;
        }

        float invDet = 1.0f / det;
        Vector3f T = ray.getOrigin() - vertices[0];
        float u = Vector3f::dot(T, P) * invDet;

        // u 必须在 [0,1] 范围内
        if (u < 0 || u > 1) {
            return false;
        }

        Vector3f Q = Vector3f::cross(T, E1);
        float v = Vector3f::dot(ray.getDirection(), Q) * invDet;

        // v 必须 >=0 且 u+v <=1
        if (v < 0 || u + v > 1) {
            return false;
        }

        float t = Vector3f::dot(E2, Q) * invDet;

        // t 必须满足 t >= tmin 且比之前记录的更近
        if (t >= tmin && t < hit.getT()) {
            hit.set(t, material, normal);
            return true;
        }

        return false;
	}
	Vector3f normal;
	Vector3f vertices[3];
protected:

};

#endif //TRIANGLE_H
