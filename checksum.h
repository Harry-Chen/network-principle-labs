#ifndef __CHECK__
#define __CHECK__
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
 
struct _iphdr //定义IP首部 
{   
    unsigned char h_verlen; //4位首部长度+4位IP版本号 
    unsigned char tos; //8位服务类型TOS 
    unsigned short total_len; //16位总长度（字节） 
    unsigned short ident; //16位标识 
    unsigned short frag; //16位标志位 
    unsigned char ttl; //8位生存时间 TTL 
    unsigned char proto; //8位协议 (TCP, UDP 或其他) 
    unsigned short checksum; //16位IP首部校验和 
    unsigned int sourceIP; //32位源IP地址 
    unsigned int destIP; //32位目的IP地址 
};

int check_sum(unsigned short *iphd,int len,unsigned short checksum);
unsigned short count_check_sum(unsigned short *iphd);

#endif
