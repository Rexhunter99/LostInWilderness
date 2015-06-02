
#include "Buffer.h"

#include "Exceptions.h"
#include <exception>
#include <iostream>



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
    if ( index >= this->buffer.size() )
    {
        throw custom_exception( "Buffer overflow." );
    }

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

size_t Buffer::rle8_compress()
{
	/* The command char is special, a value above 0 indicates that the next byte needs to be repeated as many times.
	 * If the command char is less than zero, the next as many times (without the negative sign) are uncompressed.
	 * Theoretically this should further reduce the buffer size if there are large sections of non-repeating data.
	 */
	std::vector<unsigned char> compressed_buffer;
	char			command = 1;
	unsigned char	held_u8 = this->buffer.at( 0 );
	size_t			held_cursor = 0;

	for ( size_t i = 1; i < this->buffer.size(); i++ )
	{
		// -- If we have read 126 run-length bytes, or we find a byte that is different to our held one
		if ( ( command == 127 ) || ( this->buffer.at(i) != held_u8 ) )
		{
			// Add the command byte and the held byte
			compressed_buffer.push_back( command );
			compressed_buffer.push_back( held_u8 );
			command = 0;
		}

		held_u8 = this->buffer.at(i);
		command++;
	}

	// -- Debug builds should generate a warning if the compressed buffer size is larger than the uncompressed buffer
	#ifdef _DEBUG
	if ( this->buffer.size() < compressed_buffer.size() )
	{
		// TODO: add new iostreams that output to log files
		std::cout << "[ WARNING ] The size of the compressed buffer (" <<
		compressed_buffer.size() <<
		"b) is larger than the uncompressed buffer" <<
		this->buffer.size() <<
		"b)" << std::endl;
	}
	#endif // _DEBUG

	// -- Replace the buffer contents with the compressed version
	this->buffer = compressed_buffer;

	return compressed_buffer.size();
}

size_t Buffer::rle16_compress()
{
	std::vector<unsigned char> compressed_buffer;

	return compressed_buffer.size();
}
