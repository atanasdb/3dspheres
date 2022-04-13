#ifndef CAMERA_H
#define CAMERA_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "settings.h"

namespace MyRaytracer
{
    class Camera
    {
    public:
        Camera(unsigned int windowWidth, unsigned int windowHeight, 
               const glm::dvec3 &initialPosistion);

        void mouseMove(int deltaX, int deltaY);
        void move(bool moveForward, bool moveBackward, bool moveLeft, bool moveRight);

        void reset(const glm::dvec3 &newPosition);

        glm::dvec3 getPosition() const { return position_; }
        glm::dvec3 getRotation() const { return rotation_; }

        void zoomIn() { zoomZ_+= kZoomSensitivity; }
        void zoomOut() { if (zoomZ_ > kZoomSensitivity) zoomZ_-= kZoomSensitivity; }
        int getZoom() const { return zoomZ_; }

        glm::dmat4 getViewMatrix() const { return viewMatrix_; }

    private:
        glm::dvec3 position_;
        glm::dvec3 rotation_;

        int windowMidX_;
        int windowMidY_;
        int zoomZ_;

        glm::dmat4 viewMatrix_;
    };
}

#endif // CAMERA_H