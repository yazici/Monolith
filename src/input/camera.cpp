#include "camera.hpp"
#include "voxel/model.hpp"
#include "graphic/core/uniformbuffer.hpp"

using namespace Math;

namespace Input {

	// ********************************************************************* //
	Camera::Camera( const Math::FixVec3& _position, const Math::Quaternion& _rotation,
				float _fov, float _aspect ) :
		m_renderTransformation( _position, _rotation ),
		m_latestTransformation( _position, _rotation ),
		m_fov( _fov ),
		m_aspect( _aspect ),
		m_attachMode( REFERENCE_ONLY ),
		m_phi(0.0f),
		m_theta(0.0f),
		m_nearPlane(5.0f),
		m_farPlane(50000.0f)
	{
		m_rotationMatrix = Mat3x3::Rotation(m_latestTransformation.GetRotation());
		UpdateMatrices();
	}

	// ********************************************************************* //
	void Camera::Set( Graphic::UniformBuffer& _cameraUBO )
	{
		_cameraUBO["Projection"] = Vec4(GetProjection()(0,0), GetProjection()(1,1), GetProjection()(2,2), GetProjection()(3,2));
		_cameraUBO["ProjectionInverse"] = m_inverseProjection;
		_cameraUBO["NearPlane"] = m_nearPlane;
		_cameraUBO["FarPlane"] = m_farPlane;
	}

	// ********************************************************************* //
	Vec3 Camera::GetReferencePosition() const
	{
		if( m_attachMode != REFERENCE_ONLY ) return Mat3x3::Rotation(m_renderTransformation.GetRotation()) * m_referencePos;
		return Vec3(m_attachedTo->GetPosition() - m_renderTransformation.GetPosition());
	}

	// ********************************************************************* //
	void Camera::UpdateMatrices()
	{
		m_mutex.lock();
		if( m_attachMode != REFERENCE_ONLY ) NormalizeReference();
		m_renderTransformation = m_latestTransformation;
		m_projection = Math::Mat4x4::Projection( m_fov, m_aspect, m_nearPlane, m_farPlane );
		m_mutex.unlock();
		// construct explicit invert-vector
		m_inverseProjection = Vec4(1.0f/GetProjection()(0,0), 1.0f/GetProjection()(1,1), 1.0f/GetProjection()(2,2), -GetProjection()(3,2) / GetProjection()(2,2));

		// Compute frustum planes from viewProjection
		// http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
		m_frustum[0] = m_projection.Column(3) + m_projection.Column(0);
		m_frustum[1] = m_projection.Column(3) - m_projection.Column(0);
		m_frustum[2] = m_projection.Column(3) + m_projection.Column(1);
		m_frustum[3] = m_projection.Column(3) - m_projection.Column(1);
		m_frustum[4] = m_projection.Column(3) + m_projection.Column(2);
		m_frustum[5] = m_projection.Column(3) - m_projection.Column(2);
	}


	// ********************************************************************* //
	void Camera::Rotate( float _theta, float _phi )
	{
		m_phi += _phi;
		m_theta += _theta;
		m_mutex.lock();
		if( m_attachedTo ) {
			// Do an object relative rotation.
			// In soft case the reference point is recomputed since we don't
			// want to jump back but center movement at the object.
			if( m_attachMode == REFERENCE_ONLY )
				m_referencePos = m_latestTransformation.Transform( m_attachedTo->GetPosition() );
			// Additionally the rotation is either relative to the world or to the object.
			m_worldRotation = Quaternion( 0.0f, m_phi, 0.0f ) * Quaternion( -m_theta, 0.0f, 0.0f );
			if( m_attachMode != FOLLOW_AND_ROTATE )
			{
				m_latestTransformation.SetRotation( m_worldRotation );
				m_rotationMatrix = Mat3x3::Rotation( m_latestTransformation.GetRotation() );
			}
			NormalizeReference();
		} else {
			//m_worldRotation *= ~Math::Quaternion( _theta, _phi, 0.0f );
			m_worldRotation = Quaternion( 0.0f, m_phi, 0.0f ) * Quaternion( -m_theta, 0.0f, 0.0f );
			m_latestTransformation.SetRotation( m_worldRotation );
			m_rotationMatrix = Mat3x3::Rotation( m_latestTransformation.GetRotation() );
		}
		m_mutex.unlock();
	}

