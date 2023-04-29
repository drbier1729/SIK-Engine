#pragma once

#include "Engine/Component.h"

class GameObject;
struct ParticleEmitter;
struct RigidBody;

class BallnChain : public Component {
public:
	VALID_COMPONENT(BallnChain);
	ALLOW_PRIVATE_REFLECTION;

	BallnChain();
	~BallnChain() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Modify(rapidjson::Value const& json_value) override;
	/*
	* Sets the parent object pointer
	* Sets the initial position of the ball and chain behind the parent object
	* Returns: void
	*/
	void Link() override;

	/*
	* Enables the chain link objects that are created for this.
	* Returns: void
	*/
	void Enable();

	/*
	* Disables the chain link objects that are created for this.
	* Returns: void
	*/
	void Disable() override;

	/*
	* Functions to add more chain links
	* Args:
	*	add_count - The number of links to add. Defaults to 1
	* Returns:
	* void
	*/
	void AddChainLink(Uint8 add_count = 1);

	/*
	* Functions to remove chain links.
	* Cannot reduce the link count below 3.
	* Args:
	*	remove_count - The number of links to add. Defaults to 1
	* Returns:
	* void
	*/
	void RemoveChainLink(Uint8 remove_count = 1);

	/*
	* Uses a spring-damper system to generate forces on each chain link 
	* that propagates from the parent object to the Ball
	* Returns: void
	*/
	void FixedUpdate(Float32 dt) override;

	/*
	* Update the chain links objects and wrecking ball
	* Returns: void
	*/
	void Update(Float32 dt) override;

	/*
	* Calculates damage based on velocity and mass.
	* Returns: Int32 - Damage dealt
	*/
	Int32 CalculateDamage();

	/*
	* Logic to deal damage to other objects with the wrecking ball.
	* Returns: void
	*/
	void OnCollide(GameObject* other) override;

	/*
	* Resets the balln chain
	* Returns: void
	*/
	void Reset() override;

	Vec3 const& GetAnchorOffset();
	void SetAnchorOffset(Vec3 const& _anchor_offset);

private:
	Uint8 chain_links_count;
	Float32 ball_mass;
	Float32 spring_coeff;
	Float32 damper_coeff;
	Float32 spring_rest_length;
	const char* parent_obj_name;
	GameObject* p_parent_obj;
	GameObject* anchor_obj;
	Vec3 anchor_offset;
	Vector<GameObject*> chain_link_objects;
	Vector<GameObject*> chain_segment_objects;

	ParticleEmitter* p_emitter;

	/*
	* Calculates and applies spring forces on the "curr_obj"
	* based on the position and velocities of the "prev_obj" and "next_obj"
	* Returns: void
	*/
	void ApplySpringForce(GameObject* prev_obj, GameObject* curr_obj, GameObject* next_obj);

	void EmitterChecks();

	/*
	* Checks if parent object wants to grow or shrink the chain,
	* then does so. Only works if parent object has a CarController
	* component.
	* Returns:
	* void
	*/
	void UpdateChainLength();


	/*
	* Generates small random upward forces depending on the ball's
	* vertical position and speed to emulate skidding and bounce
	* Returns:
	* void
	*/
	void ApplyBounceForce(RigidBody* ball_rb);


	/*
	* Applies a position correction to owner_rb based on proximity to
	* parent_rb. Called within FixedUpdate. (Takes args because pointers
	* are already available within the current FixedUpdate method.)
	* 
	* TODO: this seems to work, but since it is directly manipulating
	* position, there may be some instability or other quirks. May want
	* to change to Force-based if this is the case.
	* 
	* Returns: void
	*/
	void MoveWreckingBallAwayFromParent(RigidBody* owner_rb, RigidBody* parent_rb);

	/*
	* Applies a small force toward the closest TurretEnemy or GenericCarEnemy
	*
	* Returns: void
	*/
	void SeekClosestEnemy(RigidBody* ball_rb);

	/*
	* Sets the transforms on the chain segments based on the positions
	* of the chain links
	* Returns: void
	*/
	void SetSegmentTransforms();

	/*
	* Sets the transform on an individual segment.
	* Returns: void
	*/
	static void SetSegmentTransform(
		GameObject* prev_obj, GameObject* next_obj, GameObject* segment_object);

	/*
	* Resets the Ball position to an appropriately spaced distance behind the car.
	* Also Sets the chain links to similarly spaced positions.
	* Returns: void
	*/
	void ResetBallPosition();
};

