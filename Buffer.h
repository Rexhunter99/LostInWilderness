#pragma once

#ifndef __BUFFER_H__
#define __BUFFER_H__


#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

/** @class Buffer
 ** @version 2
 ** @author Rexhunter99
 ** @author TambourineReindeer
 ** @brief Utility class Buffer for encapsulating data in a self-contained way.
 **
 **
 ** The Buffer class was created to help contain many useful functions and
 ** compression/decompression, appending buffers to each other and so on.
 ** algorithms that would be used on buffers such as encryption/decryption,
 **/

class Buffer
{
private:

	// Alias the vector as a typedef
	typedef std::vector<uint8_t> vector_t;

	vector_t buffer;

	enum FlagEnum {
		F_COMPRESSED_RLE		= 1,
		F_ENCRYPTED_XOR			= 2,
		F_ENCODED_BASE64        = 3
	} flags;

public:

	/** @fn Buffer()
	 **
	 ** Initialise an empty buffer
	 **/
	Buffer();

	/** @fn Buffer( const Buffer & copy_buffer )
	 ** @param copy_buffer A Buffer instance to be copied from
	 **
	 ** Copy the contents of @c copy_buffer into this buffer
	 **/
	Buffer( const Buffer &copy_buffer );

	/** @fn Buffer( size_t byte_length )
	 ** @param byte_length An std::size_t value of the number of bytes required
	 **
	 ** Initialize the buffer to be an empty buffer of @c byte_length bytes in length.
	 **/
	Buffer( size_t byte_length );

	/** @fn Buffer( const void * copy_data, size_t byte_length )
	 ** @param copy_data A pointer to data that is to be copied into the buffer
	 ** @param byte_length The number of bytes to read into the buffer
	 **
	 ** Copy bytes from @c copy_data running the length of @c byte_length
	 **/
	Buffer( const void * copy_data, size_t byte_length );

	~Buffer();

	// -- Access/return the byte at index in the Buffer
	uint8_t & operator [] ( unsigned index ) const;
	// -- Append the Buffer append_buffer to the existing buffer data and return a new Buffer instance
	Buffer & operator + ( const Buffer & append_buffer );
	// -- Append the Buffer append_buffer to the existing buffer data
	void operator += ( const Buffer & append_buffer );

	/** @fn add( const void * append_data, size_t byte_length )
	 ** @param append_data A pointer to data that is to be copied into the buffer
	 ** @param byte_length The number of bytes to read into the buffer
	 ** @return Return a const Buffer& reference of this Buffer
	 **
	 ** Copy bytes from @c append_data running the length of @c byte_length, add it to the end of the Buffer
	 **/
	const Buffer & add( const void * append_data, size_t byte_length );

	/** @fn clear()
	 **
	 ** Empty the buffer and free any memory it was using
	 **/
	void clear();

	/** @fn ptr()
	 ** @return Returns a const void* pointer to the internal data of the buffer
	 **
	 ** Return a pointer to the data inside the buffer, works the same as @c &myBuffer[0]
	 **/
	const void * ptr() const;

	/** @fn size
	 ** @todo Perhaps add an alias "length()"
	 **
	 ** Return the size of the buffer in bytes
	 **/
	size_t size() const;

	/** @fn rle_compress()
	 ** @return Returns the size of the compressed buffer in size_t
	 ** @author Rexhunter99
	 ** @author TambourineReindeer
	 **/
    template<typename T>
	size_t rle_compress()
	{
		/* The command char is special, a value above 0 indicates that the next byte needs to be repeated as many times.
		 * If the command char is less than zero, the next as many times (without the negative sign) are uncompressed.
		 * Theoretically this should further reduce the buffer size if there are large sections of non-repeating data.
		 */
		vector_t		compressed_buffer;
		char			command = 1;
		T				held_data = this->buffer.at( 0 );
		//size_t			held_cursor = 0;

#define expand_push( d ) for ( size_t sz = 0; sz < sizeof(T); sz++ ) { compressed_buffer.push_back( (held_data >> 8*sz) & 0xFF ); }

		for ( size_t i = 1; i < this->buffer.size(); i++ )
		{
			// -- If we have read max number of run-length bytes, or we find a byte that is different to our held one
			if ( ( command == 127 ) || ( this->buffer.at(i) != held_data ) )
			{
				// Add the command byte and the held byte
				compressed_buffer.push_back( command );
				expand_push( held_data );
				command = 0;
			}
			held_data = this->buffer.at(i);
			command++;
		}
		// Push chars at end to new buffer, add 0 to signify end of file
		compressed_buffer.push_back( command );
		expand_push( held_data );
		compressed_buffer.push_back( 0 );

#undef expand_push

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
		//this->buffer.clear();
		this->buffer = compressed_buffer;

		return compressed_buffer.size();
	}

	/** @fn rle_decompress()
	 ** @return Returns the size of the decompressed buffer
	 ** @author Rexhunter99
     ** @author TambourineReindeer
     **
     ** Should perform the inverse operation of rle8_compress(). This is based on the same code however.
     ** It assumes (if the compression is done correctly) that the first char in the buffer is the command char,
     ** giving the run length (# of repetitions). The second char is then the byte to be repeated, which
     ** will repeat as many times as the command char specifies. This process continues to the end of the file.
	 **/
	template<typename T>
	size_t rle_decompress()
	{
		vector_t		decompressed_buffer;
		char			command = 0;
		unsigned char	held_data = 0; // initially no bytes are held
		size_t			held_cursor = 1; // temp var to store current # of held bytes to decompress

		for ( size_t i = 0; i < this->buffer.size(); i+=2 )
		{
			command = this->buffer.at(i); // get first byte
			if(command == 0) // byte is empty; skip
				break;
			else if ( command > 0 ) // there are one or more bytes here
			{
				held_data = this->buffer.at(i+1);
				held_cursor = command;
				while ( held_cursor )
				{
					decompressed_buffer.push_back( held_data );
					held_cursor--;
				}
			}
			else if ( command < 0 ) // there are 'command' number of uncompressed bytes
			{
				held_data = this->buffer.at(i+1);
				held_cursor = std::fabs(command); // drop negative sign
				while ( held_cursor )
				{
					/*Instead of pushing back x number of bytes, just push that byte,
					 *then move to next uncompressed byte*/
					decompressed_buffer.push_back( this->buffer.at(i) );
					held_cursor--;
					i++;
				}
			}
		}

		this->buffer = decompressed_buffer;

		return decompressed_buffer.size();
	}

	/** @fn xor_crypt( const std::string &key )
	 ** @param key A string literal that is used to encrypt/decrypt the buffer.
	 ** @brief Encrypt/decrypt the buffer so it is not human readable or 'sensical'
	 **/
	void xor_crypt( const std::string &key );

	/** @fn base64_encode()
	 ** @author John Walker (original author)
     ** @author TambourineReindeer
     **
	 ** Encodes a string Buffer according to the MIME (Multipurpose Internet Mail Extensions)
	 ** Base64 standard for encoding binary data into a radix-64 ASCII format.
	 ** I adapted this from John Walker's own code, which is in the public domain:
	 ** http://www.fourmilab.ch/webtools/base64/
	 **/
	void base64_encode( );

    /** @fn base64_decode(void)
     ** @author John Walker (original author)
     ** @author TambourineReindeer
     ** @brief Performs decoding for Base64-encoded data received across a network.
     **/
    void base64_decode(void);
};

#endif // __BUFFER_H__