	// ********************************************************************* //
	void Camera::Move( float _dx, float _dy )
	{
		// Scale _dy and _dx depending on object distance
		if( m_attachedTo )
		{
			_dx *= m_referencePos[2] * m_fov;
			_dy *= m_referencePos[2] * m_fov;
		}
		// Compute actual xy directions in camera space
		m_latestTransformation.Translate( m_rotationMatrix.YAxis() * _dy - m_rotationMatrix.XAxis() * _dx );
		// Make soft attachment
		m_attachMode = REFERENCE_ONLY;
	}

	// ********************************************************************* //
	void Camera::Scroll( float _dz )
	{
		if( m_attachedTo )
		{
			// Increasing scroll speed with increasing distance.
			_dz = m_referencePos[2] * _dz * 0.01f;
			// Avoid scrolling into the object
			_dz = Math::min( _dz, -m_attachedTo->GetRadius() * 0.75f + m_referencePos[2] );
		} // Linear movement otherwise

		m_latestTransformation.Translate( m_rotationMatrix.ZAxis() * _dz );
		m_referencePos[2] -= _dz;
	}

	// ********************************************************************* //
	void Camera::ZoomAt( const Voxel::Model& _model, AttachMode _mode )
	{
		// Rotate camera back to the world space if necessary
		if( m_attachedTo && _mode == FOLLOW_AND_ROTATE )
		{
			// TODO: Get model theta and phi to transform the relative rotation
			// to an absolute one.
		}

		m_attachedTo = &_model;
		m_attachMode = _mode;
		m_referencePos[0] = 0.0f;
		m_referencePos[1] = 0.0f;
		// Compute required distance to the model
		m_referencePos[2] = 2.0f * _model.GetRadius() / m_fov;

		m_mutex.lock();
		NormalizeReference();
		m_mutex.unlock();
	}

	// ********************************************************************* //
	void Camera::Attach( const Voxel::Model* _model, AttachMode _mode )
	{
		// Rotate camera back to the world space if necessary
		if( m_attachedTo && _mode == FOLLOW_AND_ROTATE )
		{
			// TODO: Get model theta and phi to transform the relative rotation
			// to an absolute one.
		}

		m_attachedTo = _model;
		m_attachMode = _mode;
		// Compute actual reference frame.
		m_referencePos = m_latestTransformation.Transform( _model->GetPosition() );

		m_mutex.lock();
		NormalizeReference();
		m_mutex.unlock();
	}

	// ********************************************************************* //
	void Camera::NormalizeReference()
	{
		// Transform by rotation inverse (which is multiplying from left for
		// rotations)
		m_latestTransformation.SetPosition( m_attachedTo->GetPosition() - FixVec3(m_rotationMatrix * m_referencePos) );
		// Object might be rotated -> 
		if( m_attachMode == FOLLOW_AND_ROTATE )
		{
			m_latestTransformation.SetRotation( ~m_attachedTo->GetRotation() * m_worldRotation );
			m_rotationMatrix = Mat3x3::Rotation( m_latestTransformation.GetRotation() );
		}
	}

	// ********************************************************************* //
	bool Camera::IsVisible( const Math::Sphere& _S ) const
	{
		// TODO: potential optimization: Dot-Product-Culling

		// Check against all 6 planes. If the sphere is outside any half space
		// it is outside of the whole volume.
		for( int i=0; i<6; ++i )
		{
			if( m_frustum[i].DotCoords( _S.m_center ) <= -_S.m_radius )
				return false;
		}

		return true;
	}

	// ********************************************************************* //
	WorldRay Camera::GetRay(const Vec2& _screenSpaceCoordinate) const
	{
		WorldRay ray;
		// Compute view space position of a point on the near plane
		Vec3 nearPoint = Vec3( m_inverseProjection[0] * _screenSpaceCoordinate[0],
							   m_inverseProjection[1] * _screenSpaceCoordinate[1],
							   1.0f );	// 1.0 result of inverse project of any coordinate
										// Division by 
		ray.origin = m_latestTransformation.TransformInverse(nearPoint);
		ray.direction = normalize(m_rotationMatrix * nearPoint);
		return ray;
	}

} // namespace Input