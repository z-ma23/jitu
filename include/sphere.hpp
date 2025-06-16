#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TO: Implement functions and add more fields as necessary

class Sphere : public Object3D {
public:
    Sphere() : center(Vector3f::ZERO), radius(1.0f)  {
        // unit ball at the center
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material), center(center), radius(radius) {
        // 
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {

        Vector3f oc = r.getOrigin() - center;
        float a = Vector3f::dot(r.getDirection(), r.getDirection());
        float b = 2.0f * Vector3f::dot(oc, r.getDirection());
        float c = Vector3f::dot(oc, oc) - radius * radius;
        
        float discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) {
            return false; // 无实数解，光线未击中球体
        }

        float sqrtDiscriminant = sqrtf(discriminant);
        float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
        float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

        float t = t1; // 优先取较小的t（更近的交点）
        if (t < tmin) {
            t = t2;   // 如果t1无效，尝试t2
            if (t < tmin) {
                return false; // 两个交点都在tmin之前
            }
        }

        // 检查当前t是否比之前记录的交点更近
        if (t < h.getT()) {
            Vector3f normal = (r.pointAtParameter(t) - center).normalized();
            h.set(t, material, normal); // 更新交点信息
            return true;
        }
        
        return false;
    }

protected:
    Vector3f center; // 球心坐标
    float radius;    // 球体半径
};


#endif
