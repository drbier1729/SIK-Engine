#pragma once
#include "Engine/Component.h"

class Anchored : public Component {
public:
	VALID_COMPONENT(Anchored);
	ALLOW_PRIVATE_REFLECTION;

	Anchored();
	~Anchored() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Modify(rapidjson::Value const& json_value) override;
	/*
	* Sets the anchored_to object pointer
	* Sets the initial position chain links and segments behind the anchored object
	* Returns: void
	*/
	void Link() override;

	/*
	* Enables the chain link and segment objects that are created for this.
	* Returns: void
	*/
	void Enable();

	/*
	* Disables the chain link and segment objects that are created for this.
	* Returns: void
	*/
	void Disable() override;

	/*
	* Functions to add more chain links and segments
	* Args:
	*	add_count - The number of links to add. Defaults to 1
	* Returns:
	* void
	*/
	void AddChainLink(Uint8 add_count = 1);

	/*
	* Functions to remove chain links and segments
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
	void FixedUpdate(Float32 dt);

	/*
	* Sets the chain segment positions based on the chain links
	* Returns: void
	*/
	void Update(Float32 dt);

private:
	Uint8 chain_links_count;
	Float32 spring_coeff;
	Float32 damper_coeff;
	Float32 spring_rest_length;
	const char* anchored_to_name;
	GameObject* p_anchored_to_obj;
	GameObject* anchor_point_obj;
	Vec3 anchor_offset;
	Vector<GameObject*> chain_link_objects;
	Vector<GameObject*> chain_segment_objects;

	/*
	* Calculates and applies spring forces on the "curr_obj"
	* based on the position and velocities of the "prev_obj" and "next_obj"
	* Returns: void
	*/
	void ApplySpringForce(GameObject* prev_obj, GameObject* curr_obj, GameObject* next_obj);

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
};

