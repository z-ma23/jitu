#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"

#include <string>

using namespace std;

// 反射方向计算
Vector3f reflect(const Vector3f &incident, const Vector3f &normal) {
    return incident - 2 * Vector3f::dot(incident, normal) * normal;
}

// 折射方向计算 (斯涅尔定律)
Vector3f refract(const Vector3f &incident, const Vector3f &normal, float refractiveIndex) {
    float n1 = 1.0f; // 空气折射率
    float n2 = refractiveIndex;
    Vector3f N = normal;
    
    float cosI = -Vector3f::dot(N, incident);
    if (cosI < 0) { // 从内部射出
        cosI = -cosI;
        N = -N;
        std::swap(n1, n2);
    }
    
    float eta = n1 / n2;
    float k = 1 - eta * eta * (1 - cosI * cosI);
    
    if (k < 0) { // 全反射
        return Vector3f::ZERO;
    }
    
    return eta * incident + (eta * cosI - sqrtf(k)) * N;
}

Vector3f traceRay(const Ray &ray, const SceneParser &scene, int depth, float tmin) {
    if (depth > 5) return scene.getBackgroundColor(); // 限制递归深度
    
    Group *baseGroup = scene.getGroup();
    Hit hit;
    bool isIntersect = baseGroup->intersect(ray, hit, tmin);
    
    if (isIntersect) {
        Material *material = hit.getMaterial();
        Vector3f hitPoint = ray.pointAtParameter(hit.getT());
        Vector3f normal = hit.getNormal();
        
        // 处理反射材质
        if (material->isReflective()) {
            Vector3f reflectedDir = reflect(ray.getDirection(), normal);
            Ray reflectedRay(hitPoint + reflectedDir * 1e-4, reflectedDir);
            return traceRay(reflectedRay, scene, depth + 1, tmin) * material->getDiffuseColor();
        }
        
        // 处理折射材质
        if (material->isRefractive()) {
            Vector3f refractedDir = refract(ray.getDirection(), normal, 
                                           material->getRefractiveIndex());
            if (refractedDir.length() > 0) { // 不是全反射
                Ray refractedRay(hitPoint + refractedDir * 1e-4, refractedDir);
                return traceRay(refractedRay, scene, depth + 1, tmin) * material->getDiffuseColor();
            } else { // 全反射情况
                Vector3f reflectedDir = reflect(ray.getDirection(), normal);
                Ray reflectedRay(hitPoint + reflectedDir * 1e-4, reflectedDir);
                return traceRay(reflectedRay, scene, depth + 1, tmin) * material->getDiffuseColor();
            }
        }
        
        // 漫反射材质 - 添加阴影检测
        Vector3f finalColor = Vector3f::ZERO;
        for (int li = 0; li < scene.getNumLights(); ++li) {
            Light* light = scene.getLight(li);
            Vector3f L, lightColor;
            light->getIllumination(hitPoint, L, lightColor);
            
            // 阴影检测 - 发射Shadow Ray
            Ray shadowRay(hitPoint + L * 1e-4, L); // 小偏移防止自相交
            Hit shadowHit;
            bool inShadow = baseGroup->intersect(shadowRay, shadowHit, tmin);
            
            if (!inShadow || shadowHit.getT() > (light->getPosition() - hitPoint).length()) {
                finalColor += material->Shade(ray, hit, L, lightColor);
            }
        }
        return finalColor;
    }
    return scene.getBackgroundColor();
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

    Image outImg(camera->getWidth(), camera->getHeight());
    // 循环屏幕空间的像素
    for (int x = 0; x < camera->getWidth(); ++x) {
        for (int y = 0; y < camera->getHeight(); ++y) {
            Ray camRay = camera->generateRay(Vector2f(x, y));
            Vector3f color = traceRay(camRay, sceneParser, 0, 0.001f);
            outImg.SetPixel(x, y, color);
        }
    }
    outImg.SaveBMP(outputFile.c_str());
    cout << "Hello! Computer Graphics!" << endl;
    return 0;
}

