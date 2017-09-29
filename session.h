#ifndef SESSION_H
#define SESSION_H

#include <arpa/inet.h>
#include <memory>
#include <functional>

class priNetdisk;

class session
{
public:
	enum{
		MSGHEADLEN = 4,
		MSGBODYLEN = 1024
	};
	const char SPLCHAR;
	session(const int & ,/* const sockaddr_in &,*/
			const std::function<void(const std::string &msgHead ,
									 const std::string &msgBody,
									 const int &clientSock,
									 std::string &dealResult)>& );
	~session();
	std::string clientName;
	std::function<void(const std::string &msgHead ,
					   const std::string &msgBody,
					   const int &clientSock,
					   std::string &dealResult)> dealMsg;
	char* getMsg()
	{
		return msg;
	}
	void addMsgHead(std::string&);
	friend class NWmanger;

private:
	char msg[MSGBODYLEN];
	int haveRead;
	int bodyLen;
	bool headOver;			//标识读取head 是否已经读完
	//sockaddr_in clientAddr;
	int clientSock;
	void splitMsg();


};

#endif // SESSION_H
