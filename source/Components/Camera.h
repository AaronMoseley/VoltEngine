#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include "Objects/ObjectComponent.h"
#include "Components/Transform.h"

class Camera : public ObjectComponent {
public:
	Camera() {};

	void SetFOV(const float fov) { m_fov = fov; };
	float GetFOV() const { return m_fov; }

	void SetNearPlane(float nearPlane) { m_nearPlane = nearPlane; };
	float GetNearPlane() const { return m_nearPlane; }

	void SetFarPlane(float farPlane) { m_farPlane = farPlane; };
	float GetFarPlane() const { return m_farPlane; }

	void SetIsMainCamera(bool isMainCamera) { m_isMainCamera = isMainCamera; };
	bool IsMainCamera() const { return m_isMainCamera; }

	glm::mat4 GetViewMatrix() const
	{
		std::shared_ptr<Transform> transform = GetOwner()->GetComponent<Transform>();
		glm::vec3 position = transform->GetPosition();
		return glm::lookAt(position, position + transform->Forward(), transform->Up());
	}

private:
	float m_fov = 45.0f;
	bool m_isMainCamera = true;

	float m_nearPlane = 0.1f;
	float m_farPlane = 10000.0f;
};

#endif