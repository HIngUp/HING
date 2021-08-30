/**
 * Copyright (c) 2018 Vertexcom Technologies, Inc. All rights reserved
 * Vertexcom Confidential Proprietary
 *
*/
#include <stdio.h>
#include "contiki.h" 
#include "misc/sys_hook.h"

extern void uip_debug_ipaddr_print(const uip_ipaddr_t *addr);    //uip_debug_ipaddr_print uip-debug.c
#define PRINT6ADDR(addr) \
    do { \
        uip_debug_ipaddr_print(addr); \
    } while(0)

char* upgrade_result_table[] = {"OTA_START","OTA_SUCCESS","OTA_CRC_FAIL","OTA_BOARD_FAIL","OTA_CHIP_FAIL","OTA_ROLE_FAIL","OTA_STOP"};
char* upgrade_role_table[] = {"ROOT","NODE","LEAF"};
void upgrade_finish(uint8_t state, uint8_t type, uint32_t image_length, uint32_t header_length, uint32_t schedule_time){

    printf("upgrade_callback_fn state: %s, image role type: %d, image_length: %d, header_length: %d, schedule_time: %u\n",upgrade_result_table[state],upgrade_role_table[type],image_length,header_length,schedule_time);
}

void pc_callback(uint16_t pan_id, const char *network_name, const linkaddr_t *lladdr){

    printf("recv_pc_callback_fn: %d, %s\n",pan_id,network_name);
}

void after_getIp_fn(uip_ipaddr_t *getIp)  //uip_ipaddr_t   uip.h 
{
    
	extern void vc_net_service_init(void);   //NFOUND
	vc_net_service_init();
	
	printf("Get IP: ");
	PRINT6ADDR(getIp);
	printf("\n");
}

/* Note: you can rename this process as you wish */
PROCESS(vertexcom_apps_test_process, "vertexcom_apps_test_process");	//PROCESS  vcapps.c
AUTOSTART_PROCESSES(&vertexcom_apps_test_process);				//AUTOSTART_PROCESSES  vcapps.c

PROCESS_THREAD(vertexcom_apps_test_process, ev, data) //PROCESS_THREAD  vcapps.c
{
    PROCESS_BEGIN();

    /* start your application here */
	printf("!!! vertexcom_apps_test_process\n");
#ifdef WISUN_ROOT
    SYS_HOOK.after_dhcpc_getIp_callback_fn = after_getIp_fn;
#endif
	SYS_HOOK.upgrade_callback_fn = upgrade_finish;
    SYS_HOOK.recv_pc_callback_fn = pc_callback;

    PROCESS_END();
}
