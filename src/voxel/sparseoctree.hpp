#pragma once

#include "../math/vector.hpp"
#include "../algorithm/smallsort.hpp"
#include <poolallocator.hpp>

namespace Voxel {

	/// \brief A generic sparse octree implementation.
	/// \details Each node contains data and there are special operations
	///		assuming that the data are voxels.
	///		
	///		* T is the type of voxel data.
	///		* Listener Must have an Update() method which is called if voxels are changed
	template<typename T, typename Listener>
	class SparseVoxelOctree
	{
	public:
		/// \brief Create an empty octree.
		/// \param [in] _defaultData Initial value used when something must be
		///		set back.
		SparseVoxelOctree( T _defaultData, Listener* _listener );

		/// \brief Set a voxel in the octree and call update of the listener.
		/// \details This method overwrites all covered voxels.
		/// \param [in] _position Position inside the given level.
		/// \param [in] _level Depth in the grid hierarchy. 0 is the maximum
		///		resolution of the voxel grid. The size is the logarithmic edge
		///		length of voxels (2^0).
		/// \param [in] _type Voxel to be set. A voxel equal to _defaultData can
		///		delete a voxel.
		void Set( const Math::IVec3& _position, int _level, T _type );

		/// \brief Returns the type of a voxel on a certain grid level and
		///		position.
		///	\details If the position is outside the return value is _defaultData. For
		///		levels other than 0 the returned value will be some
		///		approximating LOD (majority) of the children.
		T Get( const Math::IVec3& _position, int _level ) const;

		/// \brief Traverse through the whole tree in preorder and call the
		///		callback.
		/// \param [in] _callback This function is called for each voxel. It
		///		has to return true if the children should be traversed too.
		///		If a voxel does not have children the return value is ignored.
		///		TODO: IVec4
		///		The function takes the octree position, the voxels and gets
		///		the parameters passed.
		///	\param [in] _param Pointer to an arbitrary structure to pass
		///		additional information to the callback.
		template<typename Param>
		void Traverse( bool(*_callback)(const Math::IVec4&,T,Param*), Param* _param );

#ifdef _DEBUG
		size_t MemoryConsumption() const;
#endif
	protected:
		T m_defaultData;
		Listener* m_listener;

		/// \brief A Sparse-Voxel-Octree-Node
#		pragma pack(push,1)
		struct SVON {
			SVON* children;
			T voxel;

			SVON(T _defaultData) : children(nullptr), voxel(_defaultData)	{}

			/// \brief Set a voxel on a chosen resolution.
			/// \param [in] _currentSize The voxel size in the current recursion
			///		level. This can be used to compute the full actual position.
			///	\param [in] _position Position where to set the new voxel.
			///	\param [in] _size Logarithmic size of the voxel to be set. 0 is
			///		the layer with the highest resolution.
			///	\param [in] _type The new type.
			/// \param [in] _model The model for all update steps if voxels are
			///		overwritten.
			void Set(int _currentSize, const Math::IVec3& _position, int _size, T _type, SparseVoxelOctree* _parent);

			/// \brief Set all children to _defaultData to do an correct update and than
			///		delete the memory block.
			/// \param [in] _model The model for all update steps if voxels are
			///		overwritten.
			///	\param [in] _removePhysicaly Call the model update for each deleted voxel or not? 
			void RemoveSubTree(const Math::IVec4& _position, SparseVoxelOctree* _parent, bool _removePhysically);

			/// \brief Recursive pre-order traverse.
			/// \see SparseVoxelOctree::Traverse
			/// \param [in] _position Voxel position in 3D grid.
			/// \param [in] _size Grid size, 0 is the highest resolution.
			template<class Param>
			void Traverse( const Math::IVec4& _position, bool(*_callback)(const Math::IVec4&,T,Param*), Param* _param, T _defaultData );

			/// \brief Check if all children have the same type.
			bool IsUniform() const;

