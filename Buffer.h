#pragma once

#ifndef __BUFFER_H__
#define __BUFFER_H__


#include <cstdlib>
#include <stdint.h>
#include <vector>


/** @class Buffer
 ** @version 1
 ** @author James Ogden
 **/
class Buffer
{
private:

	std::vector<unsigned char> buffer;
	enum FlagEnum {
		F_COMPRESSED_RLE8		= 1,
		F_COMPRESSED_RLE16		= 2
	} flags;

public:

	Buffer();
	Buffer( const Buffer &copy_buffer );
	Buffer( size_t byte_length );
	Buffer( const void *copy_data, size_t byte_length );
	~Buffer();

	unsigned char operator [] ( unsigned index ) const;
	Buffer & operator + ( const Buffer &append_buffer );
	void operator += ( const Buffer &append_buffer );

	Buffer & add( const void *append_data, size_t byte_length );

	void clear();
	const void * ptr() const;
	size_t size() const;

	/**
	 * @todo //TODO: RLE-8 Compression
	 * @todo //TODO: RLE-16 Compression
	 */

	size_t rle8_compress();
	size_t rle16_compress();
	size_t rle8_decompress();
	size_t rle16_decompress();
};

#endif // __BUFFER_H__
