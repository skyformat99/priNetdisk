#include "nwmanger.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/sendfile.h>
#include <fcntl.h>

NWmanger::NWmanger(const unsigned short &port, const int &epSize):
	listenSock(-1),epollFd(-1),
	listenPort(port),epollSize(epSize)
{
	onInit();
	events = new epoll_event[EVENTSIZE];
	bzero(events, sizeof(epoll_event)*EVENTSIZE);
}

NWmanger::~NWmanger()
{
	if( events != nullptr)
		delete[]  events;
	close( this->listenSock);
	close( this->epollFd);
}

void NWmanger::doWait()
{
	this->epollWait();
}

void NWmanger::onInit()
{
	sockaddr_in serverAddr;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons( this->listenPort);
	serverAddr.sin_family = AF_INET;

	int ret = this->listenSock = ::socket(AF_INET,SOCK_STREAM,0);
	if( ret < 0)
		throw std::runtime_error("socket failed");

	int opt = 1;
	ret = setsockopt(this->listenSock,SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt));
	if(ret)
		throw  std::runtime_error("set socket failed");
	ret = setsockopt(this->listenSock,SOL_SOCKET , SO_REUSEPORT , &opt , sizeof(opt));
	if(ret)
		throw  std::runtime_error("set socket failed");
	int flags;
	flags = fcntl(this->listenSock,F_GETFL,0);
	flags |= O_NONBLOCK;
	fcntl(this->listenSock,F_SETFL,flags);

	ret = ::bind(this->listenSock,(sockaddr*)&serverAddr,sizeof(sockaddr));
	if( ret < 0)
		throw std::runtime_error("bind failed");
	::listen(this->listenSock,LISTENSIZE);

	this->epollFd = epoll_create(epollSize);
	if( this->epollFd < 0)
		throw std::runtime_error("epoll create failed");

	//将监听套接字加入读检测事件中
	epollCtl(this->listenSock,EPOLL_CTL_ADD,EPOLLIN);
	std::cout << "start listen...." <<std::endl;
}

void NWmanger::epollCtl(int fd, int op, int state)
{
	struct epoll_event ev;
	bzero(&ev,sizeof(ev));
	ev.events = state ;			//监听读还是写事件
	ev.data.fd = fd;

	int ret = epoll_ctl(this->epollFd,op,fd,&ev);
	if( ret < 0)
	{
		throw std::runtime_error(strerror(errno));
	}
}

void NWmanger::epollWait()
{
	while(1)
	{
		for( auto itor = asyncTask.begin() ; itor!= asyncTask.end() ; /*itor++*/)
		{
			if( !(*itor).second.valid())
			{
				itor++;
				continue;
			}
			int ret = (*itor).second.get();
			if( ret == 0)
			{
				epollCtl((*itor).first->clientSock,EPOLL_CTL_MOD , EPOLLIN);
				asyncTask.erase(itor++);
				std::cout <<"have over" <<std::endl;
			}
			else if( ret == EAGAIN)
			{
				epollCtl((*itor).first->clientSock,EPOLL_CTL_MOD , EPOLLOUT);
				++itor;
			}
			else
				asyncTask.erase(itor++);
		}
		int ret = epoll_wait(this->epollFd, events,EVENTSIZE,-1);
		handleEvent(ret);
	}
}

void NWmanger::handleEvent(const int& num)
{
	for( int i= 0 ; i < num ; i++)
	{
		if( events[i].data.fd == this->listenSock)
			doAccept();
		else if(events[i].events & EPOLLIN)
			doRead(events[i].data.fd);
		else if( events[i].events & EPOLLOUT)
			doTransFile(events[i].data.fd);
	}
}

void NWmanger::doAccept()
{
	int clifd;
	struct sockaddr_in cliaddr;
	socklen_t  cliaddrlen = sizeof(sockaddr);
	clifd = ::accept(this->listenSock,(struct sockaddr*)&cliaddr,&cliaddrlen);
	if (clifd == -1)
	{
		std::cerr << strerror(errno);
		return ;
	}
	else
	{
		std::cout <<"accept a new client:"<< inet_ntoa(cliaddr.sin_addr)<<":" <<cliaddr.sin_port <<std::endl;
		//添加一个客户描述符和事件
		int flags;
		flags = fcntl(clifd,F_GETFL,0);
		flags |= O_NONBLOCK;
		fcntl(clifd,F_SETFL,flags);
		epollCtl(clifd,EPOLL_CTL_ADD,EPOLLIN);
		AcceptCallBack(clifd , std::make_shared<session>(clifd/*,cliaddr*/,this->dealMsg));
	}
}

