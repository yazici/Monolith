#include "vertexbuffer.hpp"
#include "math/vector.hpp"
#include "opengl.hpp"
#include <cstdlib>
#include <cstring>
#include "utilities/assert.hpp"
#include <cstdint>

namespace Graphic {
// ************************************************************************* //
// VERTEX BUFFER
// ************************************************************************* //
// Helper function to work with the format specification
inline int VectorSize(char a)
{
	switch(a)
	{
	case '4': return 4;
	case '3': case 'p': case 'n': case 't': return 3;
	case '2': return 2;
	case '1': case 'c': case 'u': return 1;
	}
	return 0;
}

// Creating a vertex buffer with arbitrary interleaved data
void VertexBuffer::CreateVAO(const char* _vertexDeclaration)
{
	// First step analyzing the string
		// Counting VBO's size and string length
		int len = 0;
		m_vertexSize = 0;
		while(_vertexDeclaration[len])
		{
			if(_vertexDeclaration[len] == 'n') m_normalOffset = m_vertexSize;
			else if(_vertexDeclaration[len] == 'p') m_positionOffset = m_vertexSize;
			else if(_vertexDeclaration[len] == 't') m_tangentOffset = m_vertexSize;
			m_vertexSize += VectorSize(_vertexDeclaration[len]) * 4;
			++len;
		}

	// Second step create VAO and VBOs
	GL_CALL(glGenVertexArrays, 1, &m_VAO);
	GL_CALL(glBindVertexArray, m_VAO);
		// Create Vertex buffer object
		GL_CALL(glGenBuffers, 1, &m_VBO);
		GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_VBO);	// Bind VBO to VAO
		int i=0;
		int stride = 0, bindOff = 0, colorBindOff = 0, uintBindOff = 0;
		while(_vertexDeclaration[i])
		{
			int location;
			switch(_vertexDeclaration[i])
			{
			case 'p': location = int(BindingLocation::POSITION);
				GL_CALL(glVertexAttribPointer, location, 3, GL_FLOAT, GL_FALSE, m_vertexSize, (GLvoid*)(stride));
				GL_CALL(glEnableVertexAttribArray, location);
				stride += 3*sizeof(float);
				break;
			case 'n': location = int(BindingLocation::NORMAL);
				GL_CALL(glVertexAttribPointer, location, 3, GL_FLOAT, GL_FALSE, m_vertexSize, (GLvoid*)(stride));
				GL_CALL(glEnableVertexAttribArray, location);
				stride += 3*sizeof(float);
				break;
			case 't': location = int(BindingLocation::TANGENT);
				GL_CALL(glVertexAttribPointer, location, 3, GL_FLOAT, GL_FALSE, m_vertexSize, (GLvoid*)(stride));
				GL_CALL(glEnableVertexAttribArray, location);
				stride += 3*sizeof(float);
				break;
			case '1': location = int(BindingLocation::VEC) + bindOff;
				GL_CALL(glVertexAttribPointer, location, 1, GL_FLOAT, GL_FALSE, m_vertexSize, (GLvoid*)(stride));
				GL_CALL(glEnableVertexAttribArray, location);
				stride += sizeof(float);
				++bindOff;
				break;
			case '2': location = int(BindingLocation::VEC) + bindOff;
				GL_CALL(glVertexAttribPointer, location, 2, GL_FLOAT, GL_FALSE, m_vertexSize, (GLvoid*)(stride));
				GL_CALL(glEnableVertexAttribArray, location);
				stride += 2*sizeof(float);
				++bindOff;
				break;
			case '3': location = int(BindingLocation::VEC) + bindOff;
				GL_CALL(glVertexAttribPointer, location, 3, GL_FLOAT, GL_FALSE, m_vertexSize, (GLvoid*)(stride));
				GL_CALL(glEnableVertexAttribArray, location);
				stride += 3*sizeof(float);
				++bindOff;
				break;
			case '4': location = int(BindingLocation::VEC) + bindOff;
				GL_CALL(glVertexAttribPointer, location, 4, GL_FLOAT, GL_FALSE, m_vertexSize, (GLvoid*)(stride));
				GL_CALL(glEnableVertexAttribArray, location);
				stride += 4*sizeof(float);
				++bindOff;
				break;
			case 'c':
				if(colorBindOff>=3) LOG_ERROR("Too many 'c' in vertex declaration. The maximum number of color binding points is 4!");
				location = int(BindingLocation::COLOR0) + colorBindOff;
				GL_CALL(glVertexAttribPointer, location, 4, GL_UNSIGNED_BYTE, GL_TRUE, m_vertexSize, (GLvoid*)(stride));
				GL_CALL(glEnableVertexAttribArray, location);
				stride += 4;
				++colorBindOff;
				break;
			case 'u':
				if(uintBindOff>=3) LOG_ERROR("Too many 'u' in vertex declaration. The maximum number of uint binding points is 4!");
				location = int(BindingLocation::UINT0) + uintBindOff;
				GL_CALL(glVertexAttribIPointer, location, 1, GL_UNSIGNED_INT, m_vertexSize, (GLvoid*)(stride));
				GL_CALL(glEnableVertexAttribArray, location);
				stride += 4;
				++uintBindOff;
				break;
			}
			++i;
		}

