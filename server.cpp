#include "server.h"
#include <string.h>

//����ȫ�ֱ���

char	dataBuf[MAX_NUM_BUF];				//д������
BOOL	bConning;							//��ͻ��˵�����״̬
BOOL    bSend ;                             //���ͱ��λ
SOCKET	sServer;							//�����������׽���
CRITICAL_SECTION	cs;			            //�������ݵ��ٽ�������
HANDLE	hAcceptThread;						//���ݴ����߳̾��
HANDLE	hRecvThread;						//���ݽ����߳�
ClIENTVECTOR clientvector;
SVECTOR    stringvector;

/**
 * ��ʼ��
 */
BOOL InitSever(void)
{

	InitMember();//��ʼ��ȫ�ֱ���

	//��ʼ��SOCKET
	if (!InitSocket())
		return FALSE;

	return TRUE;
}

/**
 * ��ʼ��ȫ�ֱ���
 */
void	InitMember(void)
{
	InitializeCriticalSection(&cs);				            //��ʼ���ٽ���
	memset(dataBuf, 0, MAX_NUM_BUF);
	bSend = FALSE;
	bConning = FALSE;									    //������Ϊû������״̬
	hAcceptThread = NULL;									//����ΪNULL
	hRecvThread = NULL;
	sServer = INVALID_SOCKET;								//����Ϊ��Ч���׽���
	clientvector.clear();									//�������
	stringvector.clear();
}

/**
 *  ��ʼ��SOCKET
 */
bool InitSocket(void)
{
	//����ֵ
	int reVal;

	//��ʼ��Windows Sockets DLL
	WSADATA  wsData;
	reVal = WSAStartup(MAKEWORD(2,2),&wsData);

	//�����׽���
	sServer = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET== sServer)
		return FALSE;

	//�����׽��ַ�����ģʽ
	unsigned long ul = 1;
	reVal = ioctlsocket(sServer, FIONBIO, (unsigned long*)&ul);
	if (SOCKET_ERROR == reVal)
		return FALSE;

	//���׽���
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SERVERPORT);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	reVal = bind(sServer, (struct sockaddr*)&serAddr, sizeof(serAddr));
	if(SOCKET_ERROR == reVal )
		return FALSE;

	//����
	reVal = listen(sServer, SOMAXCONN);
	if(SOCKET_ERROR == reVal)
		return FALSE;

	//�ȴ��ͻ��˵�����
	cout << "Server succeeded!" << endl;
	cout << "Waiting for clients..." << endl;

	return TRUE;
}

/**
 *  ��������
 */
bool StartService(void)
{
    BOOL reVal = TRUE;	//����ֵ

	ShowTipMsg(TRUE);	//��ʾ�û�����

	char cInput;		//�����ַ�
	do
	{
		cin >> cInput;
		if ('s' == cInput || 'S' == cInput)
		{
			if (createAcceptThread())	//���ܿͻ���������߳�
			{
				ShowServerStartMsg(TRUE);		//�����̳߳ɹ���Ϣ
			}else{
				reVal = FALSE;
			}
			bSend = TRUE;
			break;//����ѭ����

		}else{
			ShowTipMsg(TRUE);
		}

	} while(cInput != 's' && cInput != 'S'); //��������'s'����'S'�ַ�

	return reVal;
}

/**
 *@des:create a client request thread
 */
