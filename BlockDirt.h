
#pragma once

#include "Block.h"


class BlockDirt : public Block
{
protected:

public:

	BlockDirt( const BlockInfo &block_info ) : Block( block_info )
	{}
	void render( Chunk *chunk, int x, int y, int z, int8_t direction, std::vector<Vertex> &vertex_buffer )
	{
		// This should handle rendering grass textures on top of
		// the dirt if it has grown grass (use the data member)
		// and also to modify the sides of the block. Only the
		// bottom of the block is the same for all states and
		// biomes.

		float left_u, top_v, right_u, bottom_v;

		// TODO: Set 'sunlight' for exposed blocks in generation
		bool air_above = (chunk->get( x, y + 1, z )==nullptr)?true:false;

		if ( air_above )
		{
			left_u = Renderer::blocks_texture->getAtlasU( this->info.ID + 1, 0 );
			right_u = Renderer::blocks_texture->getAtlasU( this->info.ID + 2, 0 );
			top_v = Renderer::blocks_texture->getAtlasV( this->info.ID + 1, 0 );
			bottom_v = Renderer::blocks_texture->getAtlasV( this->info.ID + 1, 1 );
		}
		else
		{
			left_u = Renderer::blocks_texture->getAtlasU( this->info.ID + 0, 0 );
			right_u = Renderer::blocks_texture->getAtlasU( this->info.ID + 1, 0 );
			top_v = Renderer::blocks_texture->getAtlasV( this->info.ID + 0, 0 );
			bottom_v = Renderer::blocks_texture->getAtlasV( this->info.ID + 0, 1 );
		}

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
			float left_u2, top_v2, right_u2, bottom_v2;

			left_u2 = Renderer::blocks_texture->getAtlasU( this->info.ID, 0 );
			right_u2 = Renderer::blocks_texture->getAtlasU( this->info.ID + 1, 0 );
			top_v2 = Renderer::blocks_texture->getAtlasV( this->info.ID, 0 );
			bottom_v2 = Renderer::blocks_texture->getAtlasV( this->info.ID, 1 );

			vertex_buffer.push_back( Vertex( x, y, z, 0, 0, 1, 0, left_u2, top_v2 ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, 0, 1, 0, right_u2, top_v2 ) );
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 0, 1, 0, left_u2, bottom_v2 ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z, 0, 0, 1, 0, right_u2, top_v2 ) );
			vertex_buffer.push_back( Vertex( x + 1, y, z + 1, 0, 0, 1, 0, right_u2, bottom_v2 ) );
			vertex_buffer.push_back( Vertex( x, y, z + 1, 0, 0, 1, 0, left_u2, bottom_v2 ) );
		}
		else if ( direction == 2 ) // Positive Y direction
		{
			float left_u2, top_v2, right_u2, bottom_v2;

			if ( air_above )
			{
				left_u2 = Renderer::blocks_texture->getAtlasU( this->info.ID + 2, 0 );
				right_u2 = Renderer::blocks_texture->getAtlasU( this->info.ID + 3, 0 );
				top_v2 = Renderer::blocks_texture->getAtlasV( this->info.ID + 2, 0 );
				bottom_v2 = Renderer::blocks_texture->getAtlasV( this->info.ID + 2, 1 );
			}
			else
			{
				left_u2 = Renderer::blocks_texture->getAtlasU( this->info.ID, 0 );
				right_u2 = Renderer::blocks_texture->getAtlasU( this->info.ID + 1, 0 );
				top_v2 = Renderer::blocks_texture->getAtlasV( this->info.ID, 0 );
				bottom_v2 = Renderer::blocks_texture->getAtlasV( this->info.ID, 1 );
			}

			vertex_buffer.push_back( Vertex( x, y + 1, z, 0, 0, -1, 0, left_u2, top_v2 ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z + 1, 0, 0, -1, 0, left_u2, bottom_v2 ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z, 0, 0, -1, 0, right_u2, top_v2 ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z, 0, 0, -1, 0, right_u2, top_v2 ) );
			vertex_buffer.push_back( Vertex( x, y + 1, z + 1, 0, 0, -1, 0, left_u2, bottom_v2 ) );
			vertex_buffer.push_back( Vertex( x + 1, y + 1, z + 1, 0, 0, -1, 0, right_u2, bottom_v2 ) );
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
};
