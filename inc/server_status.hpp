#ifndef SERVER_STATUS_HPP
#define SERVER_STATUS_HPP

enum class ServerTCP_Status
{
	SERVER_SUCCESS,
	SERVER_SELECT_ERROR,
	SERVER_SOCKET_ERROR,
	SERVER_UNINITIALIZED,
	SERVER_SEND_FAIL
};


#endif // SERVER_STATUS_HPP