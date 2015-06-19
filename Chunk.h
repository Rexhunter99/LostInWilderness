#pragma once

#include <cstdint>
#include <ctime>
#include <fstream>

#define CHUNK_WIDTH		16
#define CHUNK_HEIGHT	256
#define CHUNK_LENGTH	16
// Temp:
#define SEALEVEL 48

class Block;


class Chunk
{
private:

	void init( bool manual_gen = false );

public:
	Block*					blk[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_LENGTH];
	Chunk					*left,
							*right,
							*below,
							*above,
							*front,
							*back;

	int64_t			ax,
					ay,
					az,
					elements,
					biome;
	uint32_t		vbo;
	time_t			time_last_saved,		// The last time the chunk was cached to disk
					time_last_updated,		// The last time the chunk was updated
					time_inhabited;			// The total cumulative time that players have spent in this chunk
	unsigned int	needs_update:1;			// [Flag] This chunk needs to be updated
	unsigned int	initialized:1;			// [Flag] This chunk has been initialized
	unsigned int	generated:1;			// [Flag] This chunk has been generated
	unsigned int	generating:1;			// [Flag] This chunk is generating (this is set to false whenever generated is set to true!)
	float			humidity,
					temperature;

	/**
	 **/
	Chunk();

	/**
	 **/
	Chunk( int x, int y, int z, bool manual_gen = false );

	~Chunk();

	/** @fn get( int x, int y, int z ) const
	 ** @param x The column to get the block from
	 ** @param y The row to get the block from
	 ** @param z The layer to get the block from
	 ** @return The block at x,y,z or nullptr
	 ** Get the block that resides at the index x,y,z in the chunk.
	 **/
	Block * get(int x, int y, int z) const;

	/** @fn set( int x, int y, int z )
	 ** @param x The column to set the block from
	 ** @param y The row to set the block from
	 ** @param z The layer to set the block from
	 ** @param new_block The new block instance
	 ** Set the block that resides at the index x,y,z in the chunk.
	 **/
	void set(int x, int y, int z, Block *new_block );

	/** @fn isBlocked( int x1, int y1, int z1, int x2, int y2, int z2 )
	 ** @param x1 The column to get the block from
	 ** @param y1 The row to get the block from
	 ** @param z1 The layer to get the block from
	 ** @return True if the block is hidden
	 ** Check if the block at x1,y1,z1 is 'blocked' from view
	 **/
	bool isBlocked(int x1, int y1, int z1, int x2, int y2, int z2);

	void noise( int seed );

	void update();

	/** @fn render()
	 ** Update the chunk if necessary, and render it to the frame buffer using Vertex Buffer Objects
	 **/
	void render();

	void generate( int new_seed );

	/** @fn save()
	 ** Saves the chunk onto the hard drive
	 **/
	void save();

	/** @fn load()
	 ** Load the chunk from the hard drive
	 **/
	void load();
};
