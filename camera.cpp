#include <QDebug>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "settings.h"

namespace MyRaytracer
{
    Camera::Camera(unsigned int theWindowWidth, unsigned int theWindowHeight,
                   const glm::dvec3 &initialPosistion) :
        windowMidX_(theWindowWidth / 2),
        windowMidY_(theWindowHeight / 2),
        position_(initialPosistion)
    {
        reset(initialPosistion);
    }

    void Camera::reset(const glm::dvec3 &newPosition) 
    {
        position_ = newPosition;
        zoomZ_ = std::max(windowMidX_, windowMidY_) * 2;

        viewMatrix_ = glm::translate(glm::dmat4(1.0f), -position_);
    }

    void Camera::mouseMove(int deltaX, int deltaY)
    {
        double dx = -(kPitchSensitivity * deltaY);
        double dy = kYawSensitivity * deltaX;

        rotation_.x += dx;
        rotation_.y += dy;

        viewMatrix_ = glm::rotate(glm::dmat4(1.0f), glm::radians(dy), glm::dvec3(0.0f, 1.0f, 0.0f));
        viewMatrix_ = glm::rotate(viewMatrix_, glm::radians(dx), glm::dvec3(1.0f, 0.0f, 0.0f));

        // Limit loking up/down in 0..360
        if (rotation_.x < 0)
            rotation_.x += 360;
        if (rotation_.x > 360)
            rotation_.x -= 360;

        // Limit left&right in 0..360
        if (rotation_.y < 0)
            rotation_.y += 360;
        if (rotation_.y > 360)
            rotation_.y -= 360;
    }

    void Camera::move(bool moveForward, bool moveBackward, bool moveLeft, bool moveRight)
    {
        glm::dvec3 movement;

        double verticalAngle = glm::radians(rotation_.x);
        double horizontalAngle = glm::radians(rotation_.y);

        glm::dvec3 direction(
            cos(verticalAngle) * sin(horizontalAngle),
            sin(verticalAngle),
            cos(verticalAngle) * cos(horizontalAngle));

        glm::dvec3 right = glm::dvec3(
            sin(horizontalAngle + 3.14f/2.0f),
            0,
            cos(horizontalAngle - 3.14f/2.0f));

        glm::dvec3 up = glm::cross( right, direction );

        if (moveForward) {
            movement = direction * kMovementSpeed;
        } 
        if (moveBackward) {
            movement = -direction * kMovementSpeed;
        }
        if (moveRight) {
            movement = right * kMovementSpeed;
        }
        if (moveLeft) {
            movement = -right * kMovementSpeed;
        }

        position_ += movement;
        viewMatrix_ = glm::translate(glm::dmat4(1.0f), -movement);
    }
}