			/// \brief Computes which type has the majority for the 8 children.
			/// \details Nonsolids (tested with IsSolid()) are not counted.
			T MajorVoxelType();

			/// \brief Computes the child array index [0,7] assuming that the
			///		target position is within the current voxels children
			static int ComputeChildIndex(const Math::IVec3& _targetPosition, int _targetSize, int _childSize);

			/// \brief Temporarily method to benchmark the tree
#ifdef _DEBUG
			size_t MemoryConsumption() const;
#endif
		};
#		pragma pack(pop)

		/// \brief An own allocator for all nodes of this model's octree.
		/// \details Instead of single voxels this is always an array of 8.
		Jo::Memory::PoolAllocator m_SVONAllocator;

		/// \brief Use the pool allocator and call the constructor 8 times
		SVON* NewSVON();
		//void DeleteSVON(SVON* _node)	{ m_SVONAllocator.Free(_node); }

		/// \brief A sparse voxel octree with incremental differences between
		///		blueprint and this model.
		///	\details Each pointer points to a set of 8 children. So on root level there
		///		are always 8 nodes.
		SVON* m_roots;			
		Math::IVec3 m_rootPosition;	///< Position of the root node in grid space of the m_rootSize+1 level.
		int m_rootSize;				///< Level of the 8 root nodes where 0 is the highest possible resolution.

	};






	// ********************************************************************* //
	static const Math::IVec4 CHILD_OFFSETS[8] = { Math::IVec4(0,0,0,-1), Math::IVec4(1,0,0,-1), Math::IVec4(0,1,0,-1), Math::IVec4(1,1,0,-1),
		Math::IVec4(0,0,1,-1), Math::IVec4(1,0,1,-1), Math::IVec4(0,1,1,-1), Math::IVec4(1,1,1,-1) };

	// ********************************************************************* //
	template<typename T, typename Listener>
	SparseVoxelOctree<T,Listener>::SparseVoxelOctree( T _defaultData, Listener* _listener ) :
		m_defaultData(_defaultData),
		m_listener(_listener),
		m_SVONAllocator(sizeof(SVON)*8),
		m_roots(nullptr),
		m_rootSize(-1),
		m_rootPosition(0)
	{
	}

	// ********************************************************************* //
	template<typename T, typename Listener>
	typename SparseVoxelOctree<T,Listener>::SVON* SparseVoxelOctree<T,Listener>::NewSVON()
	{
		SVON* pNew = (SVON*)m_SVONAllocator.Alloc();
		for(int i=0; i<8; ++i)
			new (&pNew[i]) SVON(m_defaultData);
		return pNew;
	}

	// ********************************************************************* //
	template<typename T, typename Listener>
	void SparseVoxelOctree<T,Listener>::Set( const Math::IVec3& _position, int _level, T _type )
	{
		// Compute if the position is inside the current tree
		if(m_rootSize == -1)
		{
			// First set at all
			m_rootSize = _level;
			m_roots = NewSVON();
			m_rootPosition = _position >> 1;
		}
		int scale = m_rootSize-_level;
		Math::IVec3 position = _position>>scale;
		while( (scale < 0)
			|| (((position-m_rootPosition*2) & Math::IVec3(0xfffffffe)) != Math::IVec3(0)) )
		{
			// scale < 0: Obviously the target voxel is larger than the
			//	current root -> create new larger root(set)
			// pos: The new voxel could be outside -> requires larger root
			//	too because the tree must cover a larger area.
			SVON* pNew = NewSVON();
			int rootIndex = (m_rootPosition[0] & 1) + (m_rootPosition[1] & 1) * 2 + (m_rootPosition[2] & 1) * 4;
			pNew[rootIndex].children = m_roots;
			pNew[rootIndex].voxel = pNew[rootIndex].MajorVoxelType();
			m_rootPosition >>= 1;
			m_roots = pNew;
			++m_rootSize;
			// Still to small?
			++scale;
			position >>= 1;
		}

		// One of the eight children must contain the target position.
		// Most things from ComputeChildIndex are already computed
		// -> use last line inline.
		m_roots[ (position[0] & 1) + (position[1] & 1) * 2 + (position[2] & 1) * 4 ]
			.Set(m_rootSize, _position, _level, _type, this);
	}

