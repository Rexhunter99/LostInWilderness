
#if defined( AF_PLATFORM_LINUX )

#	include "Network.h"

#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>

typedef int socket_t;

#elif defined( AF_PLATFORM_WINDOWS )

#define WIN32_LEAN_AND_MEAN

#	include "Network.h"
#	include <WinSock2.h>
#	include <Ws2tcpip.h>

typedef SOCKET socket_t;
WSADATA		wsaData;

#endif

#include <cstdio>
#include <array>
#include <iostream>

using namespace std;


enum {
	NETWORK_TCPIP	= 0,
	NETWORK_UDP		= 1,
};

Network::HttpRequest::HttpRequest()
{
	this->m_socket = -1;
	this->m_thread = nullptr;
}

Network::HttpRequest::~HttpRequest()
{
	if ( this->m_thread )
	{
		delete this->m_thread;
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

void Network::HttpRequest::open( uint32_t method, string url, bool async, string uname, string pswd )
{
	struct	sockaddr_in		serv_addr;
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
		cerr	<< "ERROR!\n"
				<< "Function: HttpRequest::open()\n"
				<< "Source-Line: " << (__LINE__-4)
				<< "Source-File: " << __FILE__
				<< "Failed to getaddrinfo\n" << endl;
		return;
	}

    if ( ( (socket_t)this->m_socket = socket( host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol ) ) < 0 )
    {
		cerr	<< "ERROR!\n"
				<< "Function: HttpRequest::open()\n"
				<< "Source-Line: " << (__LINE__-4)
				<< "Source-File: " << __FILE__
				<< "Failed to open the socket.\n" << endl;
		return;
    }

    if ( connect( (socket_t)this->m_socket, host_info_list->ai_addr, host_info_list->ai_addrlen ) < 0 )
    {
		cerr	<< "ERROR!\n"
				<< "Function: HttpRequest::open()\n"
				<< "Source-Line: " << (__LINE__-4)
				<< "Source-File: " << __FILE__
				<< "Failed connecting.\n" << endl;
		return;
    }

	// -- Free the host info list
	freeaddrinfo( host_info_list );

	// -- Initialise the request header
	string msg = "";

	if ( method == HTTP_GET )
	{
		msg += string("GET /") + this->m_url.substr( this->m_url.find('/') ) + string(" HTTP/1.1\n" );
	}
	else if ( method == HTTP_POST )
	{
		// TODO: Strip the query string and store it for the end of the header
		msg += string("POST /") + this->m_url.substr( this->m_url.find('/') ) + string(" HTTP/1.1\n" );
	}

	msg += string("Host: ") + this->m_host + string("\n");
	msg += string("Accept: text/html,text/xml,application/xhtml+xml,application/xml\n");
	msg += string("User-Agent: C++ HttpRequest 1.0\n");
	msg += string("Connection: close\n");
	msg += string("\n");

	if ( method == HTTP_POST )
	{
		msg += string( this->m_url.substr( this->m_url.find('?') ) ) + string("\n");
		msg += string("\n");
	}

	this->m_request_header = msg;

	this->readyState = STATE_OPENED;
}

void Network::HttpRequest::send()
{
	const int			buffer_len = 32767;
	std::vector<char>	buffer;

	// -- Size the buffer
	buffer.resize( buffer_len );

	// -- Send the HttpRequest header
	int n = ::send( (socket_t)this->m_socket, this->m_request_header.c_str(), this->m_request_header.length(), 0 );

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
		int n = recv( (socket_t)this->m_socket, &buffer[0], buffer_len-1, 0 );
		if (n < 0)
		{
			fprintf( stdout, "HttpRequest::open() :: ERROR receiving from socket" );
			break;
		}
		#endif // define

		cout << buffer;

		if ( n == 0 )
		{
			break;
		}

	}

	cout << endl;
}


Network::network_initialise()
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

Network::network_cleanup()
{
	#if defined( _WINDOWS ) || defined( _WIN32 )
	WSACleanup();
	#endif
}


