
#pragma once

#include "BlockInfo.h"
#include "Chunk.h"
#include "Renderer.h"

#include <vector>

#pragma warning( disable : 4244 )


class Block
{
protected:
public:

	BlockInfo			&info;
	uint8_t				data_value;

	Block( BlockInfo &block_info ) :
		info( block_info )
	{}
	virtual ~Block()
	{}

	Block & operator= (const Block &b)
	{
		this->info = b.info;
		this->data_value = b.data_value;
		return *this;
	}

	/** @fn render()
	 ** @todo Implement custom render code for BlockDirt
	 **
	 ** Custom rendering method for the block, each derived class of Block can override this
	 **/
	virtual void render( Chunk *chunk, int x, int y, int z, int8_t direction, std::vector<Vertex> &vertex_buffer )
	{
		float left_u, top_v, right_u, bottom_v;

		left_u = Renderer::blocks_texture->getAtlasU( this->info.ID, 0 );
		right_u = Renderer::blocks_texture->getAtlasU( this->info.ID + 1, 0 );
		top_v = Renderer::blocks_texture->getAtlasV( this->info.ID, 0 );
		bottom_v = Renderer::blocks_texture->getAtlasV( this->info.ID, 1 );

		if ( direction == -1 ) // Negative X direction
		{
			vertex_buffer.push_back( Vertex( x, y, z, 0, 1, 0, 0, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 1, 0, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z, 0, 1, 0, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z, 0, 1, 0, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 1, 0, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z + 1, 0, 1, 0, 0, right_u, bottom_v ) );
		}
		else if ( direction == 1 ) // Positive X direction
		{
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, -1, 0, 0, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z, 0, -1, 0, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z + 1, 0, -1, 0, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z, 0, -1, 0, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z + 1, 0, -1, 0, 0, right_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z + 1, 0, -1, 0, 0, right_u, top_v ) );
		}
		else if ( direction == -2 ) // Negative Y direction
		{
			vertex_buffer.push_back( Vertex( x, y, z, 0, 0, 1, 0, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, 0, 1, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 0, 1, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, 0, 1, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z + 1, 0, 0, 1, 0, right_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 0, 1, 0, left_u, bottom_v ) );
		}
		else if ( direction == 2 ) // Positive Y direction
		{
			vertex_buffer.push_back( Vertex( x, y + 1, z, 0, 0, -1, 0, left_u, top_v  ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z + 1, 0, 0, -1, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z, 0, 0, -1, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z, 0, 0, -1, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z + 1, 0, 0, -1, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z + 1, 0, 0, -1, 0, right_u, bottom_v ) );
		}
		else if ( direction == -3 ) // Negative Z direction
		{
			vertex_buffer.push_back( Vertex( x, y, z, 0, 0, 0, 1, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z, 0, 0, 0, 1, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, 0, 0, 1, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z, 0, 0, 0, 1, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z, 0, 0, 0, 1, right_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, 0, 0, 1, right_u, top_v ) );
		}
		else if ( direction == 3 ) // Positive Z direction
		{
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 0, 0, -1, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z + 1, 0, 0, 0, -1, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z + 1, 0, 0, 0, -1, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z + 1, 0, 0, 0, -1, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z + 1, 0, 0, 0, -1, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z + 1, 0, 0, 0, -1, right_u, bottom_v ) );
		}
	}

	virtual void update()
	{
	}

	// -- Compare the types of blocks
	bool isSameBlock( Block *block ) const
	{
		if ( block == nullptr )
			return false;
		if ( this->info == block->info )
			return true;
		else
			return false;
	}

};
