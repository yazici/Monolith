#include "ray.hpp"
#include "transformation.hpp"

namespace Math {

// ************************************************************************* //
// Create relative ray from world space ray
ei::Ray WorldRay::getRelativeRay( const Transformation& _reference ) const
{
	const ei::Mat3x3& rotation = _reference.GetRotationMatrix();
	// Get a relative start position
	ei::Vec3 newOrig = ei::Vec3(
		float(origin[0] - _reference.GetPosition()[0]),
		float(origin[1] - _reference.GetPosition()[1]),
		float(origin[2] - _reference.GetPosition()[2]) );
	newOrig = rotation * newOrig;//TODO: transpose?

	// Rotate direction into relative system
	ei::Vec3 dir = rotation * direction;//TODO: transpose?
	dir = normalize(direction);
	return ei::Ray(newOrig, dir);
}

} // namespace Math