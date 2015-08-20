
#if defined( _LINUX )

#	include "Network.h"

#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <string.h>
#	include <unistd.h>

typedef int socket_t;

#elif defined( _WINDOWS ) || defined( _WIN32 )

#define WIN32_LEAN_AND_MEAN

#	include "Network.h"
#	include <WinSock2.h>
#	include <Ws2tcpip.h>

typedef SOCKET socket_t;
WSADATA		wsaData;

#endif

#include <cstdio>
#include <cstdlib>
#include <array>
#include <iostream>


enum {
	NETWORK_TCPIP	= 0,
	NETWORK_UDP		= 1,
};

namespace Network {

	HttpRequest::HttpRequest()
	{
		this->m_socket = -1;
	}

	HttpRequest::~HttpRequest()
	{
		// -- Make sure the thread joins
		if ( this->m_thread.joinable() )
		{
			this->m_thread.join();
		}

		if ( this->m_socket >= 0 )
		{
			#if defined( _LINUX )
			close( (socket_t)this->m_socket );
			#elif defined( _WINDOWS ) || defined( _WIN32 )
			closesocket( (socket_t)this->m_socket );
			#endif // define
		}
	}

	void HttpRequest::open( uint32_t method, std::string url, bool async, std::string uname, std::string pswd )
	{
		//struct	sockaddr_in		serv_addr;
		struct	addrinfo		host_info;
		struct	addrinfo		*host_info_list;

		this->m_url = url;
		this->m_host = url.substr( 0, url.find('/') );
		this->m_asynchronous = async;
		this->m_method = method;
		this->m_username = uname;
		this->m_password = pswd;

		memset( &host_info, 0, sizeof(host_info) );
		host_info.ai_family = AF_INET;
		host_info.ai_socktype = SOCK_STREAM;

		if ( getaddrinfo( this->m_host.c_str(), "80", &host_info, &host_info_list) != 0 )
		{
			//int e = WSAGetLastError();
			std::cerr	<< "ERROR!\n"
                        << "Function: HttpRequest::open()\n"
                        << "Source-Line: " << (__LINE__-4) << "\n"
                        << "Source-File: " << __FILE__ << "\n"
                        << "Failed to getaddrinfo\n" << std::endl;

			return;
		}

		if ( ( this->m_socket = (int)socket( host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol ) ) < 0 )
		{
			std::cerr	<< "ERROR!\n"
                        << "Function: HttpRequest::open()\n"
                        << "Source-Line: " << (__LINE__-4) << "\n"
                        << "Source-File: " << __FILE__ << "\n"
                        << "Failed to open the socket.\n" << std::endl;
			return;
		}

		if ( connect( (socket_t)this->m_socket, host_info_list->ai_addr, host_info_list->ai_addrlen ) < 0 )
		{
			std::cerr	<< "ERROR!\n"
                        << "Function: HttpRequest::open()\n"
                        << "Source-Line: " << (__LINE__-4) << "\n"
                        << "Source-File: " << __FILE__ << "\n"
                        << "Failed connecting.\n" << std::endl;
			return;
		}

		// -- Free the host info list
		freeaddrinfo( host_info_list );

		// -- Initialise the request header
		std::string msg = "";

		// FIXME: SIGSEGVs on the second "msg" line if there is no trailing '/' character
		if ( method == HTTP_GET )
		{
			msg += std::string("GET /");
			msg += this->m_url.substr( this->m_url.find('/') );
			msg += std::string(" HTTP/1.1\n" );
		}
		else if ( method == HTTP_POST )
		{
			// TODO: Strip the query string and store it for the end of the header
			msg += std::string("POST /") + this->m_url.substr( this->m_url.find('/') ) + std::string(" HTTP/1.1\n" );
		}

		msg += std::string("Host: ") + this->m_host + std::string("\n");
		msg += std::string("Accept: text/html,text/xml,application/xhtml+xml,application/xml\n");
		msg += std::string("User-Agent: C++ HttpRequest 1.0\n");
		msg += std::string("Connection: close\n");
		msg += std::string("\n");

		if ( method == HTTP_POST )
		{
			msg += std::string( this->m_url.substr( this->m_url.find('?') ) ) + std::string("\n");
			msg += std::string("\n");
		}

		this->m_request_header = msg;

		this->readyState = STATE_OPENED;
	}

	void HttpRequest::send( std::string data )
	{
		const int			buffer_len = 2048;//32767;
		std::vector<char>	buffer;

		// -- Size the buffer
		buffer.resize( buffer_len );

		// -- Send the HttpRequest header
		int n = ::send( (socket_t)this->m_socket, this->m_request_header.c_str(), this->m_request_header.length(), 0 );

		if ( n <= 0 )
		{
			#if defined( _WINDOWS ) || defined( _WIN32 )
			if ( n == SOCKET_ERROR )
			{
				//int ws_error_code = WSAGetLastError();
			}
			#elif defined( _LINUX )
			std::cerr << "HttpRequest::send()\nAn error occured while sending to the host." << std::endl;
			#endif
		}

		this->responseText = "";

		// -- Loop and receive all the data the host sends us
		while ( true )
		{
			memset( &buffer[0], 0, buffer_len );

			#if defined( _LINUX )
			int n = read( (socket_t)this->m_socket, &buffer[0], buffer_len-1 );
			if (n < 0)
			{
				fprintf( stdout, "ERROR reading from socket" );
				break;
			}
			#elif defined( _WINDOWS ) || defined( _WIN32 )
			int n = ::recv( (socket_t)this->m_socket, &buffer[0], buffer_len-1, 0 );
			if (n < 0)
			{
				cerr	<< "ERROR!\n"
						<< "Function: HttpRequest::send()\n"
						<< "Source-Line: " << (__LINE__-4) << "\n"
						<< "Source-File: " << __FILE__ << "\n"
						<< "An issue arose when receiving from the socket " << this->m_socket << "\n" << endl;
				break;
			}
			#endif

			// Process the data that was received, here
			this->responseText += &buffer[0];

			if ( n == 0 )
			{
				break;
			}

		}
	}


	bool network_initialise()
	{
		#if defined( _WINDOWS ) || defined( _WIN32 )
		if( WSAStartup( 0x0202,&wsaData ) )
		{
			fprintf( stdout, "WinSock initialization failed!\n" );
			return false;
		}
		#endif

		return true;
	}

	void network_cleanup()
	{
		#if defined( _WINDOWS ) || defined( _WIN32 )
		WSACleanup();
		#endif
	}
}

