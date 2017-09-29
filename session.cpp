#include "session.h"

#include "prinetdisk.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

session::session(const int &clientsock ,/* const sockaddr_in &clientaddr ,*/
				 const std::function<void (const std::string &,
										   const std::string &,
										   const int &, std::string &)> &tempDealMsg):
	clientSock(clientsock)/*,clientAddr(clientaddr)*/,dealMsg(tempDealMsg),SPLCHAR('#')
{
	this->headOver = false;
	this->haveRead = 0;
}
session::~session()
{
	std::cout << "destory session " <<std::endl;
}

void session::splitMsg()
{
	std::string result;
	auto itor = std::find(msg,msg+bodyLen,SPLCHAR);
	if( itor - msg == bodyLen)
		result = "MSG:msg not complete";
	else
	{
		dealMsg(std::string(msg,itor),std::string(itor+1),this->clientSock , result);
		result.insert( 0 ,msg, itor - msg + 1);		//添加返回消息头部
	}
	addMsgHead(result);
	strcpy(msg,result.c_str());
}

void session::addMsgHead(std::string &result)
{
	std::string Head = std::to_string(result.size());
	while( Head.size() != MSGHEADLEN)
		Head.insert(Head.begin(),'0');
	result.insert(0,Head);
}
















