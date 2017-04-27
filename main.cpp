#include "server.h"

int main(int argc, char* argv[])
{
	//初始化服务器
	if (!InitSever())
	{
		ExitServer();
	}

	//启动服务
	if (!StartService())
	{
		ExitServer();
	}

	 //处理数据
    inAndOut();

	//退出
	ExitServer();

	return 0;
}
