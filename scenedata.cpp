#include <QDebug>

#include <glm/vec4.hpp>
#include <glm/gtx/norm.hpp>

#include "asyncrunner.h"
#include "scenedata.h"

namespace MyRaytracer 
{
    SceneData::SceneData() :
        tree_(nullptr),
        levels_(0),
        spheresCount_(0)
    {
    }

    SceneData::~SceneData()
    {
        clear();
    }

    void SceneData::clear()
    {
        if (tree_) {
            delete [] tree_; tree_ = nullptr;
            levels_ = 0;
            spheresCount_ = 0;
        }
    }

    void SceneData::create(unsigned int level, unsigned int idx, unsigned int nodesCount, 
        const glm::dvec3 &center, const glm::dvec3 &up, double radius)
    {
        // end of recursion
        if (level > levels_ - 1)
            return;

        // This is the new basis, based on the up vector
        glm::dvec3 new_x(1, 0, 0);
        glm::dvec3 new_y(0, 1, 0);
        glm::dvec3 new_z(0, 0, 1);
        if (up.x != 0 && up.z != 0)
        {
            new_y = glm::normalize(up);
            new_z = glm::normalize(glm::cross(new_y, glm::dvec3(0, 1, 0)));
            new_x = glm::normalize(glm::cross(new_y, new_z));
        }

        // We can bound all the child spheres and its children in a sphere
        // with a sphere with a two times bigger radius
        tree_[idx].boundSphere = Sphere(center, 2*radius);
        tree_[idx].sphereObj = Sphere(center, radius);
        tree_[idx].nextSiblingInc = nodesCount;

        int subtreeNodesCount = (nodesCount - 1)  / 9;

        double newRadius = radius / 3;

        // Put equator spheres
        for (int i = 0; i < 6; i++) {
            int placeAt = idx + 1 + i * subtreeNodesCount;

            double angle = i * (360 / 6);

            double phi = glm::radians(90.0);     
            double theta = glm::radians(angle); 
            double x = sin(phi) * cos(theta);
            double y = cos(phi);
            double z = sin(phi) * sin(theta);
            glm::dvec3 tangentDirection = glm::normalize(x * new_x + y * new_y + z * new_z);

            create(level + 1, placeAt, subtreeNodesCount, center + (tangentDirection * (radius + newRadius)) , tangentDirection, newRadius);
        }
        // Put north spheres
        for (int i = 6; i < 9; i++) {
            int placeAt = idx + 1 + i * subtreeNodesCount;

            double angle = (i - 6) * (360 / 3) + 30;
            double phi = glm::radians(30.0);     
            double theta = glm::radians(angle); 
            double x = sin(phi) * cos(theta);
            double y = cos(phi);
            double z = sin(phi) * sin(theta);
            glm::dvec3 tangentDirection = glm::normalize(x * new_x + y * new_y + z * new_z);

            create(level + 1, placeAt, subtreeNodesCount, center + (tangentDirection * (radius + newRadius)) , tangentDirection, newRadius);
        }
    }

    bool SceneData::buildStructure(unsigned int levels)
    {
        if (spheresCount_ > 0)
            clear();

        spheresCount_ = getSpheresCount(levels);
        qDebug() << "Building a structure with " << spheresCount_ << " spheres ... ";

        tree_ = new (std::nothrow) BVHNode[spheresCount_];
        if (tree_ == nullptr) {
            qDebug() << "Failed to allocate memory for " << spheresCount_ << " spheres ... ";
            spheresCount_ = 0;
            return false;
        }

        levels_ = levels;

        create(0, 0, spheresCount_, glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0), 1.0);

        return true;
    }

    unsigned int SceneData::getSpheresCount(unsigned int level)
    {
        if (level == 0)
            return 0;
        if (level == 1)
            return 1;
        else 
            return getSpheresCount(level - 1)*10 - 9*getSpheresCount(level - 2);
    }

    void SceneData::TransformPointsParallelTask::operator()(unsigned int taskSphereStartIdx,
                                                            unsigned int taskSphereEndIdx)
    {
        for (unsigned int sphereIdx = taskSphereStartIdx; sphereIdx <= taskSphereEndIdx; sphereIdx++) {
            outer_.tree_[sphereIdx].boundSphere.center = 
                glm::dvec3(transformMatrix_ * glm::dvec4(outer_.tree_[sphereIdx].boundSphere.center, 1));

            outer_.tree_[sphereIdx].sphereObj.center = outer_.tree_[sphereIdx].boundSphere.center;
        }
    }

    void SceneData::transformPoints(const glm::dmat4 &transformMatrix)
    {
        AsyncRunner<TransformPointsParallelTask> runner_(TransformPointsParallelTask(*this, transformMatrix));
        runner_.run(spheresCount_);
    }

    bool Sphere::intersects(const Ray &ray, double &t) const
    {
        glm::dvec3 dst = center - ray.origin;
        double b = glm::dot(dst, ray.direction);
        double c = b*b - glm::dot(dst, dst) + radius * radius;

        if (c < 0) 
            return false;

        double d = sqrt(c); 
        double e = b + d;

        if (e < 0)
            return false;

        double f = b - d;
        t = (f > 0) ? f : e;

        return true;
    }

    bool SceneData::getIntersection(const Ray &ray, Intersection &result) const 
    {
        bool hitFound = false;

        double min_t = std::numeric_limits<double>::max();
        double intersectionBoundSphere_t;
        double intersectionSphere_t;

        unsigned int scanIndex = 0;
        while (scanIndex < spheresCount_) {
            if (!tree_[scanIndex].boundSphere.intersects(ray, intersectionBoundSphere_t)) {
                // Skip the entire sub tree and the spheres inside
                scanIndex += tree_[scanIndex].nextSiblingInc;
            } else {
                // We are just interested if we have an intersect and its intersectionSphere_t
                // value. Only if we have better intersectionSphere_t we bother to calculate 
                // the intersection point itself.
                // This is because distance to intersection = length(intersection - origin) = 
                // length((intersectionSphere_t * ray.direction + origin) - origin) ==
                // length(intersectionSphere_t * ray.direction) and this is propotional to intersectionSphere_t
                if (tree_[scanIndex].sphereObj.intersects(ray, intersectionSphere_t)) {
                    if (intersectionSphere_t < min_t) {
                        min_t = intersectionSphere_t;

                        result.point = intersectionSphere_t * ray.direction + ray.origin;
                        result.surfaceNormal = (result.point - tree_[scanIndex].sphereObj.center) / tree_[scanIndex].sphereObj.radius;

                        hitFound = true;
                    }
                }
                // Traverse all the siblings on that level as they are potential hits
                // On the end of the the level (because of the ordering in tree_) we 
                // will go to the next sibling sphere of the prev level
                scanIndex++;
            }
        }

        return hitFound;
    }
}