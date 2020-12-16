#include <uci.h>
#include <signal.h>
#include <stdlib.h>
#include "deviceclient.h"

#define DATA_PATH "/etc/samples/watson.cfg"

static const char *delimiter = " ";
volatile int interrupt = 0;

/* handle signals */
void sigHandler(int signo) {
	interrupt = 1;
}

/* append a new entry and value to specified path */
int appendToFile(const char *path, const char *entry, char *value)
{
    FILE *fPtr;

    fPtr = fopen(path, "a");

    /* fopen() return NULL if unable to open file in given mode. */
    if (fPtr == NULL)
    {
        /* Unable to open file hence exit */
        exit(EXIT_FAILURE);
    }

    fputs(entry, fPtr);
    fputs(value, fPtr);
    fputs("\n", fPtr);

    fclose(fPtr);
    return 0;
}

/* gets the string value of uci_option variable */ 
void uci_show_value(struct uci_option *o, const char *entry)
{
    struct uci_element *e;
    bool sep = false;

    switch(o->type) {
    case UCI_TYPE_STRING:
        //printf("%s\n", o->v.string);
        appendToFile(DATA_PATH, entry, o->v.string); /** This is the actual config value that gets passed */
        break;
    case UCI_TYPE_LIST:
        uci_foreach_element(&o->v.list, e) {
            printf("%s%s", (sep ? delimiter : ""), e->name);
            sep = true;
        }
        printf("\n");
        break;
    default:
        printf("<unknown>\n");
        break;
    }
}

/* Get the specified UCI configuration file entry.

   This could use some improvements because it passes an entry variable
   which is used to appent to the configuration file. Not only that but
   the passed variable isn't used anywhere except passing it onto another 
   method so this part is smelly. A better way would be to return the value
   which was read and use it elsewhere. */
int show_config_entry(char *path, const char *entry)
{
    char * path_processed = strdup(path);

    struct uci_context *c;
    struct uci_ptr ptr;

    c = uci_alloc_context();
    if (uci_lookup_ptr (c, &ptr, path_processed, true) != UCI_OK)
    {
        return 1;
    }

    uci_show_value(ptr.o, entry);
    uci_free_context(c);
    return 0;
}

/* create a configuration file at DATA_PATH 
used to initialise IBM Watson program */
int configureFile(){
    remove(DATA_PATH);
    show_config_entry("watson.watson_sct.orgId", "org=");
    show_config_entry("watson.watson_sct.deviceType", "type=");
    show_config_entry("watson.watson_sct.deviceId", "id=");
    show_config_entry("watson.watson_sct.token", "auth-token=");
    appendToFile(DATA_PATH, "domain=", "");
    appendToFile(DATA_PATH, "auth-method=", "token");
    appendToFile(DATA_PATH, "serverCertPath=", "");
    appendToFile(DATA_PATH, "useClientCertificates=", "0");
    appendToFile(DATA_PATH, "rootCACertPath=", "NULL");
    appendToFile(DATA_PATH, "clientCertPath=", "NULL");
    appendToFile(DATA_PATH, "clientKeyPath=", "NULL");

    return 0;
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
	    strcat(configFilePath,"watson.cfg"); /** Hardcoded part which is always being searched in IOT_EMBDC_HOME/samples which should be overall improved */
        }
	else{
	    return -1; /** Enviromental variable IOT_EMBDC_HOME is not defined */
    }

	rc = initialize_configfile(&client, configFilePath,0);
	free(configFilePath);

	if(rc != SUCCESS){
		return 0;
	}

	rc = connectiotf(&client);

	if(rc != SUCCESS){
		return 0; 
	}

    /** This is where the publishing part is, also hardcoded part because it publishes only a value of 32, place for improvement */
	while(!interrupt)
	{
		rc= publishEvent(&client, "status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0);
		yield(&client,1000);
		sleep(2);
	}

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
