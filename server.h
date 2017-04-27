#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED
#include <iostream>
#include <winsock2.h>
#include <winbase.h>
#include <vector>
#include <map>
#include <string>
#include <process.h>
#include "sclient.h"

using namespace std;
#pragma comment(lib, "ws2_32.lib")			//��̬�⺯��

//�궨��
#define SERVERPORT			5555			//������TCP�˿�
#define CONN_NUM            10              //���ӿͻ�������
#define TIMEFOR_THREAD_SLEEP		500		//�ȴ��ͻ��������߳�˯��ʱ��
#define WRITE_ALL   "all"                   //���������ݷ�������
#define WRITE       "write"                 //���ͱ�־
#define READ        "read"                  //������ʾ��־
#define READ_ALL    "read all"              //�������пͻ�������
typedef vector<CClient*> ClIENTVECTOR;		//��������
typedef vector<string> SVECTOR;             //�����ַ�

//��������
BOOL InitSever(void);                       //��ʼ��
void InitMember(void);
bool InitSocket(void);						//��ʼ���������׽���
void ExitServer(void);						//�ͷ���Դ
bool StartService(void);					//����������
void inAndOut(void);                        //��������
bool recvData(SOCKET s, char* buf);			//��ȡ����
bool sendData(SOCKET s, char* str);         //��������
bool RecvLine(SOCKET s, char* buf);         //��ȡһ������
void  ShowServerStartMsg(BOOL bSuc);        //��ʾ������Ϣ
void handleData(char* str);                 //���ݴ���
void ShowTipMsg(BOOL bFirstInput);          //��ʾ������ʾ��Ϣ
BOOL createCleanAndAcceptThread(void);      //������غ���
DWORD __stdcall AcceptThread(void* pParam); //�����ͻ��������߳�
DWORD __stdcall CleanThread(void* pParam);


#endif // SERVER_H_INCLUDED
