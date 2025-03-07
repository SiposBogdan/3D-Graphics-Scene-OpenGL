#ifndef Camera_hpp
#define Camera_hpp

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <string>
#include <vector>

struct Keyframe {
    glm::vec3 position;
    glm::vec3 direction;
    float time; // Time in seconds to reach this keyframe
};
namespace gps {

    
    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN};
    
    class Camera
    {
    public:
        //Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        //return the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();
        //update the camera internal parameters following a camera move event
        void move(MOVE_DIRECTION direction, float speed);
        //update the camera internal parameters following a camera rotate event
        //yaw - camera rotation around the y axis
        //pitch - camera rotation around the x axis
        void rotate(float pitch, float yaw);
        void setPosition(const glm::vec3& pos);
        void lookAt(const glm::vec3& target);
        void moveFreely(float startTime, float elapsedTime, const std::vector<Keyframe>& path);

    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
    };

}

#endif /* Camera_hpp */