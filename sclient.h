#ifndef SCLIENT_H_INCLUDED
#define SCLIENT_H_INCLUDED
#include <winsock2.h>

#include <iostream>
using namespace std;


#define TIMEFOR_THREAD_CLIENT		500		//�߳�˯��ʱ��

#define	MAX_NUM_CLIENT		10				//���ܵĿͻ��������������
#define	MAX_NUM_BUF			64				//����������󳤶�
#define INVALID_OPERATOR	1				//��Ч�Ĳ�����
#define INVALID_NUM			2				//��ĸΪ��
#define ZERO				0				//��


//���ݰ�ͷ�ṹ���ýṹ��win32��Ϊ4byte
typedef struct _head
{
	char			type;	//����
	unsigned short	len;	//���ݰ��ĳ���(����ͷ�ĳ���)
}hdr, *phdr;

//���ݰ��е����ݽṹ
typedef struct _data
{
	char	buf[MAX_NUM_BUF];//����
}DATABUF, *pDataBuf;


class CClient
{
public:
	CClient(const SOCKET sClient,const sockaddr_in &addrClient);
	virtual ~CClient();

public:
	BOOL		StartRuning(void);					//�������ͺͽ��������߳�
	void		HandleData(const char* pExpr);		//������ʽ
	BOOL		IsConning(void){					//�Ƿ����Ӵ���
				return m_bConning;
				}
	void		DisConning(void){					//�Ͽ���ͻ��˵�����
				m_bConning = FALSE;
				}
	BOOL		IsExit(void){						//���պͷ����߳��Ƿ��Ѿ��˳�
				return m_bExit;
				}
    BOOL		IsSend(void){
				m_bSend = TRUE;
				}

public:
	static DWORD __stdcall	 RecvDataThread(void* pParam);		//���տͻ�������
	static DWORD __stdcall	 SendDataThread(void* pParam);		//��ͻ��˷�������

private:
	CClient();
private:
	SOCKET		m_socket;			//�׽���
	sockaddr_in	m_addr;				//��ַ
	DATABUF		m_data;				//����
	HANDLE		m_hEvent;			//�¼�����
	HANDLE		m_hThreadSend;		//���������߳̾��
	HANDLE		m_hThreadRecv;		//���������߳̾��
	CRITICAL_SECTION m_cs;			//�ٽ�������
	BOOL		m_bConning;			//�ͻ�������״̬
	BOOL        m_bSend;            //���ݷ���״̬
	BOOL		m_bExit;			//�߳��˳�
};


#endif // SCLIENT_H_INCLUDED
