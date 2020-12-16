# IBM Watson IoT Platform packages

These packages create a sub-menu for Teltonika router to connect to IoT platform.

* Package 'libiot-watson-c' installs the nessecary libraries to install to OpenWRT router, based on https://github.com/ibm-watson-iot/iot-embeddedc
* Package 'luci_watson_web' installs a sub-menu in Teltonika router, used to write configuration data
* package 'luci_watson_web_prog' installs a C program used to connect to Watson IoT platform and it contains .init file used to turn on/off the program from the sub-menu from above

# Notes

* 'luci_watson_web_prog' package creates a new empty directory /etc/samples in the router and a file /etc/IoTFoundation.pem. The program at execution adds an enviromental variable named IOT_EMBDC_HOME which points to /etc directory. This is used by the installed libraries to get the IoTFoundation.pem file at the specified enviromental variable and searches for 'samples' directory which houses the configuration files.