BOOL createAcceptThread(void)
{
    bConning = TRUE;//���÷�����Ϊ����״̬

	//�����ͷ���Դ�߳�
	unsigned long ulThreadId;
	//�������տͻ��������߳�
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
 * ���ܿͻ�������
 */
DWORD __stdcall AcceptThread(void* pParam)
{
    SOCKET  sAccept;							//���ܿͻ������ӵ��׽���
	sockaddr_in addrClient;						//�ͻ���SOCKET��ַ

	while(bConning)						           //��������״̬
	{
		memset(&addrClient, 0, sizeof(sockaddr_in));					//��ʼ��
		int	lenClient = sizeof(sockaddr_in);				        	//��ַ����
		sAccept = accept(sServer, (sockaddr*)&addrClient, &lenClient);	//���ܿͻ�����
		if(INVALID_SOCKET == sAccept )
		{
		    Sleep(100);
			int nErrCode = WSAGetLastError();
			if(nErrCode == WSAEWOULDBLOCK)	//�޷��������һ�����赲���׽��ֲ���
			{
				Sleep(TIMEFOR_THREAD_SLEEP);
				continue;//�����ȴ�
			}
			else
            {
				return 0;//�߳��˳�
			}

		}
		else//���ܿͻ��˵�����
		{
            //��ʾ�ͻ��˵�IP�Ͷ˿�
            char *pClientIP = inet_ntoa(addrClient.sin_addr);
            u_short  clientPort = ntohs(addrClient.sin_port);
            cout<<"Accept a client."<<endl;
            cout<<"IP: "<<pClientIP<<"\tPort: "<<clientPort<<endl;
            //��������
			clientvector.push_back(sAccept);
		}
	}
	return 0;//�߳��˳�
}

/**
 *@des����������
 */
 void inAndOut(void)
 {
    while(bConning)
    {
        memset(dataBuf, 0, MAX_NUM_BUF);		//��ս��ջ�����
        cin.getline(dataBuf,MAX_NUM_BUF);			//��������
        //��������
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
 *��һ������
 */
bool RecvLine(SOCKET s, char* buf)
{
	bool retVal = TRUE;			//����ֵ
	bool bLineEnd = FALSE;		//�н���
	int	 nReadLen = 0;			//�����ֽ���
	int	 nDataLen = 0;			//���ݳ���
    memset(buf, 0, MAX_NUM_BUF);		//��ս��ջ�����
	while (!bLineEnd && bConning)	//��ͻ������� û�л���
	{
		nReadLen = recv(s, buf + nDataLen, 1, 0);	//ÿ�ν���һ���ֽ�
		//������
		if(SOCKET_ERROR == nReadLen)
		{
			int nErrCode = WSAGetLastError();
			if ( WSAEWOULDBLOCK == nErrCode )	//�������ݻ�����������
			{
				continue;						//����ѭ��
			}
			else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode) //�ͻ��˹ر�������
			{
			    retVal = FALSE;	//������ʧ��
				break;							//�߳��˳�
			}
		}

		if(0 == nReadLen)
		{
			retVal = FALSE;

			break;
		}

		if('\n'==*(buf + nDataLen))	//��ȡ����
		{
			bLineEnd = TRUE;
			bSend = TRUE;           //��ʼ����
		}
		else
		{
			nDataLen += nReadLen; //�������ݳ���
		}
	}
	return retVal;
}

/**
 *	��ȡ����
 */

bool recvData(SOCKET s, char* buf)
{
	memset(buf, 0, MAX_NUM_BUF);		//��ս��ջ�����
    int	 nReadLen = 0;			//�����ֽ���
    bool bLineEnd = FALSE;		//�н���
    BOOL retVal = TRUE;

    while(!bLineEnd)
    {
        nReadLen = recv(s, buf, MAX_NUM_BUF, 0);
        if(SOCKET_ERROR == nReadLen)
        {
            int nErrCode = WSAGetLastError();
            if ( WSAEWOULDBLOCK == nErrCode )	//�������ݻ�����������
            {
                continue;						//����ѭ��
            }
            else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode) //�ͻ��˹ر�������
            {
                retVal = FALSE;	//������ʧ��
                break;							//�߳��˳�
            }
        }

       if(0 == nReadLen)           //δ��ȡ������
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
    int retVal;                             //����ֵ
    bool bLineEnd = FALSE;		//�н���

    while(!bLineEnd)
    {
        retVal = send(s, str, strlen(str), 0);//һ�η���
        //������
        if (SOCKET_ERROR == retVal)
        {
            int nErrCode = WSAGetLastError();//�������
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

	return TRUE;		//���ͳɹ�
}

/**
 *@des:handle  data to choose what kind of pattern
 */
 void handleData(char* str)
 {
    SOCKET  sClient;
    char cnum;                //��ʾ�������׽���
    int num;

    if(!strncmp(WRITE, str, strlen(WRITE))) //�ж�����ָ���Ƿ�Ϊ
    {
        str += strlen(WRITE);
        cnum = *str++;
        num = cnum - '1';
        //������������
        sClient = clientvector.at(num);     //���͵�ָ���ͻ���

        if(!sendData(sClient, str))
        {
            ExitServer();//�˳�
        }

    }
    else if(!strncmp(READ, str, strlen(READ)))
    {
        str += strlen(READ);
        cnum = *str++;
        num = cnum - '1';
        sClient = clientvector.at(num);     //���͵�ָ���ͻ���

        //���տͻ�������
        if (!recvData(sClient, dataBuf))
        {
            ExitServer();//�˳�
        }
        //��ʾ�ͻ�������
        cout << dataBuf<<endl;
    }
    else if(('e'==str[0] || 'E'== str[0])&&('\n' == str[1]))     //�ж��Ƿ��˳�
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
 *  �ͷ���Դ
 */
void  ExitServer(void)
{
	closesocket(sServer);					//�ر�SOCKET
	WSACleanup();							//ж��Windows Sockets DLL
}

void ShowTipMsg(BOOL bFirstInput)
{
	if (bFirstInput)//��һ��
	{
		cout << endl;
		cout << endl;
		cout << "**********************" << endl;
		cout << "* s(S): Start server *" << endl;
		cout << "**********************" << endl;
		cout << "Please input:" ;

	}else{//�˳�������
		cout << endl;
		cout << endl;
		cout << "**********************" << endl;
		cout << "* e(E): Exit  server *" << endl;
		cout << "**********************" << endl;
		cout << " Please input:" ;
	}
}

/**
 * ��ʾ�����������ɹ���ʧ����Ϣ
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

