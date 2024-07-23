# RPIPicoWBattery
Demonstrate Battery Monitoring on the Raspberry Pi Pico W

Repo to support VLOG on my YouTube Channel  [@DrJonEA](https://youtube.com/@drjonea)

## Clone
Submodules are used so p lease close with --recurse-submodules or use git submodule update --init --recursive

## Dependencies
In additional to the libraries in the lib folder this example also uses code from the [Pico Examples project](https://github.com/raspberrypi/pico-examples).  This is linked using the environment variable PICO_EXAMPLES_PATH.

## Examples

Two examples both targetted for the Pico W

### Simple
This example is baremetal SDK only and with no IP stack. 

### WebService
This example pushes the readings every ten seconds to a web server via a web service call. 

The following environmental variables are used to take in the WIFI credentials 
+ WIFI_SSID
+  WIFI_PASSWORD



