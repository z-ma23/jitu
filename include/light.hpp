#ifndef LIGHT_H
#define LIGHT_H

#include <Vector3f.h>
#include "object3d.hpp"
#include "utils.hpp"

class Light {
public:
    Light() = default;

    virtual ~Light() = default;
    virtual Vector3f getPosition() const = 0;

    virtual void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const = 0;

    // 可选：添加虚函数用于面光源
    virtual bool isAreaLight() const { return false; }

    // 用于路径追踪直接采样
    virtual Vector3f samplePoint(Vector3f &lightPos, Vector3f &normal, float &pdf) const {
        pdf = 1; lightPos = getPosition(); normal = Vector3f(0, 1, 0);
        return Vector3f(); // 默认无采样
    }

    virtual Vector3f getColor() const { return Vector3f(); } // 默认无色
};


class DirectionalLight : public Light {
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f &d, const Vector3f &c) {
        direction = d.normalized();
        color = c;
    }

    ~DirectionalLight() override = default;

    Vector3f getPosition() const override {
        // 方向光没有具体位置，返回一个远点
        return Vector3f(1e10, 1e10, 1e10) * direction;
    }

    ///@param p unsed in this function
    ///@param distanceToLight not well defined because it's not a point light
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = -direction;
        col = color;
    }

private:

    Vector3f direction;
    Vector3f color;

};

class PointLight : public Light {
public:
    PointLight() = delete;

    PointLight(const Vector3f &p, const Vector3f &c) {
        position = p;
        color = c;
    }

    ~PointLight() override = default;

    Vector3f getPosition() const override {
        return position;
    }

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = (position - p);
        dir = dir / dir.length();
        col = color;
    }

private:

    Vector3f position;
    Vector3f color;

};

class AreaLight : public Light {
public:
    Vector3f position;   // 左下角顶点
    Vector3f u, v;       // 两边向量
    Vector3f normal;
    Vector3f color;
    float area;

    AreaLight(const Vector3f &pos, const Vector3f &uvec, const Vector3f &vvec, const Vector3f &c)
        : position(pos), u(uvec), v(vvec), color(c) {
        normal = Vector3f::cross(u, v).normalized();
        area = Vector3f::cross(u, v).length();
    }

    Vector3f getPosition() const override {
        return position + 0.5 * u + 0.5 * v; // 中心点
    }

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // fallback：返回一个方向
        dir = (getPosition() - p).normalized();
        col = color;
    }

    bool isAreaLight() const override { return true; }

    Vector3f samplePoint(Vector3f &lightPos, Vector3f &n, float &pdf) const override {
        float a = randf(), b = randf();
        lightPos = position + a * u + b * v;
        n = normal;
        pdf = 1.0f / area;
        return color;
    }

    Vector3f getColor() const override {
        return color;
    }
};


#endif // LIGHT_H
