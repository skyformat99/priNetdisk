#ifndef NWMANGER_H
#define NWMANGER_H

#include <vector>
#include <string>
#include <map>
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <future>

#include "session.h"

class NWmanger
{
public:
	enum
	{
		LISTENSIZE =10,
		EVENTSIZE = 50,
	};
	NWmanger(const unsigned short &port, const int &epSize);
	~NWmanger();
	void doWait();

	//回调方法
	std::function<void(const int & ,std::shared_ptr<session>)> AcceptCallBack;
	std::function<void(const int &)> DeletCallBack;
	std::function<std::shared_ptr<session>(const int &)> searchSession;
	std::function<void(const std::string &msgHead ,
					   const std::string &msgBody,
					   const int &clientSock,
					   std::string &dealResult)> dealMsg;
	void doWrite(const int &writeFd);
	void doTransFile(const int &writeFd );
private:
	std::map<std::shared_ptr<session>,std::future<int>> asyncTask;
	int listenSock;
	const unsigned short listenPort;
	int epollFd;
	const int epollSize;
	struct epoll_event *events;

	/*private member function*/
	void onInit();
	void epollCtl(int fd, int op, int state);
	void epollWait();
	void doRead(const int &readFd);

	void handleEvent(const int &num);
	void doAccept();
	void deleteSession(const int &);
};

#endif // NWMANGER_H