	GL_CALL(glBindVertexArray, 0);
	GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
}

// ************************************************************************* //
// Creating a static vertex buffer with arbitrary interleaved data.
VertexBuffer::VertexBuffer( const char* _vertexDeclaration, void* _data, int _size, PrimitiveType _type ) :
	m_data(nullptr),
	m_firstDirtyIndex(std::numeric_limits<int>::max()),
	m_lastDirtyIndex(-1),
	m_positionOffset(0xffffffff),
	m_normalOffset(0xffffffff),
	m_tangentOffset(0xffffffff),
	m_primitiveType(_type),
	m_isStatic(true),
	m_state(State::INITIALIZING)
{
	CreateVAO(_vertexDeclaration);

	// Upload the data
	if(_data) {
		GL_CALL(glBindBuffer,  GL_ARRAY_BUFFER, m_VBO );
		GL_CALL(glBufferData, GL_ARRAY_BUFFER, _size, _data, GL_STATIC_DRAW);
		GL_CALL(glBindBuffer,  GL_ARRAY_BUFFER, 0 );

		m_firstDirtyIndex = std::numeric_limits<int>::max();
		m_lastDirtyIndex = -1;
		m_cursor = m_maxNumVertices = _size / m_vertexSize;

		// Resolve thread-waiting
		m_state = State::WORKING;
	} else m_maxNumVertices = m_cursor = 0;
}

// ************************************************************************* //
// Creating a dynamic vertex buffer with arbitrary interleaved data.
VertexBuffer::VertexBuffer( const char* _vertexDeclaration, PrimitiveType _type ) :
	m_data(nullptr),
	m_firstDirtyIndex(std::numeric_limits<int>::max()),
	m_lastDirtyIndex(-1),
	m_cursor(0),
	m_maxNumVertices(64),
	m_positionOffset(0xffffffff),
	m_normalOffset(0xffffffff),
	m_tangentOffset(0xffffffff),
	m_primitiveType(_type),
	m_isStatic(false),
	m_state(State::INITIALIZING)
{
	CreateVAO(_vertexDeclaration);

	// Create the data on GPU side
	GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_VBO);
	GL_CALL(glBufferData, GL_ARRAY_BUFFER, m_vertexSize * m_maxNumVertices, nullptr, GL_DYNAMIC_DRAW);
	GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
	// Data on CPU side
	m_data = (uint8_t*)malloc( m_vertexSize * m_maxNumVertices );
}

// ************************************************************************* //
VertexBuffer::VertexBuffer( VertexBuffer&& _buffer ) :
	m_cursor( _buffer.m_cursor ),
	m_data( _buffer.m_data ),
	m_firstDirtyIndex( _buffer.m_firstDirtyIndex ),
	m_lastDirtyIndex( _buffer.m_lastDirtyIndex ),
	m_maxNumVertices( _buffer.m_maxNumVertices ),
	m_normalOffset( _buffer.m_normalOffset ),
	m_positionOffset( _buffer.m_positionOffset ),
	m_primitiveType( _buffer.m_primitiveType ),
	m_tangentOffset( _buffer.m_tangentOffset ),
	m_VAO( _buffer.m_VAO ),
	m_VBO( _buffer.m_VBO ),
	m_vertexSize( _buffer.m_vertexSize )
{
	_buffer.m_VAO = 0;
	_buffer.m_VBO = 0;
	_buffer.m_data = nullptr;
}

// ************************************************************************* //
VertexBuffer::~VertexBuffer()
{
	if( m_VAO )
	{
		// Detach and delete Vertex buffer
		GL_CALL(glBindVertexArray, m_VAO);
		//glDisableVertexAttribArray(0);
		GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
		GL_CALL(glDeleteBuffers, 1, &m_VBO);
		// Detach and delete array
		GL_CALL(glBindVertexArray, 0);
		GL_CALL(glDeleteVertexArrays, 1, &m_VAO);
	}

	free(m_data);
	m_data = nullptr;
}


