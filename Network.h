
#pragma once

#include <stdint.h>
#include <array>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace Network {


/** @class HttpRequest
 ** @brief Javascript inspired HttpRequest class
 ** @version 1
 ** @note Spec: http://www.w3.org/TR/XMLHttpRequest/ */
class HttpRequest
{
private:

	int			m_socket;
	uint32_t	m_method;
	std::string	m_host,
				m_url,
				m_username,
				m_password,
				m_response_header,
				m_request_header;
	bool		m_asynchronous;
	std::thread	m_thread;

public:

	enum HttpMethodEnum
	{
		HTTP_GET			= 0,
		HTTP_POST			= 1,
	};

	enum HttpRequestEnum
	{
		STATE_UNSENT			= 0,
		STATE_OPENED			= 1,
		STATE_HEADERS_RECEIVED	= 2,
		STATE_DOWNLOADING		= 3,
		STATE_DONE				= 4
	};

	HttpRequest();
	~HttpRequest();

	// -- Cancels the current request
	void abort();
	// -- Specifies the type of request, the URL, if the request should be handled asynchronously
	// or not, and other optional attributes of a request
	void open( uint32_t method, std::string url, bool async = false, std::string uname = "", std::string pswd = "" );
	// -- Sends the request off to the server.
	void send( std::string data = "" );
	void send( std::wstring data = L"" );
	void send( const void *data );
	//std::string getAllResponseHeaders();	//Returns header information
	//std::string getResponseHeader();	//Returns specific header information
	//setRequestHeader()	Adds a label/value pair to the header to be sent


	// -- Stores a function (or the name of a function) to be called automatically each time the readyState property changes
	void (*onreadystatechange)(void);
	// -- Returns the response data as a string
	std::string		responseText;
	// -- Holds the status of the XMLHttpRequest
	uint32_t		readyState;
	// -- Returns the response data as XML data
	//?				responseXML;
	// -- Returns the status-number (e.g. "404" for "Not Found" or "200" for "OK")
	uint32_t		status;
	// -- Returns the status-text (e.g. "Not Found" or "OK")
	std::string		statusText;
	// -- Sets the time in milliseconds that processing/fetching can occur before aborting
	uint32_t		timeout;
};

class TCPSocket
{
public:
	TCPSocket();
	~TCPSocket();

	bool init();
	bool shutdown();
};

class UDPSocket
{
public:
	UDPSocket();
	~UDPSocket();

	bool init();
	bool shutdown();
};

};
