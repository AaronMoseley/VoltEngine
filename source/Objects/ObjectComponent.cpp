#include "ObjectComponent.h"
#include "Objects/RenderObject.h"

Scene* ObjectComponent::GetScene() const
{
	if (m_owner == nullptr)
	{
		return nullptr;
	}

	return m_owner->GetSceneManager();
}

WindowManager* ObjectComponent::GetWindowManager() const
{
	RenderObject* owner = GetOwner();

	if (owner == nullptr)
	{
		return nullptr;
	}

	return GetOwner()->GetWindowManager();
}
