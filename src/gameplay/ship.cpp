#include "ship.hpp"

using namespace Math;


Ship::Ship() :
	m_primarySystem(*this, "CentralComputer"),
	m_targetVelocity(0.0f),
	m_centralComputerPosition(2012, 2012, 2012)
{
	Model::Set(m_centralComputerPosition, Voxel::ComponentType::COMPUTER);
}

// ********************************************************************* //
/*Ship::Ship(Voxel::Model* _model, const Math::IVec3& _centralComputer) :
	m_primarySystem("CentralComputer"),
	m_model( _model ),
	m_modelHandle( _model ),
	m_centralComputerPosition(_centralComputer),
	m_targetVelocity( 0.0f ),
	m_targetRotation( _model->GetRotation() )
{
	// Make sure there is a computer
	m_model->Set(m_centralComputerPosition, Voxel::ComponentType::COMPUTER);
	// TODO OnAdd for all components!
}*/

// ********************************************************************* //
Ship::~Ship()
{
}

// ********************************************************************* //
void Ship::AddComponent(const Math::IVec3& _position, Voxel::ComponentType _type)
{
	Model::Set(_position, _type);
	// Add to systems after insertion
	m_primarySystem.OnAdd(_position, _type);
	// Report the change to each neighbor
	// TODO: first sample the area, than call OnChange(area)
}

// ********************************************************************* //
void Ship::RemoveComponent(const Math::IVec3& _position)
{
	if( _position == m_centralComputerPosition )
		throw "You cant remove the central computer! This leads to unimplemented handling (conversion to model/game over...)";
	Voxel::ComponentType type = Model::Get(_position);
	Model::Set(_position, Voxel::ComponentType::UNDEFINED);
	m_primarySystem.OnRemove(_position, type);
	// Report the change to each neighbor
	// TODO: first sample the area, than call OnChange(area)
}

// ********************************************************************* //
void Ship::Save( Jo::Files::IFile& _file ) const
{
	Model::Save(_file);
	// Store ship state
	_file.Write(&m_targetVelocity, sizeof(Math::Vec3));
	_file.Write(&m_targetAngularVelocity, sizeof(Math::Vec3));
	// TODO: store precomputed system data
}

// ********************************************************************* //
void Ship::Load( const Jo::Files::IFile& _file )
{
	Model::Load(_file);
	// Load ship state
	_file.Read(sizeof(float), &m_targetVelocity);
	_file.Read(sizeof(Math::Vec3), &m_targetAngularVelocity);
	// TODO: load precomputed system data
}

// ********************************************************************* //
void Ship::Simulate(float _deltaTime)
{
	Mechanics::SystemRequierements requirements;
	// Estimate the required forces to reach the target velocities
	requirements.torque = m_inertiaTensor * (m_targetAngularVelocity - Model::m_angularVelocity) / _deltaTime;
	requirements.thrust = (m_mass / _deltaTime) * (m_targetVelocity - Model::m_velocity);

	// Do Energy management
	m_primarySystem.Estimate(_deltaTime, requirements);
	m_primarySystem.Process(_deltaTime, requirements);

	// Accelerate dependent on available torque and force.
	Model::m_angularVelocity += _deltaTime * (m_inertiaTensorInverse * requirements.torque);
	Model::m_velocity += (_deltaTime / m_mass) * requirements.thrust;

	// Also simulate the physics
	Model::Simulate(_deltaTime);
}