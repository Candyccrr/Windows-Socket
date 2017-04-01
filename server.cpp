#include "server.h"
#include <string.h>

//定义全局变量

char	dataBuf[MAX_NUM_BUF];				//写缓冲区
BOOL	bConning;							//与客户端的连接状态
BOOL    bSend ;                             //发送标记位
SOCKET	sServer;							//服务器监听套接字
CRITICAL_SECTION	cs;			            //保护数据的临界区对象
HANDLE	hAcceptThread;						//数据处理线程句柄
HANDLE	hRecvThread;						//数据接收线程
ClIENTVECTOR clientvector;
SVECTOR    stringvector;

/**
 * 初始化
 */
BOOL InitSever(void)
{

	InitMember();//初始化全局变量

	//初始化SOCKET
	if (!InitSocket())
		return FALSE;

	return TRUE;
}

/**
 * 初始化全局变量
 */
void	InitMember(void)
{
	InitializeCriticalSection(&cs);				            //初始化临界区
	memset(dataBuf, 0, MAX_NUM_BUF);
	bSend = FALSE;
	bConning = FALSE;									    //服务器为没有运行状态
	hAcceptThread = NULL;									//设置为NULL
	hRecvThread = NULL;
	sServer = INVALID_SOCKET;								//设置为无效的套接字
	clientvector.clear();									//清空向量
	stringvector.clear();
}

/**
 *  初始化SOCKET
 */
bool InitSocket(void)
{
	//返回值
	int reVal;

	//初始化Windows Sockets DLL
	WSADATA  wsData;
	reVal = WSAStartup(MAKEWORD(2,2),&wsData);

	//创建套接字
	sServer = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET== sServer)
		return FALSE;

	//设置套接字非阻塞模式
	unsigned long ul = 1;
	reVal = ioctlsocket(sServer, FIONBIO, (unsigned long*)&ul);
	if (SOCKET_ERROR == reVal)
		return FALSE;

	//绑定套接字
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SERVERPORT);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	reVal = bind(sServer, (struct sockaddr*)&serAddr, sizeof(serAddr));
	if(SOCKET_ERROR == reVal )
		return FALSE;

	//监听
	reVal = listen(sServer, SOMAXCONN);
	if(SOCKET_ERROR == reVal)
		return FALSE;

	//等待客户端的连接
	cout << "Server succeeded!" << endl;
	cout << "Waiting for clients..." << endl;

	return TRUE;
}

/**
 *  启动服务
 */
bool StartService(void)
{
    BOOL reVal = TRUE;	//返回值

	ShowTipMsg(TRUE);	//提示用户输入

	char cInput;		//输入字符
	do
	{
		cin >> cInput;
		if ('s' == cInput || 'S' == cInput)
		{
			if (createAcceptThread())	//接受客户端请求的线程
			{
				ShowServerStartMsg(TRUE);		//创建线程成功信息
			}else{
				reVal = FALSE;
			}
			bSend = TRUE;
			break;//跳出循环体

		}else{
			ShowTipMsg(TRUE);
		}

	} while(cInput != 's' && cInput != 'S'); //必须输入's'或者'S'字符

	return reVal;
}

/**
 *@des:create a client request thread
 */
BOOL createAcceptThread(void)
{
    bConning = TRUE;//设置服务器为运行状态

	//创建释放资源线程
	unsigned long ulThreadId;
	//创建接收客户端请求线程
	hAcceptThread = CreateThread(NULL, 0, AcceptThread, NULL, 0, &ulThreadId);
	if( NULL == hAcceptThread)
	{
		bConning = FALSE;
		return FALSE;
	}
	else
    {
		CloseHandle(hAcceptThread);
	}
	return TRUE;
}

/**
 * 接受客户端连接
 */
DWORD __stdcall AcceptThread(void* pParam)
{
    SOCKET  sAccept;							//接受客户端连接的套接字
	sockaddr_in addrClient;						//客户端SOCKET地址

	while(bConning)						           //服务器的状态
	{
		memset(&addrClient, 0, sizeof(sockaddr_in));					//初始化
		int	lenClient = sizeof(sockaddr_in);				        	//地址长度
		sAccept = accept(sServer, (sockaddr*)&addrClient, &lenClient);	//接受客户请求
		if(INVALID_SOCKET == sAccept )
		{
		    Sleep(100);
			int nErrCode = WSAGetLastError();
			if(nErrCode == WSAEWOULDBLOCK)	//无法立即完成一个非阻挡性套接字操作
			{
				Sleep(TIMEFOR_THREAD_SLEEP);
				continue;//继续等待
			}
			else
            {
				return 0;//线程退出
			}

		}
		else//接受客户端的请求
		{
            //显示客户端的IP和端口
            char *pClientIP = inet_ntoa(addrClient.sin_addr);
            u_short  clientPort = ntohs(addrClient.sin_port);
            cout<<"Accept a client."<<endl;
            cout<<"IP: "<<pClientIP<<"\tPort: "<<clientPort<<endl;
            //加入容器
			clientvector.push_back(sAccept);
		}
	}
	return 0;//线程退出
}

/**
 *@des：处理数据
 */
 void inAndOut(void)
 {
    while(bConning)
    {
        memset(dataBuf, 0, MAX_NUM_BUF);		//清空接收缓冲区
        cin.getline(dataBuf,MAX_NUM_BUF);			//输入数据
        //发送数据
        if(bSend)
        {
            handleData(dataBuf);
        }
    }
 }

