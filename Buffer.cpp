
#include "Buffer.h"

#include "Exceptions.h"
#include <exception>
#include <iostream>
#include <cmath>


// Table of characters for encoding and decoding base64
static uint8_t code_table[256];

Buffer::Buffer()
{
}

Buffer::Buffer( const Buffer &copy_buffer )
{
	this->buffer.insert( this->buffer.end(), copy_buffer.buffer.begin(), copy_buffer.buffer.end() );
}

Buffer::Buffer( size_t byte_length )
{
	this->buffer.resize( byte_length );
}

Buffer::Buffer( const void *copy_data, size_t byte_length )
{
	this->buffer.resize( byte_length );

	for ( size_t i=0; i<byte_length; i++ )
	{
		this->buffer.at(i) = *((uint8_t*)copy_data + i);
	}
}

Buffer::~Buffer()
{
    this->clear();
}

unsigned char & Buffer::operator [] ( unsigned index ) const
{
    if ( index >= this->buffer.size() )
    {
        throw custom_exception( "Buffer overflow." );
    }

    return const_cast<unsigned char&>(this->buffer.at(index));
}

Buffer & Buffer::operator + ( const Buffer & append_buffer )
{
	Buffer b( this->size() + append_buffer.size() );
	b.buffer.insert( b.buffer.end(), this->buffer.begin(), this->buffer.end() );
	b.buffer.insert( b.buffer.end(), append_buffer.buffer.begin(), append_buffer.buffer.end() );
	return b;
}

void Buffer::operator += ( const Buffer &append_buffer )
{
	this->buffer.insert( this->buffer.end(), append_buffer.buffer.begin(), append_buffer.buffer.end() );
}

const Buffer & Buffer::add( const void *append_data, size_t byte_length )
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


void Buffer::xor_crypt( const std::string &key )
{
	for (auto i=0; i<this->size(); i++)
	{
		this->buffer.at(i) = this->buffer.at(i) ^ key.at( i % (key.length()) );
	}
}

void Buffer::base64_encode()
{
    std::vector<uint8_t> encoded_buffer;
    int            i,j;

    /* Initialise the encode/decode character table or alphabet. According to John Walker and Christian Ferrari,
     * initialising like this allows it to work for both ASCII and EBCDIC systems (EBCDIC doesn't have all chars consecutive).
     * If this way of initialising isn't needed, it can be replaced with a predefined array later.
     */
    for(i=0; i<9; i++)
    {
        // A-J, a-j
        code_table[i] = 'A' + i;
        code_table[i+9] = 'J' + i;
        code_table[26 + i] = 'a' + i;
        code_table[35 + i] = 'j' + i;
    }

    for(i=0; i<8; i++)
    {
        // S-Z, s-z
        code_table[i + 18] = 'S' + i;
        code_table[44 + i] = 's' + i;
    }

    for(i=0; i<10; i++)
        // numbers
        code_table[52 + i] = '0' + i;

    code_table[62] = '+';
    code_table[63] = '/';

    for(size_t bi = 0; bi < this->buffer.size(); bi++) // buffer index
    {
        unsigned char input_bytes[3], output_bytes[4];
        input_bytes[0], input_bytes[1], input_bytes[2] = 0;

        for(i=0; i<3; i++)
        {
            input_bytes[i] = this->buffer.at(i);
        }

        // Please explain to me what the heck is going on here; for now pretend it's magic
        if(i>0)
        {
            output_bytes[0] = code_table[input_bytes[0] >> 2];
            output_bytes[1] = code_table[((input_bytes[0] & 3) << 4) | (input_bytes[1])];
            output_bytes[2] = code_table[((input_bytes[1] & 0xF) << 2) | (input_bytes[2] >> 6)];
            output_bytes[3] = code_table[input_bytes[2] & 0x3F];

            // add equal sign char to output if there are fewer than 3 input chars
            if(i<3)
            {
                output_bytes[3] = '=';
                if(i<2)
                {
                    output_bytes[2] = '=';
                }
            }
        }

        // Write the encoded bytes
        for(j=0; j<4; j++)
        {
            encoded_buffer.push_back( output_bytes[j]);
        }
    }

    // Replace buffer with encoded version
    this->buffer = encoded_buffer;
}

void Buffer::base64_decode(void)
{
    std::vector<uint8_t> decoded_buffer;
    int            i;

    // Initialise decoding section of code table
    for(i=0; i<255; i++)
        code_table[i] = 0x80;
    for(i='A'; i<='I'; i++)
        code_table[i] = 0 + (i - 'A');
    for(i='J'; i<='R'; i++)
        code_table[i] = 9 + (i - 'J');
    for(i='S'; i<='Z'; i++)
        code_table[i] = 18 + (i - 'S');
    for(i='a'; i<='i'; i++)
        code_table[i] = 26 + (i - 'a');
    for(i='j'; i<='r'; i++)
        code_table[i] = 35 + (i - 'j');
    for(i='s'; i<='z'; i++)
        code_table[i] = 44 + (i - 's');
    for(i='0'; i<='9'; i++)
        code_table[i] = 52 + (i - '0');
    code_table['+'] = 62;
    code_table['/'] = 63;
    code_table['='] = 0;

    for(size_t bi = 0; bi < this->buffer.size(); bi++) // buffer index
    {
        // Clarify what input_a & input_b are for; output_bytes is decoded output
        unsigned char input_a[4], input_b[4], output_bytes[3], c;

        for(i=0; i<4; i++)
        {
            c = this->buffer.at(i);
            if(code_table[c] & 0x80)
            {
                #ifdef _DEBUG
                std::cout << "Illegal character " << c << " in input file. Skipping." << std::endl;
                #endif // _DEBUG
                i--;
                continue;
            }
            input_a[i] = c;
            input_b[i] = code_table[c];
        }
        // More black magic; TODO: explain
        output_bytes[0] = (input_b[0] << 2) | (input_b[1] >> 4);
        output_bytes[1] = (input_b[1] << 4) | (input_b[2] >> 2);
        output_bytes[2] = (input_b[2] << 6) | input_b[3];
        i = input_a[2] == '=' ? 1 : (input_a[3] == '=' ? 2:3);

        if(i<3)
            return;
    }

    this->buffer = decoded_buffer;
}
