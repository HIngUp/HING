/**
 * Copyright (c) 2018 Vertexcom Technologies, Inc. All rights reserved
 * Vertexcom Confidential Proprietary
 *
*/
#include <stdio.h>
#include "contiki.h"
#include "contiki.h"
#include "contiki-net.h"
#include "shell.h"

#define SERVER1_PORT 5680
#define SERVER2_PORT 5681
static struct etimer sock_timer;   //etimer  etimer.h

#ifdef WISUN_ROOT
static struct uip_udp_conn *server1_conn = NULL;  //uip_udp_conn  uip.h
static struct uip_udp_conn *server2_conn = NULL;  //uip_udp_conn  uip.h
#else
static struct uip_udp_conn *client_conn = NULL;  //uip_udp_conn  uip.h

static void udp_socket_period_send(void)
{
    uint8_t test_str1[]="hello";
    uint8_t test_str2[]="hello2";
    uip_ipaddr_t server_ip;				 //uip_ipaddr_t   uip.h 
	uip_ip6addr(&server_ip, 0x2001, 0xdb8, 0, 0, 0, 0, 0, 0x0001);   //uip_ip6addr   uip.h

    printf("vcapps: send msg to port %u (%d bytes)\n", SERVER1_PORT, strlen(test_str1));
    uip_udp_packet_sendto(client_conn, test_str1, strlen(test_str1), &server_ip, uip_htons(SERVER1_PORT));  //uip_udp_packet_sendto uip-udp-packet.c

    printf("vcapps: send msg to port %u (%d bytes)\n", SERVER2_PORT, strlen(test_str2));
    uip_udp_packet_sendto(client_conn, test_str2, strlen(test_str2), &server_ip, uip_htons(SERVER2_PORT));   //uip_udp_packet_sendto uip-udp-packet.c
    etimer_reset(&sock_timer);     //etimer_reset etimer.c
}
#endif

#define UIP_UDP_BUF ((struct uip_udp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])    //uip_udp_hdr  uip.h

static void print_uip_udp_conn()
{
    int i;
    for(i = 0; i < UIP_UDP_CONNS; i++) {
        if (uip_udp_conns[i].lport)
            printf("[%d][%02X%02X:%u -> :%u][%s]\n", i, uip_udp_conns[i].ripaddr.u8[14],uip_udp_conns[i].ripaddr.u8[15],
            uip_htons(uip_udp_conns[i].rport), uip_htons(uip_udp_conns[i].lport), PROCESS_NAME_STRING(uip_udp_conns[i].appstate.p));
    }    //uip_ntohs  uip.c uip.h
}


/* Note: you can rename this process as you wish */
PROCESS(vertexcom_apps_test_shell_process, "vcapps shell");	  //PROCESS  vcapps.c

SHELL_COMMAND(vcapps_command,    //SHELL_COMMAND  shell.h
	      "vcapps",
	      "vcapps",
	      &vertexcom_apps_test_shell_process);

void vcapps_shell_init(void)
{
    shell_register_command(&vcapps_command);	//shell_register_command  shell.c
}

PROCESS_THREAD(vertexcom_apps_test_shell_process, ev, data) //PROCESS_THREAD  vcapps.c
{
    const char *param, *nextptr = data;
    PROCESS_BEGIN();
    if ((param = shell_get_next_options(nextptr, &nextptr)) != NULL) {
        if (strcmp(param, "udp") == 0) {
            print_uip_udp_conn();
        }
    }
    PROCESS_END();
}

PROCESS(vertexcom_apps_test_process, "vertexcom apps test process");	  //PROCESS  vcapps.c
AUTOSTART_PROCESSES(&vertexcom_apps_test_process);			//AUTOSTART_PROCESSES  vcapps.c

PROCESS_THREAD(vertexcom_apps_test_process, ev, data) //PROCESS_THREAD  vcapps.c
{
    PROCESS_BEGIN();
    vcapps_shell_init();
#ifdef WISUN_ROOT
    if ((server1_conn = udp_new(NULL, 0, NULL)) == NULL) {
        printf("vcapps: No UDP connection available, exiting the process!\n");
        PROCESS_EXIT();
    }
    udp_bind(server1_conn, uip_htons(SERVER1_PORT));     //udp_bind  NFOUND
    printf("vcapps: Listening on socket 1 port %u ...\n", uip_htons(server1_conn->lport));

    if ((server2_conn = udp_new(NULL, 0, NULL)) == NULL) {
        printf("vcapps: No UDP connection available, exiting the process!\n");
        PROCESS_EXIT();
    }
    udp_bind(server2_conn, uip_htons(SERVER2_PORT));
    printf("vcapps: Listening on socket 2 port %u ...\n", uip_htons(server2_conn->lport));
#else
    if ((client_conn = udp_new(NULL, 0, NULL)) == NULL) {
        printf("vcapps: No UDP connection available, exiting the process!\n");
        PROCESS_EXIT();
    }
    printf("vcapps: create client socket successfully\n");
    etimer_set(&sock_timer, 3000);
#endif
    while (1) {
        PROCESS_WAIT_EVENT();   //NFOUND
        if (ev == tcpip_event && uip_newdata()) {
            printf("vcapps: recv msg from port %u (%d bytes)\n", uip_htons(UIP_UDP_BUF->destport), uip_datalen());
            ((uint8_t *)uip_appdata)[uip_datalen()] = 0;
            printf("vcapps: recv [%s]\n", uip_appdata);
        } else if (ev == PROCESS_EVENT_TIMER && etimer_expired(&sock_timer)) {
#if !WISUN_ROOT
            udp_socket_period_send();
#endif
        }
    }

    PROCESS_END();
}
