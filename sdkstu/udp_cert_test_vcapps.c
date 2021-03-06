/**
 * Copyright (c) 2018 Vertexcom Technologies, Inc. All rights reserved
 * Vertexcom Confidential Proprietary
 *
*/
#include <stdio.h>
#include "contiki.h"
#include "net/ip/uip.h"
#include "net/ip/uiplib.h"
#include "net/ip/uip-debug.h"
#include "net/ip/udp-socket.h"
#include "rpl.h"
#include "shell.h"

static struct udp_socket udp_fd;  //udp-socket  udp-socket.c
static uint8_t test_pattern = 0;  
static uip_ipaddr_t remoteaddr;	  //uip_ipaddr_t   uip.h 
extern rpl_instance_t *default_instance;  //rpl_instance  rpl.h   rpl.c

#define UDP_PORT 25010
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])   //uip_udp_hdr  uip.h

static void udp_socket_input_callback(
    struct udp_socket *c, //udp-socket  udp-socket.c
    void *ptr,
    const uip_ipaddr_t *source_addr, //uip_ipaddr_t   uip.h 
    uint16_t source_port,
    const uip_ipaddr_t *dest_addr,//uip_ipaddr_t   uip.h 
    uint16_t dest_port,
    const uint8_t *data,
    uint16_t datalen)
{
    int i;
    printf("udp recv from ");
    uip_debug_ipaddr_print(source_addr);  //uip_debug_ipaddr_print  uip-debug.c
    printf(" %u bytes\n", datalen);
}

static void udp_socket_data_send(void)
{
    int i;
    uint8_t *wptr = (uint8_t *)UIP_UDP_BUF;
    for (i = 0; i < 1222; i++) {    //为什么是1222
        if (wptr > uip_buf + UIP_BUFSIZE){
            printf("%s: uip buffer overflow\n", __FUNCTION__);
        }
        *wptr++ = (i % 26) + 'a';   //
    }

    for (i = 0; i < 10; i++) {
        if (wptr > uip_buf + UIP_BUFSIZE){
            printf("%s: uip buffer overflow\n", __FUNCTION__);
        }
        *wptr++ = i + test_pattern;
    }

    printf("udp send to ");
    uip_debug_ipaddr_print(&remoteaddr);  //uip_debug_ipaddr_print  uip-debug.c
    printf(" 1232 bytes\n");
    udp_socket_sendto(&udp_fd, UIP_UDP_BUF, 1232, &remoteaddr, UDP_PORT); //udp_socket_sendto udp-socket.c
}

/* Note: you can rename this process as you wish */
PROCESS(vertexcom_apps_test_process, "vertexcom apps test process");	//PROCESS  vcapps.c
AUTOSTART_PROCESSES(&vertexcom_apps_test_process);				//AUTOSTART_PROCESSES  vcapps.c

SHELL_COMMAND(vcapps_command,    //SHELL_COMMAND  shell.h
	      "udp",
	      "udp",
	      &vertexcom_apps_test_process);   

void vcapps_shell_init(void)
{
    shell_register_command(&vcapps_command);	//shell_register_command  shell.c
}

PROCESS_THREAD(vertexcom_apps_test_process, ev, data)  //PROCESS_THREAD  vcapps.c
{
    const char *param, *nextptr = data;
    unsigned long value = 0;
    PROCESS_BEGIN();
    if ((param = shell_get_next_options(nextptr, &nextptr)) == NULL) {  //shell_get_next_options   shell.c
        vcapps_shell_init();
        if (udp_socket_register(&udp_fd, NULL, udp_socket_input_callback) == -1)   //udp_socket_register  udp-socket.c
            printf("udp_socket_register fail\n");
        if (udp_socket_bind(&udp_fd, UDP_PORT) == -1)      //udp_socket_bind udp-socket.c
            printf("udp_socket_bind fail\n");
        printf("udp register successfully\n");
        PROCESS_EXIT();
    }

    if (uiplib_ipaddrconv(param, &remoteaddr) == 1) {		//uiplib_ip6addrconv  uiplib.h
    } else if (default_instance && default_instance->current_dag && (value = shell_strtox(param, NULL)) != 0) {
        uip_ipaddr_copy(&remoteaddr, &default_instance->current_dag->dag_id);   //uip_ipaddr_copy  uip.h uip.c
        remoteaddr.u16[7] = UIP_HTONS(value);
    } else {
        goto err_out;
    }

    if ((param = shell_get_next_options(nextptr, &nextptr)) != NULL) {     //shell_get_next_options   shell.c
        if ((value = shell_strtox(param, NULL)) > 0xf0)    //shell_strtox    shell.c
            goto err_out;
        test_pattern = value;
        udp_socket_data_send();			//udp_socket_data_send  udp-socket.c
    } else {
err_out:
        printf("error: udp <ip> <pattern:0x20>\n");
    }
    PROCESS_END();
}