// ******************************************************************************** //
void VertexBuffer::Resize(unsigned _numVertices)
{
	// Private method should never be called for static buffers!
	Assert(!IsStatic(), "Static vertex buffers can not be resized!");

	m_maxNumVertices = _numVertices;

	// Discard on GPU side
	GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_VBO);
	GL_CALL(glBufferData, GL_ARRAY_BUFFER, m_vertexSize * _numVertices, nullptr, GL_DYNAMIC_DRAW);
	GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
	m_data = (uint8_t*)realloc(m_data, _numVertices * m_vertexSize);
}


// ******************************************************************************** //
// Set the number of vertices back to 0.
void VertexBuffer::Clear()
{
	Assert(!IsStatic(), "Static vertex buffers can not be cleared!");
	StartWorking();
	m_cursor = 0;
	m_firstDirtyIndex = std::numeric_limits<int>::max();
	m_lastDirtyIndex = -1;
}

// ******************************************************************************** //
void VertexBuffer::StartWorking()
{
	// Waiting
	if(m_state == State::UPLOAD_READY)
	{
		std::unique_lock<std::mutex> lock(m_uploading);
		m_state = State::WORKING;
	}
	// Until the first SetDirty od Commit(2) call INITIALIZATION is not left
	Assert(m_state == State::WORKING || m_state == State::INITIALIZING,
		"Thread state not clear!");
}

// ******************************************************************************** //
void VertexBuffer::Commit()
{
	if(m_data && IsDirty())
	{
		std::unique_lock<std::mutex> lock(m_uploading);
		// Discard everything in a range where vertices are dirty
		GL_CALL(glBindBuffer,  GL_ARRAY_BUFFER, m_VBO );
		if( IsStatic() )
		{
			GL_CALL(glBufferData, GL_ARRAY_BUFFER, m_vertexSize * m_maxNumVertices, m_data, GL_STATIC_DRAW);
			free(m_data);
			m_data = nullptr;
		} else {
			GL_CALL(glBufferSubData, GL_ARRAY_BUFFER,
				m_firstDirtyIndex * m_vertexSize,
				(m_lastDirtyIndex - m_firstDirtyIndex + 1) * m_vertexSize,
				m_data );
		}
		GL_CALL(glBindBuffer,  GL_ARRAY_BUFFER, 0 );

		m_firstDirtyIndex = std::numeric_limits<int>::max();
		m_lastDirtyIndex = -1;
		//GL_CALL(glBindBuffer,  GL_ARRAY_BUFFER, 0 );

		// Resolve thread-waiting
		m_state = State::WORKING;
	}
}

void VertexBuffer::Commit(void*& _data, int _size)
{
	Assert(_data, "No data to commit!");

	StartWorking();

	// Take data for later commit
	free(m_data);
	m_data = (uint8_t*)_data;
	_data = nullptr;

	// Derive the statistic data
	m_cursor = m_maxNumVertices = _size / m_vertexSize;

	SetDirty();
}

// ******************************************************************************** //
void VertexBuffer::DeleteVertex(unsigned _index)
{
	Assert( m_state != State::UPLOAD_READY, "Call StartWork() to make sure the buffer is in a clean state before changing things!");
	Assert( m_cursor > 0, "Cursor is already at the buffer's beginning!");	// Number of vertices
	if(m_cursor <= _index) LOG_LVL2("Vertex cannot be deleted: Index too large.");
	--m_cursor;
	if(_index != m_cursor) {
		memcpy(m_data + m_vertexSize * _index, m_data + m_vertexSize * m_cursor, m_vertexSize );
	}
}

// ******************************************************************************** //
void VertexBuffer::FlipNormals()
{
	Assert( m_state != State::UPLOAD_READY, "Call StartWork() to make sure the buffer is in a clean state before changing things!");
	if(m_normalOffset == 0xffffffff) return;
	for(unsigned i=0; i<GetNumVertices(); ++i)
	{
		Math::Vec3* pN = (Math::Vec3*)(((uint8_t*)m_data)+i*m_vertexSize+m_normalOffset);
		(*pN) = -(*pN);
	}
}

// ******************************************************************************** //
void VertexBuffer::Bind() const
{
	// In case there is nothing to upload commit will do nothing
	const_cast<VertexBuffer*>(this)->Commit();

	GL_CALL(glBindVertexArray, m_VAO);
}



};