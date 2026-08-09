#include "GenericUIObjectMeshRenderer.h"
#include "Objects/RenderObject.h"

void GenericUIObjectMeshRenderer::OwnerSetCallback()
{
	GetOwner()->SetMaterialName(GenericUIObjectMaterial::GetNameStatic());
}