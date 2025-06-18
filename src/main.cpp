#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <ctime>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "utils.hpp"

#include <string>

using namespace std;

Vector3f reflect(const Vector3f &incident, const Vector3f &normal) {
    return incident - 2 * Vector3f::dot(incident, normal) * normal;
}

Vector3f refract(const Vector3f &incident, const Vector3f &normal, float ni_over_nt, bool &refracted) {
    Vector3f unit_incident = incident.normalized();
    float dt = Vector3f::dot(unit_incident, normal);
    float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1 - dt * dt);
    if (discriminant > 0) {
        refracted = true;
        return ni_over_nt * (unit_incident - normal * dt) - normal * sqrtf(discriminant);
    } else {
        refracted = false;
        return Vector3f();
    }
}

Vector3f cosineSampleHemisphere(const Vector3f &normal) {
    float r1 = 2 * M_PI * randf();
    float r2 = randf(), r2s = sqrt(r2);
    Vector3f u = Vector3f::cross((fabs(normal.x()) > 0.1f ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0)), normal).normalized();
    Vector3f v = Vector3f::cross(normal, u);
    return (u * cos(r1) * r2s + v * sin(r1) * r2s + normal * sqrt(1 - r2)).normalized();
}

Vector3f traceRay(const Ray &ray, const SceneParser &scene, int depth) {
    if (depth > 20) return Vector3f();

    Group *baseGroup = scene.getGroup();
    Hit hit;
    if (!baseGroup->intersect(ray, hit, 1e-4f))
        return scene.getBackgroundColor();

    Vector3f hitPoint = ray.pointAtParameter(hit.getT());
    Vector3f normal = hit.getNormal().normalized();
    Vector3f wo = -ray.getDirection().normalized();
    Material *material = hit.getMaterial();

    Vector3f emission = material->getEmissionColor();
    Vector3f color = material->getDiffuseColor();
    auto type = material->getType(); // DIFF / SPEC / REFR

    // Russian Roulette
    float p = std::max(color.x(), std::max(color.y(), color.z()));
    if (depth > 5) {
        if (randf() > p) return emission;
        color = color/p;
    }

    if (type == DIFF) {
        Vector3f directLighting(0);

        for (int i = 0; i < scene.getNumLights(); ++i) {
            Light *light = scene.getLight(i);
            Vector3f lightPos, lightDir, lightNormal, Le;
            float pdf = 1.0f;
            float distanceToLight = 1e30;

            if (light->isAreaLight()) {
                Le = light->samplePoint(lightPos, lightNormal, pdf);
                lightDir = (lightPos - hitPoint);
                distanceToLight = lightDir.length();
                lightDir = lightDir.normalized();
            } else {
                light->getIllumination(hitPoint, lightDir, Le);
                lightDir.normalize();
            }

            Ray shadowRay(hitPoint + lightDir * 1e-4f, lightDir);
            Hit shadowHit;
            if (!scene.getGroup()->intersect(shadowRay, shadowHit, 1e-4f)
                || shadowHit.getT() > distanceToLight - 1e-3f) {

                float cos_theta = std::max(0.0f, Vector3f::dot(normal, lightDir));
                float cos_light = light->isAreaLight() ? std::max(0.0f, Vector3f::dot(-lightDir, lightNormal)) : 1.0f;
                float geom_term = cos_theta * cos_light / (light->isAreaLight() ? (distanceToLight * distanceToLight) : 1.0f);
                Vector3f brdf = color / M_PI;

                directLighting += Le * brdf * geom_term / pdf;
            }
        }

        Vector3f dir = cosineSampleHemisphere(normal);
        Ray newRay(hitPoint + dir * 1e-4f, dir);
        Vector3f indirect = color * traceRay(newRay, scene, depth + 1);

        return emission + directLighting + indirect;
    }else if (type == SPEC) {
        Vector3f dir = reflect(ray.getDirection(), normal).normalized();
        Ray newRay(hitPoint + dir * 1e-4f, dir);
        return emission + color * traceRay(newRay, scene, depth + 1);
    } else if (type == REFR) {
        bool into = Vector3f::dot(normal, ray.getDirection()) < 0;
        Vector3f n = into ? normal : -normal;
        float eta = into ? (1.0f / material->getRefractiveIndex()) : material->getRefractiveIndex();

        bool refracted = false;
        Vector3f refr_dir = refract(ray.getDirection(), n, eta, refracted).normalized();

        Vector3f refl_dir = reflect(ray.getDirection(), normal).normalized();
        Ray reflRay(hitPoint + refl_dir * 1e-4f, refl_dir);

        if (!refracted) {
            return emission + color * traceRay(reflRay, scene, depth + 1); // 全反射
        }

        Ray refrRay(hitPoint + refr_dir * 1e-4f, refr_dir);

        // Schlick's approximation
        float R0 = powf((1 - eta) / (1 + eta), 2);
        float c = 1 - (into ? -Vector3f::dot(ray.getDirection(), normal) : Vector3f::dot(refr_dir, normal));
        float Re = R0 + (1 - R0) * powf(c, 5);
        float Tr = 1 - Re;

        float prob = 0.25 + 0.5 * Re;
        if (depth > 2) {
            if (randf() < prob)
                return emission + color * traceRay(reflRay, scene, depth + 1) * Re / prob;
            else
                return emission + color * traceRay(refrRay, scene, depth + 1) * Tr / (1 - prob);
        } else {
            return emission + color * (traceRay(reflRay, scene, depth + 1) * Re +
                                       traceRay(refrRay, scene, depth + 1) * Tr);
        }
    }else if (type == METAL) {
        Vector3f perfect_reflect = reflect(ray.getDirection(), normal).normalized();

        // 粗糙反射（添加一点扰动）
        float fuzz = 0.8f; // 材质参数,可修改
        Vector3f perturbed = (perfect_reflect + fuzz * cosineSampleHemisphere(normal)).normalized();

        Ray newRay(hitPoint + perturbed * 1e-4f, perturbed);
        return emission + color * traceRay(newRay, scene, depth + 1);
    }

    return Vector3f(); // fallback
}

