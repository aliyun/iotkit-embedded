#include "infra_types.h"
#include "soc_api.h"

typedef struct {
  unsigned int end_time;
  bool over_flow;
}OsTimer;

void HAL_Printf(const char *fmt, ...);

unsigned int get_cur_time(void)
{
    unsigned int tick, cur_time;
    kal_get_time(&tick);
    cur_time = kal_ticks_to_milli_secs(tick);
    return cur_time;
}

void InitTimer(OsTimer* timer)
{
  timer->end_time = 0;
  timer->over_flow = false;
}

void countdown_ms(OsTimer* timer, unsigned int timeout)
{
  unsigned int current_time = get_cur_time();
  timer->end_time = current_time + timeout;
  if(timer->end_time < current_time) {
     timer->over_flow = true;
  }
}

char expired(OsTimer* timer)
{
  int left = 0;
  if (timer->over_flow) {
    left = 0xFFFFFFFF - get_cur_time() + timer->end_time;
  }
  else {
    left = timer->end_time - get_cur_time();
  }
  return (left < 0);
}

int HAL_TCP_Destroy(uintptr_t fd)
{
	HAL_Printf("tcp destroy,sock: %d\n",fd);
	if(-1 != fd)
		soc_close(fd);
    
	return 0;
}

uintptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
	int rc = -1, sock = 0;

    sock = soc_create(SOC_PF_INET, SOC_SOCK_STREAM, 0, MOD_MMI, gaccount_id);

	if (sock >= 0)
	{
		/*set the socket as no blocking*/
        kal_bool option = KAL_TRUE;
		kal_uint8 socket_opt = 1;
        rc=soc_setsockopt(n->sock, SOC_NBIO, &option, sizeof(option));
        HAL_Printf("soc_setsockopt return %d\n",rc);

		socket_opt = SOC_READ | SOC_WRITE | SOC_CONNECT | SOC_CLOSE;
		rc = soc_setsockopt(n->sock, SOC_ASYNC, &socket_opt, sizeof(socket_opt));
		if (rc < 0)
		{
			HAL_Printf("soc_setsockopt async return %d\n",rc);
			return -1;
		}
    }
	else{
        HAL_Printf("cmns create sock failed %d\n",n->sock);
        rc = -1;
	}
    return (uintptr_t)rc;
}

int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
	soc_fd_set readfds;
	OsTimer rtimer;
	int bytes = 0;
	int rc = 0;
    soc_timeval_struct tv;

	if(0 == timeout_ms){
		timeout_ms = 10;
	}
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    InitTimer(&rtimer);
    countdown_ms(&rtimer, timeout_ms);
    HAL_Printf("linkit_read,timeout is %d sec.%d us\n",tv.tv_sec,tv.tv_usec);
    HAL_Printf("linkit_read want to read %d\n", len);

	do{
		int slectrc=0;
		SOC_FD_ZERO(&readfds);
    	SOC_FD_SET(n->sock, &readfds);

		slectrc = soc_select(n->sock+1, &readfds, NULL, NULL, &tv);
        HAL_Printf("soc_select, return %d\n",slectrc);
		if(slectrc>=1 && SOC_FD_ISSET(n->sock, &readfds)){
            HAL_Printf("SOC_FD_ISSET\n");
			do{
				rc = soc_recv(n->sock, buffer+bytes, len-bytes, 0);
                HAL_Printf("soc_recv=%d\n",rc);
				if(rc > 0){
					bytes += rc;
				}
				else if(rc < 0){
					// SOC_WOULDBLOCK should be continue?
					if(SOC_WOULDBLOCK != rc){
						rc = -1;
					}
					else if(SOC_WOULDBLOCK == rc){
						if(bytes>0 && bytes<len){
	                        tv.tv_sec = timeout_ms / 1000;
	                        tv.tv_usec = (timeout_ms % 1000) * 1000;
							countdown_ms(&rtimer, timeout_ms);
						}
					}
					break;
				}
				else {// FIN from the peer side
					rc = -1;
					break;
				}
			}while((bytes<len) && !expired(&rtimer));
		}
		else if(0 == slectrc){
			if(bytes>0 && bytes<len){
				tv.tv_sec = timeout_ms / 1000;
				tv.tv_usec = (timeout_ms % 1000) * 1000;
				countdown_ms(&rtimer, timeout_ms);
				HAL_Printf("linkit_read soc read %d, wait remaining data\n", bytes);
			}
		}
		else{
			//soc_select return value < 0, it means connect is broken
			bytes = 0;
			break;
		}
		HAL_Printf("linkit_read soc already read %d\n", bytes);

		tv.tv_sec = timeout_ms / 1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;
	}while(!expired(&rtimer) && !(rc==-1) && (bytes<len));
	return bytes;
}

int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
	int rc = 0;
	soc_timeval_struct tv;
	soc_fd_set writefds;

	tv.tv_sec = 0;  /* 30 Secs Timeout */
	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors

	SOC_FD_ZERO(&writefds);
	SOC_FD_SET(fd, &writefds);
    HAL_Printf("send packet length is %d\n",len);
	if(soc_select(fd+1, NULL, &writefds, NULL, &tv) >= 0){
		if(SOC_FD_ISSET(fd, &writefds)){
			rc = soc_send(fd, buf, len, 0);
            HAL_Printf("send packet rc is %d\n",rc);
		}
	}
	HAL_Printf("send packet length result is %d\n",rc);
	return rc;
}