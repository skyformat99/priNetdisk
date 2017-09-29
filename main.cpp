#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include "prinetdisk.h"

using namespace std;

int main(int argc, char *argv[])
{
	try{
		priNetdisk  temp(8888,512);
		temp.onInit();
		temp.doLoop();
	}catch(exception &ec)
	{
		cout <<ec.what() <<endl;
	}

	return 0;
}
