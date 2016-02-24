#pragma once

#include <ei/3dtypes.hpp>
#include <hybridarray.hpp>
#include <vector>
#include <memory>

#include "utilities/metaproghelper.hpp"
#include "utilities/assert.hpp"
//#include "utilities/flagoperators.hpp"

namespace Graphic {

	/// \brief Component based particle systems.
	/// \details There are different rendering systems with different parameters.
	///		A particle can also have different simulation components. Everything can
	///		be combined arbitrary.
	namespace ParticleSystems
	{
		/// \brief A list of valid components for different particle system definitions.
		/// \details All flags are orthogonal and any combination is valid. The
		///		functionality is inferred automatically. E.g. If you add POSITION
		///		and VELOCITY the particle will be moved each frame.
		struct Component {
			enum Val: uint
			{
				POSITION = 1,		///< Every particle needs a position. Always add this component!
				VELOCITY = 2,		///< Change of position over time.
				LIFETIME = 4,		///< Lifetime in seconds until despawn. If not provided the particle lasts forever.
				GRAVITATION = 8,	///< Add a single center of gravitation which draws all particles to it or pushes them away.
			};
		};

		enum struct RenderType
		{
			BLOB,
			QUAD,
			BOX,
			RAY
		};

		/// \brief Add a custom particle to the appropriate system. Simulation
		///		and rendering will be done efficiently together with all other
		///		particles of the same type.
		/*template<uint PFlags, typename... Params>
		static void AddParticle(Params... _params)
		{
			SubSystems<PFlags>::Get().AddParticle<1, PFlags>(_params...);
		}*/

		class Manager
		{
		public:
			/// \brief Simulate all kinds of particle systems.
			static void Simulate(float _deltaTime);
		private:
			template<uint> friend class System;
			/// \brief Register a new system to the global management.
			///		This is called automatically!
			static void Register(class SystemActions* _system);
			/// \brief Find and remove from the list. This is called automatically!
			static void Release(class SystemActions* _system);

			// A list of pointers to all instances of particle systems.
			static std::vector<class SystemActions*> m_registeredSystems;
		};

		/// \brief Helper class for component system meta programming.
		struct NoComponent
		{
			template<typename T>
			static void Add(const T&) {}
			static void Remove(size_t _idx) {}
		};

		/// \brief Position components for all particles of a system
		struct PositionComponents
		{
			std::vector<ei::Vec3> m_positions;
			void Add(const ei::Vec3& _pos) { m_positions.push_back(_pos); }
			void Remove(size_t _idx) { m_positions[_idx] = m_positions.back(); m_positions.pop_back(); }
		};

		/// \brief Velocity components for all particles of a system
		struct VeloctiyComponents
		{
			std::vector<ei::Vec3> m_velocities;
			void Add(const ei::Vec3& _vel) { m_velocities.push_back(_vel); }
			void Remove(size_t _idx) { m_velocities[_idx] = m_velocities.back(); m_velocities.pop_back(); }
		};

		/// \brief Lifetimes in seconds for all particles of a system
		struct LifetimeComponents
		{
			std::vector<float> m_lifetimes;
			void Add(float _life) { m_lifetimes.push_back(_life); }
			void Remove(size_t _idx) { m_lifetimes[_idx] = m_lifetimes.back(); m_lifetimes.pop_back(); }
		};

		/// \brief System wide component of a gravitation center.
		struct GravitationComponent
		{
			ei::Vec3 m_gravitationCenter;
			float m_gravitation;	///< Gravitation constant. Positive means pull and negative is anti-gravitation.
		};

		/// \brief Helper class for component system meta programming.
		template<typename Base>
		struct FuncNOP
		{
			void Run(float _deltaTime) {}
		};

		/// \brief Simulation component. Automatically added when POSITION and VELOCITY are given
		template<typename Base>
		struct FuncAdvection : public virtual Base
		{
			void Run(float _deltaTime)
			{
				for(size_t i = 0; i < m_velocities.size(); ++i)
					m_positions[i] += m_velocities[i];
			}
		};

		/// \brief Simulation component decreasing the lifetime and despawning particles.
		template<typename Base>
		struct FuncDie : public virtual Base
		{
			void Run(float _deltaTime)
			{
				for(size_t i = 0; i < m_lifetimes.size(); ++i)
				{
					m_lifetimes[i] -= _deltaTime;
					if(m_lifetimes[i] < 0.0f)
						Remove(i--);
				}
			}
		};

		/// \brief Simulation component. Automatically added when GRAVITATION and VELOCITY are given
		template<typename Base>
		struct FuncGravitation : public virtual Base
		{
			void Run(float _deltaTime)
			{
				for(size_t i = 0; i < m_velocities.size(); ++i)
				{
					ei::Vec3 toCenter = m_gravitationCenter - m_positions[i];
					float dSq = lensq(toCenter);
					// Normalize and scale with gravitation in one step
					m_velocities[i] += toCenter * (m_gravitation / (dSq * sqrt(dSq)));
				}
			}
		};

