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

public:

	Buffer();
	Buffer( const Buffer &copy_buffer );
	Buffer( const void *copy_data, size_t byte_length );
	~Buffer();

	unsigned char operator [] ( unsigned index ) const;
	Buffer & operator + ( const Buffer &append_buffer );
	void operator += ( const Buffer &append_buffer );

	Buffer & add( const void *append_data, size_t byte_length );

	void clear();
	const void * ptr() const;
	size_t size() const;
};

#endif // __BUFFER_H__