/**
 *@des:read a line words
 */
 /**
 *读一行数据
 */
bool RecvLine(SOCKET s, char* buf)
{
	bool retVal = TRUE;			//返回值
	bool bLineEnd = FALSE;		//行结束
	int	 nReadLen = 0;			//读入字节数
	int	 nDataLen = 0;			//数据长度
    memset(buf, 0, MAX_NUM_BUF);		//清空接收缓冲区
	while (!bLineEnd && bConning)	//与客户端连接 没有换行
	{
		nReadLen = recv(s, buf + nDataLen, 1, 0);	//每次接收一个字节
		//错误处理
		if(SOCKET_ERROR == nReadLen)
		{
			int nErrCode = WSAGetLastError();
			if ( WSAEWOULDBLOCK == nErrCode )	//接受数据缓冲区不可用
			{
				continue;						//继续循环
			}
			else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode) //客户端关闭了连接
			{
			    retVal = FALSE;	//读数据失败
				break;							//线程退出
			}
		}

		if(0 == nReadLen)
		{
			retVal = FALSE;

			break;
		}

		if('\n'==*(buf + nDataLen))	//读取结束
		{
			bLineEnd = TRUE;
			bSend = TRUE;           //开始发送
		}
		else
		{
			nDataLen += nReadLen; //增加数据长度
		}
	}
	return retVal;
}

/**
 *	读取数据
 */

bool recvData(SOCKET s, char* buf)
{
	memset(buf, 0, MAX_NUM_BUF);		//清空接收缓冲区
    int	 nReadLen = 0;			//读入字节数
    bool bLineEnd = FALSE;		//行结束
    BOOL retVal = TRUE;

    while(!bLineEnd)
    {
        nReadLen = recv(s, buf, MAX_NUM_BUF, 0);
        if(SOCKET_ERROR == nReadLen)
        {
            int nErrCode = WSAGetLastError();
            if ( WSAEWOULDBLOCK == nErrCode )	//接受数据缓冲区不可用
            {
                continue;						//继续循环
            }
            else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode) //客户端关闭了连接
            {
                retVal = FALSE;	//读数据失败
                break;							//线程退出
            }
        }

       if(0 == nReadLen)           //未读取到数据
        {
            retVal = FALSE;
            break;
        }
        bLineEnd = TRUE;

    }


    return retVal;
}

/**
 * @Des:send data to client
 */
bool sendData(SOCKET s, char* str)
{
    int retVal;                             //返回值
    bool bLineEnd = FALSE;		//行结束

    while(!bLineEnd)
    {
        retVal = send(s, str, strlen(str), 0);//一次发送
        //错误处理
        if (SOCKET_ERROR == retVal)
        {
            int nErrCode = WSAGetLastError();//错误代码
            if (WSAEWOULDBLOCK == nErrCode)
            {
                continue;
            }
            else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode)
            {
                return FALSE;
            }
        }
        bLineEnd = TRUE;
    }

	return TRUE;		//发送成功
}

/**
 *@des:handle  data to choose what kind of pattern
 */
 void handleData(char* str)
 {
    SOCKET  sClient;
    char cnum;                //显示几号字套接字
    int num;

    if(!strncmp(WRITE, str, strlen(WRITE))) //判断输入指令是否为
    {
        str += strlen(WRITE);
        cnum = *str++;
        num = cnum - '1';
        //增加容量处理
        sClient = clientvector.at(num);     //发送到指定客户端

        if(!sendData(sClient, str))
        {
            ExitServer();//退出
        }

    }
    else if(!strncmp(READ, str, strlen(READ)))
    {
        str += strlen(READ);
        cnum = *str++;
        num = cnum - '1';
        sClient = clientvector.at(num);     //发送到指定客户端

        //接收客户端数据
        if (!recvData(sClient, dataBuf))
        {
            ExitServer();//退出
        }
        //显示客户端数据
        cout << dataBuf<<endl;
    }
    else if(('e'==str[0] || 'E'== str[0])&&('\n' == str[1]))     //判断是否退出
    {
        bConning = FALSE;
        ExitServer();
        ShowTipMsg(FALSE);
    }
    else
    {
        cout <<"Input error!!"<<endl;
    }
 }

/**
 *  释放资源
 */
void  ExitServer(void)
{
	closesocket(sServer);					//关闭SOCKET
	WSACleanup();							//卸载Windows Sockets DLL
}

void ShowTipMsg(BOOL bFirstInput)
{
	if (bFirstInput)//第一次
	{
		cout << endl;
		cout << endl;
		cout << "**********************" << endl;
		cout << "* s(S): Start server *" << endl;
		cout << "**********************" << endl;
		cout << "Please input:" ;

	}else{//退出服务器
		cout << endl;
		cout << endl;
		cout << "**********************" << endl;
		cout << "* e(E): Exit  server *" << endl;
		cout << "**********************" << endl;
		cout << " Please input:" ;
	}
}

/**
 * 显示启动服务器成功与失败消息
 */
void  ShowServerStartMsg(BOOL bSuc)
{
	if (bSuc)
	{
		cout << "**********************" << endl;
		cout << "* Server succeeded!  *" << endl;
		cout << "**********************" << endl;
	}else{
		cout << "**********************" << endl;
		cout << "* Server failed   !  *" << endl;
		cout << "**********************" << endl;
	}

}

