#include "server.h"

int main(int argc, char* argv[])
{
	//��ʼ��������
	if (!InitSever())
	{
		ExitServer();
	}

	//��������
	if (!StartService())
	{
		ExitServer();
	}

	 //��������
    inAndOut();

	//�˳�
	ExitServer();

	return 0;
}
