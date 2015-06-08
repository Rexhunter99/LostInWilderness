
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
#include <thread>

using namespace std;


enum {
	NETWORK_TCPIP	= 0,
	NETWORK_UDP		= 1,
};


void Network::HttpRequest::open( uint32_t method, string url, bool async, string uname, string pswd )
{
	if ( async == true )
	{
		// TODO: Spawn a thread here that re-runs this method with async = false
		return;
	}

	socket_t s;
	struct sockaddr_in	serv_addr;
    struct addrinfo		host_info;
    struct addrinfo		*host_info_list;
    char				buffer[32767];
	string				host;

	host = url.substr( 0, url.find('/') );

	memset( &host_info, 0, sizeof(host_info) );
	host_info.ai_family = AF_INET;
	host_info.ai_socktype = SOCK_STREAM;

	if ( getaddrinfo( host.c_str(), "80", &host_info, &host_info_list) != 0 )
	{
		fprintf( stdout, "HttpRequest::open() :: Failed to getaddrinfo\n" );
		return;
	}

    if ( ( s = socket( host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol ) ) < 0 )
    {
		fprintf( stdout, "HttpRequest::open() :: Failed opening socket\n" );
		return;
    }

    if ( connect( s, host_info_list->ai_addr, host_info_list->ai_addrlen ) < 0 )
    {
		fprintf( stdout, "HttpRequest::open() :: Failed connecting\n" );
		return;
    }

	if ( true )
	{
		string msg = 	string("GET /") + url.substr( url.find('/') ) + string(" HTTP/1.1\n" );
		msg +=			string("Host: www.thecarnivoressaga.com\n");
		msg +=			string("Connection: close\n");
		msg +=			"\n";
		int n = ::send( s, msg.c_str(), msg.length(), 0 );
	}

	while ( true )
	{
		memset( buffer, 0, 32767 );

		#if defined( _LINUX )
		int n = read( s, buffer, 32767-1 );
		if (n < 0)
		{
			fprintf( stdout, "ERROR reading from socket" );
			break;
		}
		#elif defined( _WINDOWS ) || defined( _WIN32 )
		int n = recv( s, buffer, 32767-1, 0 );
		if (n < 0)
		{
			fprintf( stdout, "HttpRequest::open() :: ERROR receiving from socket" );
			break;
		}
		#endif // define

		if ( n == 0 )
		{
			break;
		}

	}

	freeaddrinfo( host_info_list );

    #if defined( _LINUX )
    close( s );
    #elif defined( _WINDOWS ) || defined( _WIN32 )
    closesocket( s );
    #endif // define

    this->readyState = STATE_CONNECTED;
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