	// ********************************************************************* //
	template<typename T, typename Listener>
	T SparseVoxelOctree<T,Listener>::Get( const Math::IVec3& _position, int _level ) const
	{
		// Get on an empty model? Would be better if this never happens ->
		// better performance because no 'if' on m_rootSize required.
		assert(m_rootSize != -1);

		// Special cases: not inside octree
		int scale = m_rootSize-_level;
		if( scale < 0 ) return m_defaultData;
		Math::IVec3 position = _position >> scale;
		if( ((position-m_rootPosition*2) & Math::IVec3(0xfffffffe)) != Math::IVec3(0) ) return m_defaultData;

		// Search in the octree (while not on target level or tree ends)
		SVON* current = &m_roots[ (position[0] & 1) + (position[1] & 1) * 2 + (position[2] & 1) * 4 ];
		while( (scale > 0) && current->children ) {
			assert( IsSolid( current->voxel ) );
			--scale;
			position = _position >> scale;
			current = &current->children[ (position[0] & 1) + (position[1] & 1) * 2 + (position[2] & 1) * 4 ];
		}

		return current->voxel;
	}

	// ********************************************************************* //
	template<typename T, typename Listener> template<typename Param>
	void SparseVoxelOctree<T,Listener>::Traverse( bool(*_callback)(const Math::IVec4&,T,Param*), Param* _param )
	{
		if( m_roots )
			for( int i=0; i<8; ++i )
			{
				if( m_roots[i].voxel != m_defaultData )
					m_roots[i].Traverse(IVec4(m_rootPosition, m_rootSize+1)+CHILD_OFFSETS[i], _callback, _param, m_defaultData);
				else
					assert( !m_roots[i].children );
			}
	}

	// ********************************************************************* //
#ifdef _DEBUG
	template<typename T, typename Listener>
	size_t SparseVoxelOctree<T,Listener>::MemoryConsumption() const
	{
		size_t sum = 0;
		for( int i=0; i<8; ++i )
			sum += m_roots[i].MemoryConsumption();
		return sum;
	}
#endif



	// ********************************************************************* //
	template<typename T, typename Listener>
	void SparseVoxelOctree<T,Listener>::SVON::Set(int _currentSize, const Math::IVec3& _position, int _size, T _type, SparseVoxelOctree* _parent)
	{
		if( _currentSize == _size )
		{
			// This is the target voxel. Delete everything below.
			if( children ) RemoveSubTree(IVec4(_position, _size), _parent, true);
			// Physical update of this voxel
			_parent->m_listener->Update(IVec4(_position, _size), voxel, _type);
			voxel = _type;
		} else {
			// Go into recursion
			int childIndex = ComputeChildIndex(_position, _size, _currentSize-1);
			if( !children )
			{ // Create new children
				children = _parent->NewSVON();
				// Set all children to the same type as this node.
				// It was uniform before!
				for(int i=0; i<8; ++i) children[i].voxel = voxel;
			}
			children[childIndex].Set(_currentSize-1, _position, _size, _type, _parent);
			// It could be that the children were deleted or are of the same
			// type now.
			if( IsUniform() )
			{
				// To do the model update the current position must be computed.
				int scale = _currentSize-_size;
				Math::IVec3 position = _position>>scale;
				// A uniform node does not need its children.
				RemoveSubTree(IVec4(position, _currentSize), _parent, false);
				//_parent->m_listener->Update(position, _currentSize, type, _type);
				voxel = _type;
			} else {
				// Update the current type based one the (maybe) changed child types.
				voxel = MajorVoxelType();
			}
		}
	}


