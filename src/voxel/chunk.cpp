#include "chunk.hpp"
#include "../graphic/device.hpp"
#include "../graphic/uniformbuffer.hpp"
#include <cstdlib>
#include <cstring>

using namespace Math;

namespace Voxel {

	/// \brief The level offsets in number of voxels to the start of the octree
	const int LEVEL_OFFSETS[] = {0,1,9,73,585,4681};
	const int NUM_OCTREE_NODES = 37449;

#	define INDEX(P, L)		(LEVEL_OFFSETS[L] + (P.x) + (1<<(L))*((P.y) + (1<<(L))*(P.z)))
#	define INDEX2(X,Y,Z, L)	(LEVEL_OFFSETS[L] + (X) + (1<<(L))*((Y) + (1<<(L))*(Z)))


	Chunk::Chunk() :
		m_voxels( NUM_OCTREE_NODES, "u", Graphic::VertexBuffer::PrimitiveType::POINT )
	{
		m_octree = (OctreeNode*)malloc( sizeof(OctreeNode) * NUM_OCTREE_NODES );
		memset( m_octree, 0, sizeof(OctreeNode) * NUM_OCTREE_NODES );
	}


	Chunk::~Chunk()
	{
		free(m_octree);
		m_octree = nullptr;
	}


	void Chunk::Set( const Math::IVec3& _position, int _level, VoxelType _type )
	{
		int levelSize = 1<<_level;
		assert(_position.x >= 0 && _position.x < levelSize);
		assert(_position.y >= 0 && _position.y < levelSize);
		assert(_position.z >= 0 && _position.z < levelSize);

		// Trivialy set
		int index = INDEX( _position, _level);
		m_octree[index].type = _type;
		// A set always forces all children to the same type -> uniform
		m_octree[index].flags = 0x02 | (_type==VoxelType::NONE ? 0 : 0x01);

		// Fill children with the same type
		if( _level < 5 )
		{
			int baseX = _position.x * 2;
			int baseY = _position.y * 2;
			int baseZ = _position.z * 2;
			Set( IVec3( baseX,   baseY,   baseZ   ), _level+1, _type );
			Set( IVec3( baseX+1, baseY,   baseZ   ), _level+1, _type );
			Set( IVec3( baseX,   baseY+1, baseZ   ), _level+1, _type );
			Set( IVec3( baseX+1, baseY+1, baseZ   ), _level+1, _type );
			Set( IVec3( baseX,   baseY,   baseZ+1 ), _level+1, _type );
			Set( IVec3( baseX+1, baseY,   baseZ+1 ), _level+1, _type );
			Set( IVec3( baseX,   baseY+1, baseZ+1 ), _level+1, _type );
			Set( IVec3( baseX+1, baseY+1, baseZ+1 ), _level+1, _type );
		}

		// Update parents
		int baseX = _position.x & 0xfffffffe;
		int baseY = _position.y & 0xfffffffe;
		int baseZ = _position.z & 0xfffffffe;
		while( _level > 0 )
		{
			// Count different types in the neighbourhood and take majority.
			bool solid = true;		// False if at least one node is of type NONE
			bool uniformChildren = true;
			OctreeNode pTypes[] = {
				m_octree[INDEX2( baseX,   baseY,   baseZ,   _level)],
				m_octree[INDEX2( baseX+1, baseY,   baseZ,   _level)],
				m_octree[INDEX2( baseX,   baseY+1, baseZ,   _level)],
				m_octree[INDEX2( baseX+1, baseY+1, baseZ,   _level)],
				m_octree[INDEX2( baseX,   baseY,   baseZ+1, _level)],
				m_octree[INDEX2( baseX+1, baseY,   baseZ+1, _level)],
				m_octree[INDEX2( baseX,   baseY+1, baseZ+1, _level)],
				m_octree[INDEX2( baseX+1, baseY+1, baseZ+1, _level)] };
			// Find majority element
			int iMajIdx=0, iMajCount=1;
			solid &= pTypes[0].type != VoxelType::NONE;
			uniformChildren &= pTypes[0].IsUniform();
			for( int i=1; i<8; ++i )
			{
				solid &= pTypes[i].type != VoxelType::NONE;
				uniformChildren &= pTypes[i].IsUniform();
				// Moore's Voting algorithm part 1
				if( pTypes[i].type == pTypes[iMajIdx].type ) ++iMajCount;
				else --iMajCount;
				if( !iMajCount ) {iMajCount=1; iMajIdx = i;}
			}
			// Passed all tests set parent and go upward
			baseX >>= 1;
			baseY >>= 1;
			baseZ >>= 1;
			--_level;
			index = INDEX2(baseX, baseY, baseZ, _level);
			m_octree[index].type = pTypes[iMajIdx].type;
			m_octree[index].flags = (solid ? 1 : 0) | ((uniformChildren && (iMajCount==8)) ? 2 : 0);
			baseX &= 0xfffffffe;
			baseY &= 0xfffffffe;
			baseZ &= 0xfffffffe;
		}
	}

