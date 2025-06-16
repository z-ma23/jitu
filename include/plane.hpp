#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Plane() : normal(Vector3f::UP), d(0) {

    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m), normal(normal.normalized()), d(d) {
        assert(normal.length() == 1);
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float denom = Vector3f::dot(normal, r.getDirection());
        
        // 如果光线与平面平行（分母接近0），无交点
        if (fabs(denom) < 1e-6) {
            return false;
        }

        float t = (d - Vector3f::dot(normal, r.getOrigin())) / denom;
        
        // 检查 t 是否有效（在 tmin 之后，且比之前记录的 t 更近）
        if (t >= tmin && t <= h.getT()) {
            h.set(t, material, normal);
            return true;
        }

        return false;
    }

protected:
    Vector3f normal; // 单位法向量（平面方程中的 a, b, c）
    float d;
};

#endif //PLANE_H
		

