#include "prinetdisk.h"
#include <functional>
#include <cstring>


priNetdisk::priNetdisk(const unsigned short &port,const int &epSize):
	netWork(port,epSize)
{

}

void priNetdisk::doLoop()
{
	netWork.doWait();
}

void priNetdisk::onAccept(const int &clientSock , const std::shared_ptr<session> pSession)
{
	clientMap[clientSock] = pSession;
}

void priNetdisk::onDelete(const int &clientSock)
{
	clientMap.erase(clientSock);
}

std::shared_ptr<session> priNetdisk::onSearch(const int &clientSock)
{
	return clientMap[clientSock];
}



/* 进行回调函数注册 */
void priNetdisk::onInit()
{
	/*网络模块回调函数注册*/
	netWork.AcceptCallBack = std::bind(&priNetdisk::onAccept,this,
									   std::placeholders::_1 ,
									   std::placeholders::_2);

	netWork.DeletCallBack = std::bind(&priNetdisk::onDelete,this,std::placeholders::_1);
	netWork.searchSession = std::bind(&priNetdisk::onSearch,this,std::placeholders::_1);
	netWork.dealMsg = std::bind(&priNetdisk::onMsgDeal,this,
								std::placeholders::_1,
								std::placeholders::_2,
								std::placeholders::_3,
								std::placeholders::_4);
	/* 消息处理函数添加 */
	registerMsgFun("setName" , std::bind(&priNetdisk::setClientName,this,
										 std::placeholders::_1 ,
										 std::placeholders::_2 ,
										 std::placeholders::_3 ));
	registerMsgFun("searchClient" , std::bind(&priNetdisk::searchClientSocket,this,
										 std::placeholders::_1 ,
										 std::placeholders::_2 ,
										 std::placeholders::_3 ));
}

priNetdisk::~priNetdisk()
{
	for( auto &te : clientMap)
	{
		close( te.first );
	}
}

void priNetdisk::registerMsgFun(const std::string &funName,
								const std::function<void (const std::string &,
														  const int& clientSock,
														  std::string &)> &tempFun)
{
	this->dealMsgFunc[funName] = tempFun;
}

void priNetdisk::onMsgDeal(const std::string &msgHead ,
						   const std::string &msgBody,
						   const int &clientSock,
						   std::string &dealResult)
{
	if( dealMsgFunc.find(msgHead) == dealMsgFunc.end())	//对象消息处理函数未找到
	{
		dealResult = "dealFunction not found";
		return ;
	}
	this->dealMsgFunc[msgHead](msgBody ,clientSock, dealResult);//调用消息处理函数
}

/*消息处理函数*/
//1.设置客户端名称
void priNetdisk::setClientName(const std::string &clientName, const int &clientSock , std::string &result)
{
	if( clientMap.find(clientSock) == clientMap.end())		//对应客户端socket未找到
	{
		result = "client object not found";
		return;
	}
	clientMap[clientSock]->clientName = clientName;
	clientMemu[clientName] = clientSock;
	result = "add "+clientName;
	return ;
}
//2.查找客户端IP+PORT
void priNetdisk::searchClientSocket(const std::string &otherClientName, const int &clientSock, std::string &result)
{
	if( clientMemu.find(otherClientName) == clientMemu.end())
	{
		result = otherClientName+"Not Found";
		return ;
	}
	/*给主动客户端查找  被动客户端*/
	int resultSock = clientMemu[otherClientName];
	sockaddr_in otherClientAddr;
	socklen_t addrLen = sizeof( sockaddr );
	if(getpeername(resultSock,(sockaddr*)&otherClientAddr,&addrLen) < 0)
	{
		result = "get other ip:port error";
		return ;
	}

	result.erase();
	result.append(inet_ntoa(otherClientAddr.sin_addr));
	result.push_back(':');
	result.append(std::to_string(otherClientAddr.sin_port));

	/*给被动客户端发送主动客户端信息*/
	sockaddr_in  clientAddr;
	std::string  clientMsg;
	if(getpeername(clientSock,(sockaddr*)&clientAddr,&addrLen) < 0)
	{
		clientMsg = "get other ip:port error";
	}
	else
	{
		clientMsg.append(inet_ntoa(clientAddr.sin_addr));
		clientMsg.push_back(':');
		clientMsg.append(std::to_string(clientAddr.sin_port));
	}
	clientMap[resultSock]->addMsgHead(clientMsg);
	strcpy(clientMap[resultSock]->getMsg() ,clientMsg.c_str());
	netWork.doWrite(resultSock);
}
/*END*/
