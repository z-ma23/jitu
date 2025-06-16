#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>


// TO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D {

public:

    Group() {};

    explicit Group (int num_objects) : objectList(num_objects){}

    ~Group() override {}

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        bool hitAnything = false;
        for (auto obj : objectList) {
            if (obj) hitAnything |= obj->intersect(r, h, tmin);
        }
        return hitAnything;
    }

    void addObject(int index, Object3D *obj) {
        if (index >= 0 && index <= objectList.size()) {
            objectList.insert(objectList.begin() + index, obj);
        } else {
            std::cerr << "Invalid index for addObject\n";
        }
    }

    int getGroupSize() const{
        return objectList.size();
    }

private:
    std::vector<Object3D*> objectList;
};

#endif
	
