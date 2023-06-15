#include "Camera.h"

Camera::Camera() {}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed)
{
    position = startPosition;
    worldUp = startUp;
    yaw = startYaw;
    pitch = startPitch;
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    right = glm::normalize(glm::cross(front, worldUp));

    moveSpeed = startMoveSpeed;
    turnSpeed = startTurnSpeed;

    update();
}

void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
    GLfloat velocity = moveSpeed * deltaTime;

    if (keys[GLFW_KEY_UP])
    {
        position += front * velocity;
    }

    if (keys[GLFW_KEY_DOWN])
    {
        position -= front * velocity;
    }

    if (keys[GLFW_KEY_LEFT])
    {
        position -= right * velocity;
    }

    if (keys[GLFW_KEY_RIGHT])
    {
        position += right * velocity;
    }
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{
    xChange *= turnSpeed;
    yChange *= turnSpeed;

    yaw += xChange;
    pitch += yChange;

    if (pitch > 89.0f)
    {
        pitch = 89.0f;
    }

    if (pitch < -89.0f)
    {
        pitch = -89.0f;
    }

    update();
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange, const glm::vec3& objectPosition, const glm::vec3& previousObjectPosition)
{
    xChange *= turnSpeed;
    yChange *= turnSpeed;

    // Calculate the vector from the object position to the camera position
    glm::vec3 objectToCamera = position - objectPosition;

    // Rotate the objectToCamera vector based on the mouse movement
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-xChange), up);
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(yChange), right);
    objectToCamera = glm::vec3(rotationMatrix * glm::vec4(objectToCamera, 1.0f));

    // Calculate the object's movement vector
    glm::vec3 objectMovement = objectPosition - previousObjectPosition;

    // Update the camera's position based on the new objectToCamera vector and object's movement
    position = objectPosition + objectToCamera + objectMovement;

    FocusOnObject(objectPosition);

    update();
}

void Camera::SetPosition(glm::vec3 position)
{
    this->position = position;
}

void Camera::FocusOnObject(const glm::vec3& objectPosition)
{
    front = glm::normalize(objectPosition - position);
    pitch = glm::degrees(asin(front.y));
    yaw = glm::degrees(atan2(front.z, front.x));
    update();
}

glm::mat4 Camera::calculateViewMatrix()
{
    return glm::lookAt(position, position + front, up);
}

glm::vec3 Camera::getCameraPosition()
{
    return position;
}

glm::vec3 Camera::getCameraDirection()
{
    return glm::normalize(front);
}

void Camera::update()
{
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

Camera::~Camera() {}
