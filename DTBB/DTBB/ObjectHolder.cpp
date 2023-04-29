#include "stdafx.h"

#include "ObjectHolder.h"

#include "Engine/GameObject.h"

#include "TurretEnemy.h"

void ObjectHolder::Link() {

}

void ObjectHolder::OnCollide(GameObject* other) {
	for (GameObject* p_attach : attachments) {
		// Turret Enemy
		if (TurretEnemy* p_te = p_attach->HasComponent<TurretEnemy>()) {
			// works because TurretEnemy top game object does not have a rigidbody
			p_te->OnCollide(other);
		}
	}
}

void ObjectHolder::Update(Float32 dt) {

}

void ObjectHolder::AddAttachment(GameObject* _p_attach) {
	attachments.push_back(_p_attach);
}

void ObjectHolder::RemoveAttachment(GameObject* _p_attach) {
	for (Uint32 i = 0; i < attachments.size(); ++i) {
		GameObject* attachment = attachments[i];
		if (attachment == _p_attach) {
			attachments.erase(attachments.begin() + i);
		}
	}
}

Vector<GameObject*> const& ObjectHolder::GetAttachedObjects() {
	return attachments;
}

BEGIN_ATTRIBUTES_FOR(ObjectHolder)
END_ATTRIBUTES