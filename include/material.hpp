#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include <iostream>

enum MaterialType { DIFF, SPEC, REFR, METAL };

// TO: Implement Shade function that computes Phong introduced in class.
class Material {
public:

    explicit Material(const Vector3f &d_color,const Vector3f &e_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0,float refl = 0, float refr = 0, float refr_index = 1.0f,MaterialType t = DIFF,const Vector3f &ambient = Vector3f(0.4, 0.4, 0.4)) :
            diffuseColor(d_color),emissionColor(e_color), specularColor(s_color), shininess(s),reflectivity(refl), refractivity(refr), refractiveIndex(refr_index),type(t),ambientColor(ambient) {

    }

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }

    virtual Vector3f getEmissionColor() const {
        return emissionColor;
    }

    bool isReflective() const { return reflectivity > 0; }
    bool isRefractive() const { return refractivity > 0; }
    float getRefractiveIndex() const { return refractiveIndex; }

    Vector3f getAmbientColor() const { return ambientColor; }

    MaterialType getType() const{ return type;};


    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor,float shadowIntensity = 1.0f) {
        //phong模型
        Vector3f N = hit.getNormal(), V = -ray.getDirection().normalized();
        Vector3f L = dirToLight.normalized();
        Vector3f R = (2 * (Vector3f::dot(L, N)) * N - L).normalized();

        float diffuseFactor = max0x(Vector3f::dot(L, N));
        float specularFactor = pow(max0x(Vector3f::dot(V, R)), shininess);

        diffuseFactor *= shadowIntensity;
        specularFactor *= shadowIntensity;

        Vector3f shaded = lightColor * (
            diffuseColor * diffuseFactor + 
            specularColor * specularFactor
        );
        return shaded;
    }

protected:
    Vector3f diffuseColor;
    Vector3f emissionColor;
    Vector3f specularColor;
    float shininess;
    float reflectivity;    // 反射系数 [0,1]
    float refractivity;    // 折射系数 [0,1]
    float refractiveIndex; // 折射率
    Vector3f ambientColor; // 环境光项
    MaterialType type;
    float max0x(float x) { return std::max((float)0, x); }
};


#endif // MATERIAL_H
