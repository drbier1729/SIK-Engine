#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward declarations
class GameObject;

class ObjectHolder : public Component {
public:
	VALID_COMPONENT(ObjectHolder);
	ALLOW_PRIVATE_REFLECTION;
	DEFAULT_SERIALIZE(ObjectHolder);

	ObjectHolder() = default;
	~ObjectHolder() noexcept = default;

	void Link() override;
	void OnCollide(GameObject* other) override;

	void Update(Float32 dt);

	void AddAttachment(GameObject* _p_attach);
	void RemoveAttachment(GameObject* _p_attach);

	Vector<GameObject*> const& GetAttachedObjects();

private:
	Vector<GameObject*> attachments;
};