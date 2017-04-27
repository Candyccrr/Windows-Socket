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
#pragma comment(lib, "ws2_32.lib")			//动态库函数

//宏定义
#define SERVERPORT			5555			//服务器TCP端口
#define CONN_NUM            10              //连接客户端数量
#define TIMEFOR_THREAD_SLEEP		500		//等待客户端请求线程睡眠时间
#define WRITE_ALL   "all"                   //向所有数据发送数据
#define WRITE       "write"                 //发送标志
#define READ        "read"                  //接收显示标志
#define READ_ALL    "read all"              //接收所有客户端数据
typedef vector<CClient*> ClIENTVECTOR;		//向量容器
typedef vector<string> SVECTOR;             //内容字符

//函数申明
BOOL InitSever(void);                       //初始化
void InitMember(void);
bool InitSocket(void);						//初始化非阻塞套接字
void ExitServer(void);						//释放资源
bool StartService(void);					//启动服务器
void inAndOut(void);                        //处理数据
bool recvData(SOCKET s, char* buf);			//读取数据
bool sendData(SOCKET s, char* str);         //发送数据
bool RecvLine(SOCKET s, char* buf);         //读取一行数据
void  ShowServerStartMsg(BOOL bSuc);        //显示错误信息
void handleData(char* str);                 //数据处理
void ShowTipMsg(BOOL bFirstInput);          //显示输入提示信息
BOOL createCleanAndAcceptThread(void);      //开启监控函数
DWORD __stdcall AcceptThread(void* pParam); //开启客户端请求线程
DWORD __stdcall CleanThread(void* pParam);


#endif // SERVER_H_INCLUDED
