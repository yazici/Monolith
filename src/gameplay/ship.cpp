#include "ship.hpp"

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
	_file.Write(&m_targetVelocity, sizeof(float));
	_file.Write(&m_targetRotation, sizeof(Math::Quaternion));
	// TODO: store precomputed system data
}

// ********************************************************************* //
void Ship::Load( const Jo::Files::IFile& _file )
{
	Model::Load(_file);
	// Load ship state
	_file.Read(sizeof(float), &m_targetVelocity);
	_file.Read(sizeof(Math::Quaternion), &m_targetRotation);
	// TODO: load precomputed system data
}

// ********************************************************************* //
void Ship::Simulate(float _deltaTime)
{
	// Also simulate the physics
	Model::Simulate(_deltaTime);

	// Do Energy management
	m_primarySystem.Estimate(_deltaTime);
	m_primarySystem.Process(_deltaTime);
}