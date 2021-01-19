#include <string.h>
#include <uci.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "deviceclient.h"

#define DATA_PATH "/etc/samples/watson.cfg"

static const char *delimiter = " ";
volatile int interrupt = 0;

/* handle signals */
void sigHandler(int signo) {
	printf("SigINT received.\n");
	interrupt = 1;
}

/* connect to IBM Watson cloud using a configuration file */
int connectToWatson()
{
    int rc = -1;

	iotfclient  client;

	//catch interrupt signal
	signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

	char* configFilePath;

    
	if(isEMBDCHomeDefined()){

	    getSamplesPath(&configFilePath);
	    configFilePath = realloc(configFilePath,strlen(configFilePath)+15);
	    strcat(configFilePath,"watson.cfg");
        }
	else{
	    printf("IOT_EMBDC_HOME is not defined\n");
	    printf("Define IOT_EMBDC_HOME to client library path to execute samples\n");
	    return -1;
    }

	rc = initialize_configfile(&client, configFilePath,0);
	free(configFilePath);

	if(rc != SUCCESS){
		printf("initialize failed and returned rc = %d.\n Quitting..", rc);
		return 0;
	}

	//unsigned short interval = 59;
	//setKeepAliveInterval(interval);

	rc = connectiotf(&client);

	if(rc != SUCCESS){
		printf("Connection failed and returned rc = %d.\n Quitting..", rc);
		return 0;
	}

	if(!client.isQuickstart){
	    subscribeCommands(&client);
	    setCommandHandler(&client, myCallback);
        }

	while(!interrupt)
	{
		printf("Publishing the event stat with rc ");
		rc= publishEvent(&client, "status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0);
		printf(" %d\n", rc);
		yield(&client,1000);
		sleep(2);
	}

	printf("Quitting!!\n");

	disconnect(&client);
	return 0;
}

int main(int argc, char * argv[])
{
    putenv("IOT_EMBDC_HOME=/etc");
    configureFile();
    connectToWatson();
    return 0;
}
