#ifndef PRINETDISK_H
#define PRINETDISK_H

#include <map>
#include <memory>
#include <unistd.h>
#include <functional>
#include "session.h"
#include "nwmanger.h"

class priNetdisk
{
public:
	priNetdisk(const unsigned short &port,const int &epSize);
	void onInit();	//初始化，包括网络NWmanger
	void doLoop();
	~priNetdisk();
	void registerMsgFun(const std::string &funName ,
						const std::function<void(const std::string &,
												 const int &,
												 std::string &)> &);

	void onMsgDeal(const std ::string &msgHead,
				  const std::string &msgBody,
				  const int &clientSock,
				  std::string &dealResult);

private:
	/* CallBack function */
	void onAccept(const int &clientSock , const std::shared_ptr<session> pSession);
	void onDelete(const int &clientSock);
	std::shared_ptr<session> onSearch(const int &clientSock);
	/*消息处理函数*/
	void setClientName(const std::string& clientName , const int &clientSock, std::string &result);
	void searchClientSocket(const std::string &otherClientName, const int &clientSock, std::string &result);
	void transFile(const std::string &otherClientName, const int &clientSock, std::string &result);
	/* END */
	NWmanger netWork;
	std::map<int,std::shared_ptr<session>> clientMap;
	std::map<std::string , int > clientMemu;
	std::map<std::string, std::function<void (const std::string&,
											  const int&,
											  std::string& )>> dealMsgFunc;
};
#endif // PRINETDISK_H
