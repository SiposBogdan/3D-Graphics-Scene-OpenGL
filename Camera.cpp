#include "Camera.hpp"
#include <iostream>
#include <vector> 

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));

    }



    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO

        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition += cameraFrontDirection * speed;
            break;

        case MOVE_BACKWARD:
            cameraPosition -= cameraFrontDirection * speed;
            break;

        case MOVE_RIGHT:
            cameraPosition += cameraRightDirection * speed;
            break;

        case MOVE_LEFT:
            cameraPosition -= cameraRightDirection * speed;
            break;

        case MOVE_UP:
            cameraPosition += speed * cameraUpDirection;
            break;
        case MOVE_DOWN:
            cameraPosition -= speed * cameraUpDirection;
            break;
        }
        cameraTarget = cameraPosition + cameraFrontDirection;

    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        glm::vec3 front = glm::normalize(direction);
        cameraFrontDirection = front;
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        cameraTarget = cameraPosition + cameraFrontDirection;

    }

    void Camera::moveFreely(float startTime, float elapsedTime, const std::vector<Keyframe>& path) {
        for (size_t i = 0; i < path.size() - 1; ++i) {
            if ((elapsedTime - startTime) >= path[i].time && (elapsedTime - startTime) <= path[i + 1].time) {
                float t = ((elapsedTime - startTime) - path[i].time) / (path[i + 1].time - path[i].time);
                cameraPosition = glm::mix(path[i].position, path[i + 1].position, t);
                cameraFrontDirection = glm::normalize(glm::mix(path[i].direction, path[i + 1].direction, t));
                break;
            }
        }
    }



}