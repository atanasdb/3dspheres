#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace MyRaytracer
{
    struct Ray
    {
        glm::dvec3 origin;
        glm::dvec3 direction;
    };

    struct Intersection
    {
        glm::dvec3 point;
        glm::dvec3 surfaceNormal;
    };

    struct Sphere
    {
        Sphere() : center(0, 0, 0), radius(0) {}
        Sphere(const glm::dvec3 &center, double radius) : center(center), radius(radius) {}

        // Checks if a ray intersects a sphere and returns just the 
        // scalar t in vect' = vect * t + origin
        bool intersects(const Ray &, double &) const;

        glm::dvec3 center;
        double radius;
    };

    struct BVHNode 
    {
        BVHNode() : nextSiblingInc(0) {}
        BVHNode(const Sphere &boundSphere, const Sphere &sphereObj, unsigned int nextSiblingInc) : 
            boundSphere(boundSphere), sphereObj(sphereObj), nextSiblingInc(nextSiblingInc) {}

        Sphere boundSphere, sphereObj;
        unsigned int nextSiblingInc;
    };

    class SceneData
    {
    public:
        SceneData();
        ~SceneData();

        // Creates a BVH tree of the spheres
        bool buildStructure(unsigned int levels);
        // Cleans the scene and deallocates all the memory
        void clear();

        glm::dvec3 lightPos() const { return lightPos_; }
        void setLightPos(const glm::dvec3 &newPos) { lightPos_ = newPos; }

        // Applies a matrix transformation to all the spheres
        void transformPoints(const glm::dmat4 &transformMatrix); 

        // Finds the first intersection of a ray and the structure of spheres
        bool getIntersection(const Ray &ray, Intersection &result) const;

        // Gets the number of spheres in this specific structure
        unsigned int getSpheresCount() const { return spheresCount_; }

        // Returns how much sphere we have for a construction with specified levels
        static unsigned int getSpheresCount(unsigned int level);

    private:
        // Disables copying and assigning
        SceneData &operator=(const SceneData &);
        SceneData(const SceneData &);

        // Crates a sphere on a specific level and inserts it the the tree
        void create(unsigned int level, unsigned int idx, unsigned int nextSiblingInc,
            const glm::dvec3 &center, const glm::dvec3 &up, double radius);

        // Functor that allows the parallelisation of spheres transformation
        struct TransformPointsParallelTask
        {
        public:
            TransformPointsParallelTask(SceneData& outer, const glm::dmat4 &transformMatrix) : 
                outer_(outer), transformMatrix_(transformMatrix) {}
            // Traverse the range of spheres and transforms them
            void operator()(unsigned int taskSphereStartIdx, unsigned int taskSphereEndIdx);
        private:
            glm::dmat4 transformMatrix_;
            SceneData &outer_;
        };

        // BVH tree represented as an array
        BVHNode *tree_;

        unsigned int levels_;
        unsigned int spheresCount_;

        glm::dvec3 lightPos_;
    };
}

#endif //SCENEDATA_H