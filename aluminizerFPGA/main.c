/*
 * Copyright (c) 2007-2009 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>

#include "xmk.h"
#include "xenv_standalone.h"
#include "xparameters.h"

/* ethernet controller base address */
#define PLATFORM_EMAC_BASEADDR XPAR_LLTEMAC_0_BASEADDR

#include "platform.h"

#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/init.h"
#include "netif/xadapter.h"

#include "config_apps.h"


extern void ionizer_application_thread(void*);

void print_ip(char *msg, struct ip_addr *ip) 
{
    print(msg);
    xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip), 
            ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw)
{

    print_ip("Board IP: ", ip);
    print_ip("Netmask : ", mask);
    print_ip("Gateway : ", gw);
}

int main()
{
	xil_printf("\r\n\r\nInitializing platform...\r\n");
    if (init_platform() < 0) {
        xil_printf("ERROR initializing platform.\n\r");
        return -1;
    }

    xil_printf("Starting kernel...\r\n");
    
    /* start the kernel - does not return */
    xilkernel_main();

    return 0;
}

struct netif server_netif;

void network_thread(void* p)
{
    struct netif *netif;
    struct ip_addr ipaddr, netmask, gw;

    /* the mac address of the board. this should be unique per board */
    unsigned char mac_ethernet_address[] = { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

    netif = &server_netif;

    /* initialize IP addresses to be used */
    IP4_ADDR(&ipaddr,  192, 168,   1, 10);
    IP4_ADDR(&netmask, 255, 255, 255,  0);
    IP4_ADDR(&gw,      192, 168,   1,  1);

    /* print out IP settings of the board */
    xil_printf("\n\r\n\r");
    xil_printf("-----lwIP Socket Application ------\n\r");
    print_ip_settings(&ipaddr, &netmask, &gw);

    /* Add network interface to the netif_list, and set it as default */
    if (!xemac_add(netif, &ipaddr, &netmask, &gw, mac_ethernet_address, PLATFORM_EMAC_BASEADDR)) {
        xil_printf("Error adding N/W interface\n\r");
        return;
    }
    netif_set_default(netif);

    /* specify that the network if is up */
    netif_set_up(netif);

    /* start packet receive thread - required for lwIP operation */
    sys_thread_new("xemacif_input_thread", (void(*)(void*))xemacif_input_thread, netif, 
            THREAD_STACKSIZE, 
            DEFAULT_THREAD_PRIO);

    /* now we can start application threads */
    /* start ionizer server thread */
    sys_thread_new("ionizer", ionizer_application_thread, 0, 
            THREAD_STACKSIZE,
            DEFAULT_THREAD_PRIO);

    return;
}



#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/
int main_thread()
{
	xil_printf("\033[H\033[J"); //clears the screen
	xil_printf("Entered main thread...\r\n");
	xil_printf("Initializing lwIP...\r\n");
	
    /* initialize lwIP before calling sys_thread_new */
    lwip_init();

	xil_printf("Starting network thread...\r\n");
    /* any thread using lwIP should be created using sys_thread_new */
    sys_thread_new("NW_THREAD", network_thread, NULL,
            THREAD_STACKSIZE,
            DEFAULT_THREAD_PRIO);

    return 0;
}
#if defined(__cplusplus)
}
#endif /* __cplusplus */
