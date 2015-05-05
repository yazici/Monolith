#pragma once

#include "voxel/model.hpp"
#include "computersystem.hpp"

/// \brief A ship is a model with additional simulation and control options.
/// \details A standard ship design contains a single computer with one
///		instance of each other system as sub-system.
///
///		A ship is an abstraction layer. Models can be converted to ships and
///		the other way around.
class Ship: public Voxel::Model
{
public:
	/// \brief Create a ship with an empty model (only the central computer is added).
	Ship();

	/// \brief Convert a model into a ship
	/// \param [inout] _model Model which is handled as a ship.
	///	\param [in] _centralComputer Each ship needs a computer. It is set to the given position
	//Ship(Voxel::Model* _model, const Math::IVec3& _centralComputer);

	~Ship();

	/// \brief Add a new component to the ship.
	void AddComponent(const Math::IVec3& _position, Voxel::ComponentType _type);

	/// \brief Remove a component because it is destroyed or in editor...
	void RemoveComponent(const Math::IVec3& _position);

	/// \brief Store this ship to a file.
	void Save( Jo::Files::IFile& _file ) const;

	/// \brief Load ship from file
	void Load( const Jo::Files::IFile& _file );

	/// \brief Player or AI can set the velocity. The drives automatically try
	///		to reach it.
	void SetTargetVelocity( float _targetVelocity )	{ m_targetVelocity = _targetVelocity; }

	/// \brief Player or AI can set the rotation. The drives automatically try
	///		to reach it.
	void SetTargetRotation( const Math::Quaternion& _rotation )	{ m_targetRotation = _rotation; }

	/// \brief Get current ship rotation.
	const Math::Quaternion& GetRotation() const { return Model::GetRotation(); }
	const Math::IVec3& GetCentralComputerPosition() const { return m_centralComputerPosition; }

	/// \brief Simulate ship logic
	virtual void Simulate(float _deltaTime) override;

protected:
	Mechanics::ComputerSystem m_primarySystem;
	Math::IVec3 m_centralComputerPosition;
	/*int m_ticks;		///< Simulation counter which reduces the number of specific simulation steps. Is reset to 0 when it reaches WEAPON_TICKS * THRUSTER_TICKS * ....
	// individual systems every n ticks. This list defines the number of frames
	// between each simulation call. One means every frame.
	const int WEAPON_TICKS = 3;
	const int DRIVE_TICKS = 1;
	const int COMPUTER_TICKS = 2;*/

	// Controllable state
	Math::Quaternion m_targetRotation;
	float m_targetVelocity;
};