void NWmanger::doRead(const int& readFd)
{
	std::cout << readFd << " start read "  <<std::endl;
	std::shared_ptr<session> pCliSession = searchSession(readFd);
	int ret = 0;
	if( pCliSession->headOver )		//读取body
	{
		int &haveRead = pCliSession->haveRead;
		int &bodyLen = pCliSession->bodyLen;
		//while( haveRead < bodyLen)
		{
			ret = ::read(readFd , pCliSession->msg+haveRead , bodyLen - haveRead );
			if( ret <= 0)
			{
				deleteSession(readFd);
				return ;
			}
			haveRead += ret;
		}
		if( haveRead == bodyLen)
		{
			pCliSession->headOver = false;
			pCliSession->msg[haveRead] = '\0';
			haveRead = 0;
			std::cout<<  pCliSession->clientSock <<"received  :"
					  <<  pCliSession->msg <<std::endl;
			pCliSession->splitMsg();
			doWrite(pCliSession->clientSock);
		}
	}
	else				//读取head
	{
		int &haveRead = pCliSession->haveRead;
		//while( haveRead < session::MSGHEADLEN)
		{
			ret = ::read(readFd , pCliSession->msg+haveRead ,session::MSGHEADLEN - haveRead );
			if( ret <= 0)
			{
				deleteSession(readFd);
				return ;
			}
			haveRead += ret;
		}
		if( haveRead == session::MSGHEADLEN )
		{
			pCliSession->msg[session::MSGHEADLEN] = '\0';

			for( auto itor = pCliSession->msg ; (*itor)!='\0' ; itor ++)
				if( !( (*itor) >= '0' && (*itor) <= '9'))
				{
					/* 头部出现错误
					 * 解决方案：
					 * 1. 发送消息给客户端，重发
					 * 2. 清除本地收到的消息，重新接受头部
					 * */
					strcpy(pCliSession->msg,"0013ERROR:msgHead");
					doWrite(readFd);
					return ;
				}
			pCliSession->bodyLen = std::atoi(pCliSession->msg);
			pCliSession->headOver = true;
			haveRead = 0;
		}
	}
}

void NWmanger::deleteSession(const int &clientSock)
{
	epollCtl(clientSock,EPOLL_CTL_DEL,EPOLLIN);
//	epollCtl(clientSock,EPOLL_CTL_DEL,EPOLLIN|EPOLLOUT);
//	close(clientSock);
	DeletCallBack(clientSock);
}

void NWmanger::doWrite(const int&writeFd)
{
	std::cout << writeFd << " start write "  <<std::endl;
	std::shared_ptr<session> pCliSession = searchSession(writeFd);
	/* ....  */
	int ret = send(writeFd,pCliSession->msg,strlen(pCliSession->msg),0);
	if(ret <0)
	{
		if( errno == EWOULDBLOCK)
		{
			/*....*/
		}
	}

	std::cout << pCliSession->msg <<std::endl;

}

void NWmanger::doTransFile(const int &writeFd)
{
	std::shared_ptr<session> clientPtr = searchSession(writeFd);
	asyncTask[clientPtr] = std::async([clientPtr]()
	{
		off64_t ret = 0;
		off64_t& haveSend = clientPtr->haveSend;
		{
			ret = sendfile64(clientPtr->clientSock,clientPtr->sendFileFd,
							 NULL,clientPtr->fileSize-haveSend);
			if( ret < 0)
			{
				if( errno == EAGAIN )
					return EAGAIN;
				else
					std::cout << strerror(errno)<<std::endl;
			}
			haveSend += ret;
		}
//		std::cout << haveSend <<std::endl;
		if( haveSend < clientPtr->fileSize)
			return EAGAIN;
		return 0;
	});
}
















