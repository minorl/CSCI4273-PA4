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

// typedef struct {
//   int hlp; 
//   char other_info[ETH_OI_LEN]; 
//   size_t len;
// } Header;

typedef struct __attribute__((__packed__))  {
  int hlp; 
  char other_info[ETH_OI_LEN]; 
  size_t len;
} ethHeader;

typedef struct __attribute__((__packed__))  {
  int hlp; 
  char other_info[IP_OI_LEN]; 
  size_t len;
} ipHeader;

typedef struct {
  int hlp; 
  char other_info[TCP_OI_LEN]; 
  size_t len;
} tcpHeader;

typedef struct __attribute__((__packed__))  {
  int hlp; 
  char other_info[UDP_OI_LEN]; 
  size_t len;
} udpHeader;

typedef struct __attribute__((__packed__))  {
  int hlp; 
  char other_info[FTP_OI_LEN]; 
  size_t len;
} ftpHeader;

typedef struct __attribute__((__packed__))  {
  int hlp; 
  char other_info[TEL_OI_LEN]; 
  size_t len;
} telHeader;

typedef struct __attribute__((__packed__))  {
  int hlp; 
  char other_info[RDP_OI_LEN]; 
  size_t len;
} rdpHeader;

typedef struct __attribute__((__packed__))  {
  int hlp; 
  char other_info[DNS_OI_LEN]; 
  size_t len;
} dnsHeader;
#endif