/***
 * main.cpp - HTTP Get over socket
 * Send Battery data to web server
 * Jon Durrant
 * 22-Jul-2024
 *
 *
 */

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "WifiHelper.h"
#include "Request.h"

#include "hardware/adc.h"
extern "C" {
#include <power_status.h>
}


#ifndef BATTERY_URL
#define BATTERY_URL  "http://vmu22a.local.jondurrant.com:5000/args"
#endif




//Check these definitions where added from the makefile
#ifndef WIFI_SSID
#error "WIFI_SSID not defined"
#endif
#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD not defined"
#endif

#define TASK_PRIORITY     ( tskIDLE_PRIORITY + 1UL )
#define BUF_LEN					2048


void runTimeStats(){
  TaskStatus_t         * pxTaskStatusArray;
  volatile UBaseType_t uxArraySize, x;
  unsigned long        ulTotalRunTime;


  /* Take a snapshot of the number of tasks in case it changes while this
  function is executing. */
  uxArraySize = uxTaskGetNumberOfTasks();
  printf("Number of tasks %d\n", uxArraySize);

  /* Allocate a TaskStatus_t structure for each task.  An array could be
  allocated statically at compile time. */
  pxTaskStatusArray = (TaskStatus_t*) pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

  if (pxTaskStatusArray != NULL){
    /* Generate raw status information about each task. */
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray,
                                       uxArraySize,
                                       &ulTotalRunTime);



    /* For each populated position in the pxTaskStatusArray array,
    format the raw data as human readable ASCII data. */
    for (x = 0; x < uxArraySize; x++){
      printf("Task: %d \t cPri:%d \t bPri:%d \t hw:%d \t%s\n",
             pxTaskStatusArray[x].xTaskNumber,
             pxTaskStatusArray[x].uxCurrentPriority,
             pxTaskStatusArray[x].uxBasePriority,
             pxTaskStatusArray[x].usStackHighWaterMark,
             pxTaskStatusArray[x].pcTaskName
      );
    }


    /* The array is no longer needed, free the memory it consumes. */
    vPortFree(pxTaskStatusArray);
  } else{
    printf("Failed to allocate space for stats\n");
  }

  HeapStats_t heapStats;
  vPortGetHeapStats(&heapStats);
  printf("HEAP avl: %d, blocks %d, alloc: %d, free: %d\n",
         heapStats.xAvailableHeapSpaceInBytes,
         heapStats.xNumberOfFreeBlocks,
         heapStats.xNumberOfSuccessfulAllocations,
         heapStats.xNumberOfSuccessfulFrees
  );

}




void debugCB(const int logLevel, const char *const logMessage){
	printf("WOLFSSL DEBUG(%d): %s\n", logLevel, logMessage);
}




char UserBuf[BUF_LEN];
Request req((char *)UserBuf, BUF_LEN);

void sendBattery(){
	bool vsys;
	float volts;
	bool res;
	const char URL[] = BATTERY_URL;
	char time[10];
	char vol[10];

	const char * source = "VBUS";
	if (power_source(&vsys) == PICO_OK) {
		if (vsys){
			source = "VSYS";
		} else {
			source = "VBUS";
		}
	}
	int voltage_return = power_voltage(&volts);
	uint32_t now = to_ms_since_boot (get_absolute_time ()) / 1000;
	sprintf(time, "%u", now );
	sprintf(vol,"%.2f", volts);

	std::map<std::string, std::string> query;
	query["time"]=time;
	query["power"]=source;
	query["volts"]=vol;


	printf("Sending %u, %s, %.2f\n",
			now,
			source,
			volts);
	res = req.get(URL, &query);
	if ( res ){
		res = (req.getStatusCode() == 200);
	}
	if (res){
		printf("Result: %.*s\n", req.getPayloadLen(), req.getPayload());
	} else {
		printf("Request failed %d\n", req.getStatusCode());
	}

}


void main_task(void* params){

	printf("Main task started\n");


	if (WifiHelper::init()){
	printf("Wifi Controller Initialised\n");
	} else {
	printf("Failed to initialise controller\n");
	return;
	}


	printf("Connecting to WiFi... %s \n", WIFI_SSID);

	if (WifiHelper::join(WIFI_SSID, WIFI_PASSWORD)){
	printf("Connect to Wifi\n");
	}
	else {
	printf("Failed to connect to Wifi \n");
	}


	//Print MAC Address
	char macStr[20];
	WifiHelper::getMACAddressStr(macStr);
	printf("MAC ADDRESS: %s\n", macStr);

	//Print IP Address
	char ipStr[20];
	WifiHelper::getIPAddressStr(ipStr);
	printf("IP ADDRESS: %s\n", ipStr);



	for (;;){


		sendBattery();
		//runTimeStats();

		vTaskDelay(10000);


		if (!WifiHelper::isJoined()){
		  printf("AP Link is down\n");

		  if (WifiHelper::join(WIFI_SSID, WIFI_PASSWORD)){
			printf("Connect to Wifi\n");
		  } else {
			printf("Failed to connect to Wifi \n");
		  }
		}

	}

}


void vLaunch(void) {
  TaskHandle_t task;

  xTaskCreate(main_task, "MainThread", 20000, NULL, TASK_PRIORITY, &task);

  /* Start the tasks and timer running. */
  vTaskStartScheduler();
}


int main(void) {
  stdio_init_all();
  adc_init();
  sleep_ms(2000);
  printf("GO\n");


  vLaunch();

  return 0;
}
