#ifndef HEADERS_H
#define HEADERS_H

#define ETH_OI_LEN 8
#define IP_OI_LEN 12
#define TCP_OI_LEN 4
#define UDP_OI_LEN 4
#define FTP_OI_LEN 8
#define TEL_OI_LEN 8
#define RDP_OI_LEN 12
#define DNS_OI_LEN 8

typedef struct {
  int hlp; 
  char other_info[ETH_OI_LEN]; 
  int len;
} Header;

typedef struct {
  int hlp; 
  char other_info[ETH_OI_LEN]; 
  int len;
} ethHeader;

typedef struct {
  int hlp; 
  char other_info[IP_OI_LEN]; 
  int len;
} ipHeader;

typedef struct {
  int hlp; 
  char other_info[TCP_OI_LEN]; 
  int len;
} tcpHeader;

typedef struct {
  int hlp; 
  char other_info[UDP_OI_LEN]; 
  int len;
} udpHeader;

typedef struct {
  int hlp; 
  char other_info[FTP_OI_LEN]; 
  int len;
} ftpHeader;

typedef struct {
  int hlp; 
  char other_info[TEL_OI_LEN]; 
  int len;
} telHeader;

typedef struct {
  int hlp; 
  char other_info[RDP_OI_LEN]; 
  int len;
} rdpHeader;

typedef struct {
  int hlp; 
  char other_info[DNS_OI_LEN]; 
  int len;
} dnsHeader;
#endif