		// The SystemData creates a type which contains the specific properties
		// defined by the flag word. This is the aggregation of different components into
		// a single identity.
		// It further creates an AddParticle which properly called only succeeds if an
		// initial value for each property is given with the proper type.
		// Example: SystemData<POSTITION | VELOCITY> will contain the arrays
		// m_positions and m_velocity.
		template<uint PFlags>
		class SystemData :
			public inherit_conditional<(PFlags & Component::POSITION) != 0, PositionComponents, NoComponent>,
			public inherit_conditional<(PFlags & Component::VELOCITY) != 0, VeloctiyComponents, NoComponent>,
			public inherit_conditional<(PFlags & Component::LIFETIME) != 0, LifetimeComponents, NoComponent>,
			public inherit_conditional<(PFlags & Component::GRAVITATION) != 0, GravitationComponent, NoComponent>
		{
			#define PFlagsWOGlobal  (PFlags & ~(Component::GRAVITATION))
			// Compiletime while loop which finds the next flag set in PFlags
			template<uint TheFlag, bool>
			struct NextFlag {
				static const uint Get = TheFlag;
			};
			template<uint TheFlag>
			struct NextFlag<TheFlag, false> {
				static const uint Get = NextFlag<TheFlag << 1, ((TheFlag << 1) & PFlagsWOGlobal) != 0 || TheFlag == 0>::Get;
			};
		public:
			template<uint TheFlag, uint RemainingFlags>
			void AddParticle() // End of recursion
			{
				Assert(RemainingFlags == 0, "Too few parameters provided!");
			}

			template<uint TheFlag = 1, uint RemainingFlags = (PFlags & ~(Component::GRAVITATION)), typename Param0, typename... Params>
			void AddParticle(const Param0& _param0, Params... _params)
			{
				Assert(TheFlag != 0, "Too many parameters provided!");
				// The flag is set -> consume the argument.
				inherit_conditional<(TheFlag & PFlags) != 0 && (TheFlag & Component::POSITION) != 0, PositionComponents, NoComponent>::Add(_param0);
				inherit_conditional<(TheFlag & PFlags) != 0 && (TheFlag & Component::VELOCITY) != 0, VeloctiyComponents, NoComponent>::Add(_param0);
				inherit_conditional<(TheFlag & PFlags) != 0 && (TheFlag & Component::LIFETIME) != 0, LifetimeComponents, NoComponent>::Add(_param0);
				AddParticle<NextFlag<TheFlag, false>::Get, RemainingFlags ^ TheFlag>(_params...);
			}

			void Remove(size_t _idx)
			{
				inherit_conditional<(PFlags & (uint)Component::POSITION) != 0, PositionComponents, NoComponent>::Remove(_idx);
				inherit_conditional<(PFlags & (uint)Component::VELOCITY) != 0, VeloctiyComponents, NoComponent>::Remove(_idx);
				inherit_conditional<(PFlags & (uint)Component::LIFETIME) != 0, LifetimeComponents, NoComponent>::Remove(_idx);
			}
			#undef PFlagsWOGlobal
		};

		// Base class to make functions of particle systems callable independent of their
		// type.
		class SystemActions
		{
		public:
			virtual void Simulate(float _deltaTime) {}
		};

		// The SubSystem extends the SubSystemData by mixing in functions conditional.
		// Dependent on the available components different simulation functions are glued
		// together.
		// Conditional inheritance is used to execute code conditional which otherwise
		// would not compile. The trick is that an empty dummy function from the default
		// type gets called instead of invalid code if a condition is not fulfilled.
		template<uint PFlags>
		class System :
			public virtual SystemData<PFlags>,
			public inherit_conditional<(PFlags & Component::POSITION) != 0 && (PFlags & Component::VELOCITY) != 0, FuncAdvection<SystemData<PFlags>>, FuncNOP<SystemData<PFlags>>>,
			public inherit_conditional<(PFlags & Component::LIFETIME) != 0, FuncDie<SystemData<PFlags>>, FuncNOP<SystemData<PFlags>>>,
			public inherit_conditional<(PFlags & Component::POSITION) != 0 && (PFlags & Component::VELOCITY) != 0 && (PFlags & Component::GRAVITATION) != 0, FuncGravitation<SystemData<PFlags>>, FuncNOP<SystemData<PFlags>>>,
			public SystemActions
		{
		public:
			System()
			{
				// Register the system where the runtime can see it.
				Manager::Register(this);
			}
			~System()
			{
				Manager::Release(this);
			}

			void Simulate(float _deltaTime) override
			{
				inherit_conditional<((PFlags & Component::POSITION) != 0) && ((PFlags & Component::VELOCITY) != 0), FuncAdvection<SystemData<PFlags>>, FuncNOP<SystemData<PFlags>>>::Run(_deltaTime);
				inherit_conditional<((PFlags & Component::LIFETIME) != 0), FuncDie<SystemData<PFlags>>, FuncNOP<SystemData<PFlags>>>::Run(_deltaTime);
				inherit_conditional<((PFlags & Component::POSITION) != 0) && ((PFlags & Component::VELOCITY) != 0) && ((PFlags & Component::GRAVITATION) != 0), FuncGravitation<SystemData<PFlags>>, FuncNOP<SystemData<PFlags>>>::Run(_deltaTime);
			}
		};

		// Kind of a hash-map to instantiate the dynamical produced types. For each type
		// which is used in the program a single instance will be created and mapped.
		// Thus all unused combinations of flags will not cost us anything.
		// I.e. the mapping is sparse!
		/*template<uint Key>
		class SubSystems {
		public:
			static SubSystem<Key>& Get () {
				static SubSystem<Key> v;
				return v;
			}
		};*/
	}

	using PSComponent = ParticleSystems::Component;

} // namespace Graphic