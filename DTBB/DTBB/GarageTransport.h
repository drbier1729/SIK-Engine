#pragma once
#include "Engine/Component.h"
#include "Engine/Serializer.h"
#include "Engine/GUIObject.h"

struct LocalLight;

class GarageTransport : public Component {
public:
	VALID_COMPONENT(GarageTransport);
	ALLOW_PRIVATE_REFLECTION;

	GarageTransport();
	~GarageTransport() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);

	void Link() override;

	void Update(Float32 dt);

	void Disable() override;
	void Enable() override;

private:
	LocalLight* p_light;
	Bool can_transport;
	Bool enabled;

	static Vector<GUIObject*> gui_objects;

	// Serializable members
	Float32 transport_radius;
	Vec3 light_color;

	/*
	* Checks if the player is within the transport area
	* Sets the can_transport flag
	* Returns: void
	*/
	void CheckPlayerPosition();

	/*
	* Shows/hides the GUI prompt to enter the garage based on the can_transport flag
	* Returns: void
	*/
	void UpdateUIPrompt(Float32 dt);
};

