/**
 * Copyright (c) 2018 Vertexcom Technologies, Inc. All rights reserved
 * Vertexcom Confidential Proprietary
 *
*/
#include <stdio.h>
#include "contiki.h"
#include "net/ip/uip.h"
#include "net/ip/udp-socket.h"

static struct udp_socket udp_fd;  //udp-socket  udp-socket.c
#define UDP_PORT 5555
static struct ctimer udp_timer;   //ctimer   ctimer.h

static void udp_socket_input_callback(
    struct udp_socket *c,
    void *ptr,
    const uip_ipaddr_t *source_addr,  //uip_ipaddr_t   uip.h 
    uint16_t source_port,
    const uip_ipaddr_t *dest_addr,    //uip_ipaddr_t   uip.h 
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

#ifndef WISUN_ROOT
static void udp_socket_period_send(void *ptr)
{
    printf("udp_socket_send\n");
    uint8_t test_str[]="hello";
    uip_ipaddr_t server_ip;  //uip_ipaddr_t   uip.h 
	uip_ip6addr(&server_ip, 0x2001, 0xdb8, 0, 0, 0, 0, 0, 0x0001);  //uip_ip6addr  uip.h
    udp_socket_sendto(&udp_fd, test_str, strlen(test_str), &server_ip, UDP_PORT); //udp_socket_sendto   udp-socket.c
    ctimer_reset(&udp_timer);
}
#endif

/* Note: you can rename this process as you wish */
PROCESS(vertexcom_apps_test_process, "vertexcom apps test process");
AUTOSTART_PROCESSES(&vertexcom_apps_test_process);

PROCESS_THREAD(vertexcom_apps_test_process, ev, data)
{
    PROCESS_BEGIN();

    printf("started vertexcom apps\n");

    /* start your application here */

    if (udp_socket_register(&udp_fd, NULL, udp_socket_input_callback) == -1) //udp_socket_register  udp-socket.c
        printf("udp_socket_register fail\n");
    if (udp_socket_bind(&udp_fd, UDP_PORT) == -1)			//udp_socket_bind  udp-socket.c
        printf("udp_socket_bind fail\n");
#ifndef WISUN_ROOT
    ctimer_set(&udp_timer, 5 * CLOCK_SECOND, udp_socket_period_send, NULL);  //ctimer_set  ctimer.c
#endif
    PROCESS_END();
}
