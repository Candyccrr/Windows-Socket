#include <process.h>
#include <stdio.h>
#include "sclient.h"

extern BOOL bSend;
extern char	dataBuf[MAX_NUM_BUF];
/*
 * ���캯��
 */
CClient::CClient(const SOCKET sClient, const sockaddr_in &addrClient)
{
	//��ʼ������
	m_hThreadRecv = NULL;
	m_hThreadSend = NULL;
	m_socket = sClient;
	m_addr = addrClient;
	m_bConning = FALSE;
	m_bExit = FALSE;
	m_bSend = FALSE;
	memset(m_data.buf, 0, MAX_NUM_BUF);

	//�����¼�
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);//�ֶ������ź�״̬����ʼ��Ϊ���ź�״̬

	//��ʼ���ٽ���
	InitializeCriticalSection(&m_cs);
}
/*
 * ��������
 */
CClient::~CClient()
{
	closesocket(m_socket);			//�ر��׽���
	m_socket = INVALID_SOCKET;		//�׽�����Ч
	DeleteCriticalSection(&m_cs);	//�ͷ��ٽ�������
	CloseHandle(m_hEvent);			//�ͷ��¼�����
}

/*
 * �������ͺͽ��������߳�
 */
BOOL CClient::StartRuning(void)
{
	m_bConning = TRUE;//��������״̬

	//�������������߳�
	unsigned long ulThreadId;
	m_hThreadRecv = CreateThread(NULL, 0, RecvDataThread, this, 0, &ulThreadId);
	if(NULL == m_hThreadRecv)
	{
		return FALSE;
	}else{
		CloseHandle(m_hThreadRecv);
	}

	//�������տͻ������ݵ��߳�
	m_hThreadSend =  CreateThread(NULL, 0, SendDataThread, this, 0, &ulThreadId);
	if(NULL == m_hThreadSend)
	{
		return FALSE;
	}else{
		CloseHandle(m_hThreadSend);
	}

	return TRUE;
}


/*
 * ���տͻ�������
 */
DWORD  CClient::RecvDataThread(void* pParam)
{
	CClient *pClient = (CClient*)pParam;	//�ͻ��˶���ָ��
	int		reVal;							//����ֵ
	char	temp[MAX_NUM_BUF];				//��ʱ����


	while(pClient->m_bConning)				//����״̬
	{
	    memset(temp, 0, MAX_NUM_BUF);
		reVal = recv(pClient->m_socket, temp, MAX_NUM_BUF, 0);	//��������

		//������󷵻�ֵ
		if (SOCKET_ERROR == reVal)
		{
			int nErrCode = WSAGetLastError();

			if ( WSAEWOULDBLOCK == nErrCode )	//�������ݻ�����������
			{
				continue;						//����ѭ��
			}else if (WSAENETDOWN == nErrCode ||//�ͻ��˹ر�������
					 WSAETIMEDOUT == nErrCode ||
					WSAECONNRESET == nErrCode )
			{
				break;							//�߳��˳�
			}
		}

		//�ͻ��˹ر�������
		if ( reVal == 0)
		{
			break;
		}

		//�յ�����
		if (reVal > 0)
		{
		    EnterCriticalSection(&pClient->m_cs);
		    char *pClientIP = inet_ntoa(pClient->m_addr.sin_addr);
            u_short  clientPort = ntohs(pClient->m_addr.sin_port);
			cout<<"IP: "<<pClientIP<<"\tPort: "<<clientPort<<":"<<temp<<endl;      //�����ʾ����
            LeaveCriticalSection(&pClient->m_cs);

			memset(temp, 0, MAX_NUM_BUF);	//�����ʱ����
		}

	}
	pClient->m_bConning = FALSE;			//��ͻ��˵����ӶϿ�
	return 0;								//�߳��˳�
}

/*
 * @des: ��ͻ��˷�������
 */
DWORD CClient::SendDataThread(void* pParam)
{
	CClient *pClient = (CClient*)pParam;//ת����������ΪCClientָ��
	while(pClient->m_bConning)//����״̬
	{
        if(pClient->m_bSend || bSend)
        {
			//�����ٽ���
			EnterCriticalSection(&pClient->m_cs);
			//��������
			int val = send(pClient->m_socket, dataBuf, strlen(dataBuf),0);
			//�����ش���
			if (SOCKET_ERROR == val)
			{
				int nErrCode = WSAGetLastError();
				if (nErrCode == WSAEWOULDBLOCK)//�������ݻ�����������
				{
					continue;
				}else if ( WSAENETDOWN == nErrCode ||
						  WSAETIMEDOUT == nErrCode ||
						  WSAECONNRESET == nErrCode)//�ͻ��˹ر�������
				{
					//�뿪�ٽ���
					LeaveCriticalSection(&pClient->m_cs);

					pClient->m_bConning = FALSE;	//���ӶϿ�
//					pClient->m_bExit = TRUE;		//�߳��˳�
					pClient->m_bSend = FALSE;
					break;
				}else {
					//�뿪�ٽ���
					LeaveCriticalSection(&pClient->m_cs);
					pClient->m_bConning = FALSE;	//���ӶϿ�
//					pClient->m_bExit = TRUE;		//�߳��˳�
					pClient->m_bSend = FALSE;
					break;
				}
			}
			//�ɹ���������
			//�뿪�ٽ���
			memset(dataBuf, 0, MAX_NUM_BUF);		//��ս��ջ�����
			LeaveCriticalSection(&pClient->m_cs);
			//�����¼�Ϊ���ź�״̬
			pClient->m_bSend = FALSE;
			bSend = FALSE;
		}

	}

	return 0;
}

