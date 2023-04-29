#pragma once
#include "Test.h"

#include "Engine/Collision.h"

class CollisionPrimitivesTest : public Test
{
public:
	void Setup(EngineExport*) override;

	void Run() override;

	void Teardown() override;

private:
	inline void NextCollider();

private:
	Collision::Collider::Type controlled = Collision::Collider::Type::Sphere;
};

inline void CollisionPrimitivesTest::NextCollider() {
	using Collision::Collider;

	switch (controlled)
	{
		break; case Collider::Type::Sphere:
		{
			controlled = Collider::Type::Capsule;
		}
		break; case Collider::Type::Capsule:
		{
			controlled = Collider::Type::Hull;
		}
		break; case Collider::Type::Hull:
		{
			controlled = Collider::Type::Sphere;
		}
	}
}
