
#include "Buffer.h"



Buffer::Buffer()
{
}

Buffer::Buffer( const Buffer &copy_buffer )
{
	this->buffer.insert( this->buffer.end(), copy_buffer.buffer.begin(), copy_buffer.buffer.end() );
}

Buffer::Buffer( const void *copy_data, size_t byte_length )
{
	for ( size_t i=0; i<byte_length; i++ )
	{
		this->buffer.push_back( *((unsigned char*)copy_data + i) );
	}
}

Buffer::~Buffer()
{
	this->clear();
}

unsigned char Buffer::operator [] ( unsigned index ) const
{
    return this->buffer[ index ];
}

Buffer & Buffer::operator + ( const Buffer &append_buffer )
{
	this->buffer.insert( this->buffer.end(), append_buffer.buffer.begin(), append_buffer.buffer.end() );
	return *this;
}

void Buffer::operator += ( const Buffer &append_buffer )
{
	this->buffer.insert( this->buffer.end(), append_buffer.buffer.begin(), append_buffer.buffer.end() );
}

Buffer & Buffer::add( const void *append_data, size_t byte_length )
{
	for ( size_t i=0; i<byte_length; i++ )
	{
		this->buffer.push_back( *((unsigned char*)append_data + i) );
	}

	return *this;
}

void Buffer::clear()
{
	this->buffer.clear();
}

const void * Buffer::ptr() const
{
	return &this->buffer[0];
}

size_t Buffer::size() const
{
	return this->buffer.size();
}
