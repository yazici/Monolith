/// \file predeclarations.hpp // TODO: test doxygen
/// \brief A list of predeclarations for all classes in alphabetical order.

#pragma once

#include <cstdint>

#include "utilities/logger.hpp"

// External things
struct GLFWwindow;

// General classes without scopes
class GSMainMenu;
class GSPlay;
class GSEditor;
class GSEditorChoice;
class GSGameplayOpt;
class GSGraphicOpt;
class GSInputOpt;
class GSSoundOpt;
class IGameState;
typedef IGameState* IGameStateP;
class Monolith;

namespace Generators {
	class Asteroid;
} // namespace Generators

namespace Graphic {
	class Effect;
	class Hud;
	class SamplerState;
	class Texture;
	class UniformBuffer;
	class Font;
	class Framebuffer;
	class Texture;
	class PostProcessing;

	namespace Marker {
		class Grid;
		class Box;
	}
} // namespace Graphic

namespace Input {
	class Camera;
	class Manager;

	enum struct VirtualKey;
} // namespace Input

namespace Math {
	class Mat4x4;
	class Mat3x3;
	class Quaternion;
	class Plane;
	struct Sphere;
	struct Ray;
	struct WorldRay;
	template<int n, class Data>	class Vector;
//	template<int n>	class FixedPoint;
//	typedef FixedPoint<30> Fix;
	typedef Vector<2,int> IVec2;
	typedef Vector<3,int> IVec3;
	typedef Vector<4,int> IVec4;
	typedef Vector<2,float> Vec2;
	typedef Vector<3,float> Vec3;
	typedef Vector<4,float> Vec4;
//	typedef Vector<3,FixedPoint<30>> FixVec3;
	class Transformation;
	template<int N> class CubeMap;
	/// A typedef to have global control about the tessellation refinement
	typedef CubeMap<3> SphericalFunction;
} // namespace Math

namespace Utils {
	class Color32F;
	class Color8U;
} // namespace Utils


namespace Voxel {
	class Chunk;
	class Model;
	enum struct VoxelType: uint8_t;
	struct VoxelVertex;
	struct Material;
	struct Component;
} // namespace Voxel
