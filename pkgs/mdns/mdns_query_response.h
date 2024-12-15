#ifndef MDNS_QUERY_RESPONSE_H
#define MDNS_QUERY_RESPONSE_H

#ifdef cplusplus
extern "C" {
#endif
int
send_mdns_query_service(char *service_name, char *result_buf, int result_len,
			int timeout_ms);
#ifdef cplusplus
  }
#endif

typedef struct query_response_s
{
  char host_address[128];
  int  host_port;
  int  has_txt;
  char txt_dict[256];
} query_response_t;
#endif
  
