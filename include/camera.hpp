#ifndef CAMERA_HPP
#define CAMERA_HPP

#include<GL/glew.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class camera
{
public:
    glm::vec3 pos; //Camera's position in world coordinates.
    glm::vec3 right, front, up; //Camera's local direction vectors.
    glm::vec3 world_up; //World up direction. In general this can be different from the local 'up' of the camera.
    
    float yaw, pitch; //Camera's yaw, pitch angles. Basically these are the right-left (yaw) and up-down (pitch) of the camera.
    
    float velocity, max_velocity; //Camera's current and max velocity.
    float acceleration; //Camera's acceleration.
    glm::vec3 last_direction; //Camera's last velocity direction, i.e. right before the corresponding motion action (e.g. keyboard key) is released.
    bool is_moving; //Camera's current motion state.

    float mouse_sensitivity;

    float fov; //Camera's field of view.

    //Constructor :
    camera(glm::vec3 init_pos = glm::vec3(0.0f,0.0f,0.0f),
           glm::vec3 init_world_up = glm::vec3(0.0f,0.0f,1.0f),
           float init_yaw = 90.0f, //Point at the +y axis by default.
           float init_pitch = 0.0f,
           float init_max_velocity = 5.0f,
           float init_acceleration = 10.0f,
           float init_mouse_sensitivity = 0.1f,
           float init_fov = 45.0f)
    {
        pos = init_pos;
        world_up = init_world_up;
        yaw = init_yaw;
        pitch = init_pitch;
        velocity = 0.0f;
        max_velocity = init_max_velocity;
        acceleration = init_acceleration;
        last_direction = glm::vec3(0.0f,0.0f,0.0f);
        is_moving = false;
        mouse_sensitivity = init_mouse_sensitivity;
        fov = init_fov;

        update_local_vectors();
    }

    //Update the camera's position.
    void move(float time_tick)
    {
        if (is_moving)
            pos += velocity*time_tick*last_direction;
    }

    //Smooth camera acceleration algorithm.
    void accelerate(float time_tick, glm::vec3 direction)
    {
        velocity += 1.5f*acceleration*time_tick;
        if (velocity > max_velocity)
            velocity = max_velocity;

        pos += velocity*time_tick*direction;
        last_direction = direction;
        is_moving = true;
    }

    //Smooth camera deceleration algorithm.
    void decelerate(float time_tick)
    {
        if (velocity > 0.0f)
        {
            velocity -= 1.0f*acceleration*time_tick;
            if (velocity < 0.0f)
            {
                velocity = 0.0f;
                is_moving = false;
            }
            pos += velocity*time_tick*last_direction;
        }
    }
  
    //Right-left (yaw) and up-down (pitch) rotation.
    void rotate(float xoffset, float yoffset)
    {
        yaw -= xoffset*mouse_sensitivity;
        pitch -= yoffset*mouse_sensitivity;

        if (pitch >= 88.9f)
            pitch = 88.9f;
        else if (pitch <= -88.9f)
            pitch = -88.9f;

        if (abs(yaw) > 360.0f)
            yaw = 0.0f;

        //Upon orientaion change, recalculate the camera local direction vectors, because these will change as well.
        update_local_vectors();
    }

    //Camera's zoom effect. Basically we only change the fov and then this is passed as argument in the projection matrix, wherever the matrix is calculated...
    void zoom(float yoffset)
    {
        fov -= 1.0f*yoffset;

        //Set bounds to the fov.
        if (fov <= 1.0f)
            fov = 1.0f;
        else if (fov >= 150.0f)
            fov = 150.0f; //This will cause a very distorted view.
    }

    void update_local_vectors()
    {
        front.x = cos(glm::radians(pitch))*cos(glm::radians(yaw));
        front.y = cos(glm::radians(pitch))*sin(glm::radians(yaw));
        front.z = sin(glm::radians(pitch));
        front = glm::normalize(front);
        right = glm::normalize(glm::cross(front, world_up));
        up = glm::normalize(glm::cross(right, front));
    }

    glm::mat4 view()
    {
        return glm::lookAt(pos, pos + front, up);
    }
};

#endif
