#include "rip.h"

TRtEntry *g_pstRouteEntry = NULL;

char *pcLocalAddr[10]={};//存储本地接口ip地址
char *pcLocalName[10]={};//存储本地接口的接口名

void requestpkt_Encapsulate()
{
	//封装请求包  command =1,version =2,family =0,metric =16
	
}

/*****************************************************
*Func Name:    rippacket_Receive  
*Description:  接收rip报文
*Input:        
*	 
*Output: 
*
*Ret  ：
*
*******************************************************/
void rippacket_Receive()
{
	//接收ip设置

	//创建并绑定socket
	
	//把本地地址加入到组播中 	
	
	while(1)
	{

		//接收rip报文   存储接收源ip地址
		//判断command类型，request 或 response
		
	}
	
}


/*****************************************************
*Func Name:    rippacket_Send  
*Description:  向接收源发送响应报文
*Input:        
*	  1.stSourceIp    ：接收源的ip地址，用于发送目的ip设置
*Output: 
*
*Ret  ：
*
*******************************************************/
void rippacket_Send(struct in_addr stSourceIp)
{
	//本地ip设置

	//发送目的ip设置

	//创建并绑定socket

	//发送
	return;	
}

/*****************************************************
*Func Name:    rippacket_Multicast  
*Description:  组播请求报文
*Input:        
*	  1.pcLocalAddr   ：本地ip地址
*Output: 
*
*Ret  ：
*
*******************************************************/
void rippacket_Multicast(char *pcLocalAddr)
{
	//本地ip设置
	
	//目的ip设置


	//创建并绑定socket

	//发送
	return;
}

/*****************************************************
*Func Name:    request_Handle  
*Description:  响应request报文
*Input:        
*	  1.stSourceIp   ：接收源的ip地址
*Output: 
*
*Ret  ：
*
*******************************************************/
void request_Handle(struct in_addr stSourceIp)
{
	//处理request报文
	return;
}

/*****************************************************
*Func Name:    response_Handle  
*Description:  响应response报文
*Input:        
*	  1.stSourceIp   ：接收源的ip地址
*Output: 
*
*Ret  ：
*
*******************************************************/
void response_Handle(struct in_addr stSourceIp)
{
	//处理response报文
	return;
}

/*****************************************************
*Func Name:    route_SendForward  
*Description:  响应response报文
*Input:        
*	  1.uiCmd        ：插入命令
*	  2.pstRtEntry   ：路由信息
*Output: 
*
*Ret  ：
*
*******************************************************/
void route_SendForward(unsigned int uiCmd,TRtEntry *pstRtEntry)
{
	//建立tcp短连接，发送插入、删除路由表项信息到转发引擎
	return;
}

void ripdaemon_Start()
{
	//创建更新线程，30s更新一次

	//组播请求报文
    
	//接收rip报文

	return;
}

void routentry_Insert()
{
	//将本地路由添加到rip路由表里
	return ;
}

void localinterf_GetInfo()
{
	struct ifaddrs *pstIpAddrStruct = NULL;
	struct ifaddrs *pstIpAddrStCur  = NULL;
	void *pAddrPtr=NULL;
	const char *pcLo = "127.0.0.1";
	
	getifaddrs(&pstIpAddrStruct);
	pstIpAddrStCur = pstIpAddrStruct;
	
	int i = 0;
	while(pstIpAddrStruct != NULL)
	{
		if(pstIpAddrStruct->ifa_addr->sa_family==AF_INET)
		{
			pAddrPtr = &((struct sockaddr_in *)pstIpAddrStruct->ifa_addr)->sin_addr;
			char cAddrBuf[INET_ADDRSTRLEN];
			memset(&cAddrBuf,0,sizeof(INET_ADDRSTRLEN));
			inet_ntop(AF_INET, pAddrPtr, cAddrBuf, INET_ADDRSTRLEN);
			if(strcmp((const char*)&cAddrBuf,pcLo) != 0)
			{
				pcLocalAddr[i] = (char *)malloc(sizeof(INET_ADDRSTRLEN));
				pcLocalName[i] = (char *)malloc(sizeof(IF_NAMESIZE));
				strcpy(pcLocalAddr[i],(const char*)&cAddrBuf);
				strcpy(pcLocalName[i],(const char*)pstIpAddrStruct->ifa_name);
				i++;
			}	
		}
		pstIpAddrStruct = pstIpAddrStruct->ifa_next;
	}
	freeifaddrs(pstIpAddrStCur);
	return ;
}

int main(int argc,char* argv[])
{
	g_pstRouteEntry = (TRtEntry *)malloc(sizeof(TRtEntry));
	if(g_pstRouteEntry == NULL)
	{
		perror("g_pstRouteEntry malloc error !\n");
		return -1;
	}
	localinterf_GetInfo();
	routentry_Insert();
	ripdaemon_Start();
	return 0;
}