float clamp(float x) {
    return x < 0 ? 0 : (x > 1 ? 1 : x);
}

int main(int argc, char *argv[]) {
    for (int argNum = 1; argNum < argc; ++argNum) {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3) {
        cout << "Usage: ./bin/PA1 <input scene file> <output bmp file>" << endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2];  // only bmp is allowed.

    // TO: Main RayCasting Logic
    // First, parse the scene using SceneParser.
    // Then loop over each pixel in the image, shooting a ray
    // through that pixel and finding its intersection with
    // the scene.  Write the color at the intersection to that
    // pixel in your output image.
    SceneParser sceneParser(inputFile.c_str());
    Camera* camera = sceneParser.getCamera();
    const int spp = 32;

    Image outImg(camera->getWidth(), camera->getHeight());
    // 循环屏幕空间的像素
    for (int x = 0; x < camera->getWidth(); ++x) {
        for (int y = 0; y < camera->getHeight(); ++y) {
            Vector3f color(0, 0, 0);
            for (int s = 0; s < spp; ++s) {
                float dx = randf(), dy = randf();
                Ray camRay = camera->generateRay(Vector2f(x + dx, y + dy));
                color += traceRay(camRay, sceneParser, 0);
            }
            color = color/spp;
            color = Vector3f(
                powf(clamp(color.x()), 1.0f / 2.2f),
                powf(clamp(color.y()), 1.0f / 2.2f),
                powf(clamp(color.z()), 1.0f / 2.2f)
            );
            outImg.SetPixel(x, y, color);
        }
    }
    outImg.SaveBMP(outputFile.c_str());
    cout << "Hello! Computer Graphics!" << endl;
    return 0;
}

