
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config_local.h"

#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwipopts.h"

#include "config_apps.h"
#include "MessageLoop.h"

extern "C"
{
	#include "sleep.h"
	#include "pulse_controller.h"
}

u16_t ionizer_port = 6007;

void print_app_header()
{
	xil_printf("%20s port: %6d\n\r", "IonizerES server", ionizer_port);
}

void init_system()
{
	//Initial DDS frequencies.  These should be good defaults.
	unsigned ftw0[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned tOK = 0;
	unsigned i, j;

#ifdef CONFIG_HG
	xil_printf("Mercurizer version 0.1 starting up...\r\n");
#endif

#ifdef CONFIG_Al
	xil_printf("Aluminizer version 2.0 starting up...\r\n");

	//221 MHz for pre-cooling DDS (0) to keep the ions cold
	//170 MHz for detection DDS (3) to keep AOM warm
	ftw0[0] = 949187772;
	ftw0[3] = 730144440;
#endif

	printf("Initializing pulse controller...\r\n");
	PULSE_CONTROLLER_init(pulser, NDDS);
	printf("Initializing pulse controller...done.\r\n");

	PULSE_CONTROLLER_clear_timing_check(pulser);
	printf("tOK = %u\r\n", PULSE_CONTROLLER_timing_ok(pulser));

	PULSE_CONTROLLER_enable_timing_check(pulser);

	for (i = 0; i < 10; i++)
	{
		for (j = 0; j < NDDS; j++)
			PULSE_CONTROLLER_set_dds_freq(pulser, j, ftw0[j]);

		usleep(100000);
		tOK += PULSE_CONTROLLER_timing_ok(pulser);
	}

	PULSE_CONTROLLER_disable_timing_check(pulser);

	printf("tOK = %u\r\n", tOK);
}

void ionizer_application_thread(void*)
{
	int sock, new_sd;
	sockaddr_in address, remote;
	int size;

	print_app_header();

	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return;

	address.sin_family = AF_INET;
	address.sin_port = htons(ionizer_port);
	address.sin_addr.s_addr = INADDR_ANY;

	if (lwip_bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
		return;

	lwip_listen(sock, 5);

	size = sizeof(remote);

	init_system();

	init_remote_interfaces();

	while (1)
	{

		xil_printf("listening...\r\n");

		if ((new_sd = lwip_accept(sock, (struct sockaddr *)&remote, (socklen_t *)&size)) > 0)
			message_loop(new_sd);

	}
}