	// ********************************************************************* //
	template<typename T, typename Listener>
	void SparseVoxelOctree<T,Listener>::SVON::RemoveSubTree(const Math::IVec4& _position, SparseVoxelOctree* _parent, bool _removePhysically)
	{
		assert( _position[3] >= 0 );
		assert( children );
		for(int i=0; i<8; ++i)
		{
			// Recursive
			if( children[i].children )
				children[i].RemoveSubTree(_position+CHILD_OFFSETS[i], _parent, _removePhysically);
			else if(_removePhysically)
				_parent->m_listener->Update(_position+CHILD_OFFSETS[i], voxel, _parent->m_defaultData);
		}
		// Delete
		_parent->m_SVONAllocator.Free(children);
		children = nullptr;
	}

	// ********************************************************************* //
	template<typename T, typename Listener> template<class Param>
	void SparseVoxelOctree<T,Listener>::SVON::Traverse( const Math::IVec4& _position, bool(*_callback)(const Math::IVec4&,T,Param*), Param* _param, T _defaultData )
	{
		if( _callback(_position,voxel,_param) && children )
		{
			Math::IVec4 position(_position[0]<<1, _position[1]<<1, _position[2]<<1, _position[3]);
			for( int i=0; i<8; ++i )
			{
				if( children[i].voxel != _defaultData )
					children[i].Traverse(position + CHILD_OFFSETS[i], _callback, _param, _defaultData);
				else
					assert( !children[i].children );
			}
		}
	}

	// ********************************************************************* //
	template<typename T, typename Listener>
	bool SparseVoxelOctree<T,Listener>::SVON::IsUniform() const
	{
		if( !children ) return true;	// Uniform the same as the parent.
		if( children[0].children ) return false;	// If a child has children the uniformness is undefined
		for( int i=1; i<8; ++i )
			if( children[i].voxel != children[0].voxel || children[i].children ) return false;
		return true;
	}

	// ********************************************************************* //
	template<typename T, typename Listener>
	T SparseVoxelOctree<T,Listener>::SVON::MajorVoxelType()
	{
		assert( children );
		T types[8];
		// Enumerate and sort all types of the child voxels
		int num = 0;
		for(int i=0; i<8; ++i) if( IsSolid(children[i].voxel) )
			types[num++] = children[i].voxel;
		assert( num > 0 );
		Algo::SmallSort(types, num);
		// Simple iterate over the type array and count which type has the
		// most elements.
		int count0 = 0;		// Previous major
		int count1 = 1;		// Current element counter
		T t = types[0];
		for(int i=1; i<num; ++i )
		{
			if( types[i] == types[i-1] ) ++count1;
			else {
				if( count1 > count0 ) {t=types[i-1]; count0 = count1;}
				count1 = 1;
			}
		}
		return t;
	}

	// ********************************************************************* //
	template<typename T, typename Listener>
	int SparseVoxelOctree<T,Listener>::SVON::ComputeChildIndex(const Math::IVec3& _targetPosition, int _targetSize, int _childSize)
	{
		// Find out the correct position
		int scale = _childSize-_targetSize;
		Math::IVec3 position = _targetPosition>>scale;
		// Now compute the index from the position
		return (position[0] & 1) + (position[1] & 1) * 2 + (position[2] & 1) * 4;
	}


	// ********************************************************************* //
#ifdef _DEBUG
	template<typename T, typename Listener>
	size_t SparseVoxelOctree<T,Listener>::SVON::MemoryConsumption() const
	{
		size_t size = 0;	// Count this-size at parent
		if( children ) {
			size = 8*sizeof(SVON);	// The child array
			for( int i=0; i<8; ++i )
				size += children[i].MemoryConsumption();
		}
		return size;//*/
		// Theoretic model
		/*size_t size = sizeof(SVON)+1;
		if( children ) {
			for( int i=0; i<8; ++i )
				if( IsSolid(children[i].voxel) )
					size += children[i].MemoryConsumtion();
		}
		return size;//*/
	}
#endif
} // namespace Voxel