/**
 * Copyright (c) 2018 Vertexcom Technologies, Inc. All rights reserved
 * Vertexcom Confidential Proprietary
 *
*/
#include <stdio.h>
#include "contiki.h"

#if defined(CONFIG_DEBUG_SEND_UDP_APP) && !defined(CONFIG_WISUN_ROOT)
#include "net/ip/uip.h"
#include "net/ip/udp-socket.h"
#include "net/ip/uip.h"
#include "net/wisun/wisun-state.h"

static struct udp_socket udp_fd;  //udp-socket  udp-socket.c
#define UDP_PORT 5555
static struct ctimer udp_timer;   //ctimer ctimer.h
static int m_udpIntvl=15;

static void udp_socket_input_callback(
    struct udp_socket *c,   //udp-socket  udp-socket.c
    void *ptr,
    const uip_ipaddr_t *source_addr,	  //uip_ipaddr_t   uip.h
    uint16_t source_port,
    const uip_ipaddr_t *dest_addr,	  //uip_ipaddr_t   uip.h
    uint16_t dest_port,
    const uint8_t *data,
    uint16_t datalen)
{
    int i;
    printf("udp_socket_input_callback datalen=%u\n", datalen);
    for (i = 0; i < datalen; i++)
        printf("%c", data[i]);
    printf("\n");
}

static void udp_socket_period_send(void *ptr)
{
    static uint32_t s_cnt=0;
    printf("udp %u\n",++s_cnt);
    uint8_t test_str[]="hello";
    uip_ipaddr_t server_ip;	  //uip_ipaddr_t   uip.h
	uip_ip6addr(&server_ip, 0x2001, 0xdb8, 0, 0, 0, 0, 0, 0x0001);	  //uip_ip6addr   uip.h
    udp_socket_sendto(&udp_fd, test_str, strlen(test_str), &server_ip, UDP_PORT);   //udp_socket_sendto udp-socket.c
    ctimer_reset(&udp_timer);  //ctimer_reset  ctimer.c
}

void udpIntvl_set(int ms)
{
    m_udpIntvl=ms;
}

int udpIntvl_get()
{
    return m_udpIntvl;
}
#endif

/* Note: you can rename this process as you wish */
PROCESS(vertexcom_apps_test_process, "vertexcom_apps_test_process");	//PROCESS  vcapps.c
AUTOSTART_PROCESSES(&vertexcom_apps_test_process);				//AUTOSTART_PROCESSES  vcapps.c

PROCESS_THREAD(vertexcom_apps_test_process, ev, data)
{
    PROCESS_BEGIN();

    /* start your application here */
#if defined(CONFIG_DEBUG_SEND_UDP_APP) && !defined(CONFIG_WISUN_ROOT)
    extern int current_state;
    printf("started vertexcom apps\n");
    printf("waiting for PandSelectedState or IdleState...\n");
    while (1) {
        if (current_state==PanSelectedState||current_state==IdleState)
            break;
        PROCESS_WAIT_EVENT();
    }
    printf("state=%u\n",current_state);

    if (udp_socket_register(&udp_fd, NULL, udp_socket_input_callback) == -1) //udp_socket_register  udp-socket.c
        printf("udp_socket_register fail\n");
    if (udp_socket_bind(&udp_fd, UDP_PORT) == -1)  //udp_socket_bind udp-socket.c
        printf("udp_socket_bind fail\n");
    ctimer_set(&udp_timer, m_udpIntvl, udp_socket_period_send, NULL);  //ctimer_set  ctimer.c
#endif

    PROCESS_END();
}
