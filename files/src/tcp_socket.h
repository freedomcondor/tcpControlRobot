/*-------------------------------------------------------------*/
/*
 * 	version 1.1:	add read time out
 */
/*-------------------------------------------------------------*/
class TCPSocket
{
private:
	int sock, server_fd;
	int SERorCLI;
	char address[50];
	int port;

public:

	TCPSocket(char address_ch[], int port);
	~TCPSocket();
	int Open();
	int Close();

	int Read(char *buffer,int *len);
	int Read(char *buffer,int *len, int timeout);
	int Write(char *buffer, int len);
};
