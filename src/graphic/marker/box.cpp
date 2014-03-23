#include "box.hpp"
#include "../../math/math.hpp"
using namespace Math;

namespace Graphic {
namespace Marker {

	// ********************************************************************* //
	// Create a wireframe box.
	Box::Box( const Math::Vec3& _size, float _fading, const Utils::Color32F& _color, Graphic::Content* _stateObjects ) :
		m_renderer( _color, 0.01f, _stateObjects )
	{
		// Invert: makes computations easier and avoid singularities in the edge length.
		_fading = max( 0.001f, 1.0f - _fading );
		Math::Vec3 sizeHalf = _size * 0.5f;

		// For each of the 12 borders create two lines (to enable transparency
		// in the middle).
		// 
		// In X direction
		m_renderer.AddLine( Vec3(-sizeHalf[0],                      -sizeHalf[1], -sizeHalf[2]),
							Vec3(-sizeHalf[0] + _fading * _size[0], -sizeHalf[1], -sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0],                      -sizeHalf[1], -sizeHalf[2]),
							Vec3( sizeHalf[0] - _fading * _size[0], -sizeHalf[1], -sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3(-sizeHalf[0],                       sizeHalf[1], -sizeHalf[2]),
							Vec3(-sizeHalf[0] + _fading * _size[0],  sizeHalf[1], -sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0],                       sizeHalf[1], -sizeHalf[2]),
							Vec3( sizeHalf[0] - _fading * _size[0],  sizeHalf[1], -sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3(-sizeHalf[0],                      -sizeHalf[1],  sizeHalf[2]),
							Vec3(-sizeHalf[0] + _fading * _size[0], -sizeHalf[1],  sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0],                      -sizeHalf[1],  sizeHalf[2]),
							Vec3( sizeHalf[0] - _fading * _size[0], -sizeHalf[1],  sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3(-sizeHalf[0],                       sizeHalf[1],  sizeHalf[2]),
							Vec3(-sizeHalf[0] + _fading * _size[0],  sizeHalf[1],  sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0],                       sizeHalf[1],  sizeHalf[2]),
							Vec3( sizeHalf[0] - _fading * _size[0],  sizeHalf[1],  sizeHalf[2]),
							1.0f, 0.0f );

		// In Y direction
		m_renderer.AddLine( Vec3(-sizeHalf[0], -sizeHalf[1],                      -sizeHalf[2]),
							Vec3(-sizeHalf[0], -sizeHalf[1] + _fading * _size[0], -sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3(-sizeHalf[0],  sizeHalf[1],                      -sizeHalf[2]),
							Vec3(-sizeHalf[0],  sizeHalf[1] - _fading * _size[0], -sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0], -sizeHalf[1],                      -sizeHalf[2]),
							Vec3( sizeHalf[0], -sizeHalf[1] + _fading * _size[0], -sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0],  sizeHalf[1],                      -sizeHalf[2]),
							Vec3( sizeHalf[0],  sizeHalf[1] - _fading * _size[0], -sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3(-sizeHalf[0], -sizeHalf[1],                       sizeHalf[2]),
							Vec3(-sizeHalf[0], -sizeHalf[1] + _fading * _size[0],  sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3(-sizeHalf[0],  sizeHalf[1],                       sizeHalf[2]),
							Vec3(-sizeHalf[0],  sizeHalf[1] - _fading * _size[0],  sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0], -sizeHalf[1],                       sizeHalf[2]),
							Vec3( sizeHalf[0], -sizeHalf[1] + _fading * _size[0],  sizeHalf[2]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0],  sizeHalf[1],                       sizeHalf[2]),
							Vec3( sizeHalf[0],  sizeHalf[1] - _fading * _size[0],  sizeHalf[2]),
							1.0f, 0.0f );

		// In Z direction
		m_renderer.AddLine( Vec3(-sizeHalf[0], -sizeHalf[1], -sizeHalf[2]                     ),
							Vec3(-sizeHalf[0], -sizeHalf[1], -sizeHalf[2] + _fading * _size[0]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3(-sizeHalf[0], -sizeHalf[1],  sizeHalf[2]                     ),
							Vec3(-sizeHalf[0], -sizeHalf[1],  sizeHalf[2] - _fading * _size[0]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0], -sizeHalf[1], -sizeHalf[2]                     ),
							Vec3( sizeHalf[0], -sizeHalf[1], -sizeHalf[2] + _fading * _size[0]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0], -sizeHalf[1],  sizeHalf[2]                     ),
							Vec3( sizeHalf[0], -sizeHalf[1],  sizeHalf[2] - _fading * _size[0]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3(-sizeHalf[0],  sizeHalf[1], -sizeHalf[2]                     ),
							Vec3(-sizeHalf[0],  sizeHalf[1], -sizeHalf[2] + _fading * _size[0]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3(-sizeHalf[0],  sizeHalf[1],  sizeHalf[2]                     ),
							Vec3(-sizeHalf[0],  sizeHalf[1],  sizeHalf[2] - _fading * _size[0]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0],  sizeHalf[1], -sizeHalf[2]                     ),
							Vec3( sizeHalf[0],  sizeHalf[1], -sizeHalf[2] + _fading * _size[0]),
							1.0f, 0.0f );
		m_renderer.AddLine( Vec3( sizeHalf[0],  sizeHalf[1],  sizeHalf[2]                     ),
							Vec3( sizeHalf[0],  sizeHalf[1],  sizeHalf[2] - _fading * _size[0]),
							1.0f, 0.0f );
	}

	// ********************************************************************* //
	// Draw all the lines with the glowing wireframe effect.
	void Box::Draw( const Math::Mat4x4& _worldViewProjection )
	{
		m_renderer.Draw( _worldViewProjection );
	}

} // namespace Marker
} // namespace Graphic