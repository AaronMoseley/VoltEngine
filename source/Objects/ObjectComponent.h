#pragma once

#include "Management/WindowManager.h"

class RenderObject;
class Scene;

class ObjectComponent {
public:
	ObjectComponent() {};
	virtual ~ObjectComponent() = default;

	virtual void Start() {};
	virtual void Update(float deltaTime) {};

	RenderObject* GetOwner() const { return m_owner; }
	void SetOwner(RenderObject* owner) { m_owner = owner; }
	Scene* GetScene() const;

	WindowManager* GetWindowManager() const;

	void SetEnabled(const bool enabled) { m_enabled = enabled; }
	bool IsEnabled() const { return m_enabled; }

	bool HasStarted() const { return m_started; }
	void SetStarted(const bool started) { m_started = started; }

private:
	RenderObject* m_owner = nullptr;
	bool m_enabled = true;
	bool m_started = false;
};