
#pragma once

#include "Block.h"
#include "ResourceManager.h"


class BlockSaltWater : public Block
{
protected:

public:

	BlockSaltWater( const BlockInfo &block_info ) : Block( block_info )
	{}

	void render( Chunk *chunk, int x, int y, int z, int8_t direction, std::vector<Vertex> &vertex_buffer )
	{
		Block *b = chunk->get( x,y+1,z );
		if ( b != nullptr && b->info == ResourceManager::iResourceManager->getBlockInfo( "salt_water" ) )
		{
			Block::render( chunk, x, y, z, direction, vertex_buffer );
			return;
		}

		float left_u, top_v, right_u, bottom_v;
		float top = float(y+1) - (2.0f / 16.0f);

		left_u = Renderer::blocks_texture->getAtlasU( this->info.ID, 0 );
		right_u = Renderer::blocks_texture->getAtlasU( this->info.ID + 1, 0 );
		top_v = Renderer::blocks_texture->getAtlasV( this->info.ID, 0 );
		bottom_v = Renderer::blocks_texture->getAtlasV( this->info.ID, 1 );

		if ( direction == -1 ) // Negative X direction
		{
			vertex_buffer.push_back( Vertex( x, y, z, 0, 1, 0, 0, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 1, 0, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, top, z, 0, 1, 0, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x, top, z, 0, 1, 0, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 1, 0, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, top, z + 1, 0, 1, 0, 0, right_u, bottom_v ) );
		}
		else if ( direction == 1 ) // Positive X direction
		{
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, -1, 0, 0, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, top, z, 0, -1, 0, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z + 1, 0, -1, 0, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, top, z, 0, -1, 0, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, top, z + 1, 0, -1, 0, 0, right_u, bottom_v ) );
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
			vertex_buffer.push_back( Vertex( x, top, z, 1, 0, -1, 0, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, top, z + 1, 1, 0, -1, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, top, z, 1, 0, -1, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, top, z, 1, 0, -1, 0, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, top, z + 1, 1, 0, -1, 0, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, top, z + 1, 1, 0, -1, 0, right_u, bottom_v ) );
		}
		else if ( direction == -3 ) // Negative Z direction
		{
			vertex_buffer.push_back( Vertex( x, y, z, 0, 0, 0, 1, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, top, z, 0, 0, 0, 1, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, 0, 0, 1, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, top, z, 0, 0, 0, 1, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, top, z, 0, 0, 0, 1, right_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, 0, 0, 1, right_u, top_v ) );
		}
		else if ( direction == 3 ) // Positive Z direction
		{
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 0, 0, -1, left_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z + 1, 0, 0, 0, -1, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x, top, z + 1, 0, 0, 0, -1, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x, top, z + 1, 0, 0, 0, -1, left_u, bottom_v ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z + 1, 0, 0, 0, -1, right_u, top_v ) );
			vertex_buffer.push_back( Vertex( x + 1, top, z + 1, 0, 0, 0, -1, right_u, bottom_v ) );
		}
	}
};