	Chunk::OctreeNode Chunk::Get( const Math::IVec3& _position, int _level )
	{
		int levelSize = 1<<_level;
		if(_position.x < 0 || _position.x >= levelSize) return OctreeNode();
		if(_position.y < 0 || _position.y >= levelSize) return OctreeNode();
		if(_position.z < 0 || _position.z >= levelSize) return OctreeNode();

		return m_octree[LEVEL_OFFSETS[_level] + _position.x + levelSize*(_position.y + levelSize*_position.z)];
	}


	void Chunk::ComputeVertexBuffer()
	{
		// Currently CPU method: recursively traverse the octree and add
		// largest voxel if visible.
		m_voxels.Clear();
		FillVBRecursive( Math::IVec3(0,0,0), 0 );
		m_voxels.Commit();
	}


	void Chunk::FillVBRecursive( const Math::IVec3& _position, int _level )
	{
		OctreeNode T = Get( _position, _level );
		// Stop at empty voxels
		if( T.GetType()==VoxelType::NONE && T.IsUniform() ) return;

		// Recursion required?
		// If all children have the same type stop recursion.
		bool hasChildren = !T.IsUniform() && (_level < 5);

		if( T.GetType()!=VoxelType::NONE && !hasChildren )
		{
			// Add the current node (at least it is added for the LOD)
			VoxelVertex V;
			V.SetVisibility((Get( Math::IVec3(_position.x-1, _position.y, _position.z), _level )).IsSolid() ? 0 : 1 ,
							(Get( Math::IVec3(_position.x+1, _position.y, _position.z), _level )).IsSolid() ? 0 : 1 ,
							(Get( Math::IVec3(_position.x, _position.y-1, _position.z), _level )).IsSolid() ? 0 : 1 ,
							(Get( Math::IVec3(_position.x, _position.y+1, _position.z), _level )).IsSolid() ? 0 : 1 ,
							(Get( Math::IVec3(_position.x, _position.y, _position.z-1), _level )).IsSolid() ? 0 : 1 ,
							(Get( Math::IVec3(_position.x, _position.y, _position.z+1), _level )).IsSolid() ? 0 : 1 );
			if( V.IsVisible() )
			{
				V.SetPosition( _position );
				V.SetSize( 5-_level );
				V.SetTexture( int(T.GetType()) );
				m_voxels.Add( &V );
			}
		}

		if( hasChildren )
		{
			FillVBRecursive( _position * 2 + Math::IVec3(0,0,0), _level+1 );
			FillVBRecursive( _position * 2 + Math::IVec3(1,0,0), _level+1 );
			FillVBRecursive( _position * 2 + Math::IVec3(0,1,0), _level+1 );
			FillVBRecursive( _position * 2 + Math::IVec3(1,1,0), _level+1 );
			FillVBRecursive( _position * 2 + Math::IVec3(0,0,1), _level+1 );
			FillVBRecursive( _position * 2 + Math::IVec3(1,0,1), _level+1 );
			FillVBRecursive( _position * 2 + Math::IVec3(0,1,1), _level+1 );
			FillVBRecursive( _position * 2 + Math::IVec3(1,1,1), _level+1 );
		}
	}

	void Chunk::Draw( Graphic::UniformBuffer& _objectConstants, const Math::Matrix& _viewProjection )
	{
		// TODO: culling & LOD

		// -16 is the translation to center the chunks
		// TODO: mass mid point (Schwerpunkt)
		_objectConstants["WorldViewProjection"] = MatrixTranslation(m_position) * _viewProjection;

		_objectConstants["Corner000"] = Vec4( -0.5f, -0.5f, -0.5f, 0.0f ) * _viewProjection;
		_objectConstants["Corner001"] = Vec4( -0.5f, -0.5f,  0.5f, 0.0f ) * _viewProjection;
		_objectConstants["Corner010"] = Vec4( -0.5f,  0.5f, -0.5f, 0.0f ) * _viewProjection;
		_objectConstants["Corner011"] = Vec4( -0.5f,  0.5f,  0.5f, 0.0f ) * _viewProjection;
		_objectConstants["Corner100"] = Vec4(  0.5f, -0.5f, -0.5f, 0.0f ) * _viewProjection;
		_objectConstants["Corner101"] = Vec4(  0.5f, -0.5f,  0.5f, 0.0f ) * _viewProjection;
		_objectConstants["Corner110"] = Vec4(  0.5f,  0.5f, -0.5f, 0.0f ) * _viewProjection;
		_objectConstants["Corner111"] = Vec4(  0.5f,  0.5f,  0.5f, 0.0f ) * _viewProjection;

		// TODO: auto commit
		_objectConstants.Commit();
		Graphic::Device::DrawVertices( m_voxels, 0, m_voxels.GetNumVertices() );
	}
};