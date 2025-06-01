// config.h
#ifndef CONFIG_H
#define CONFIG_H

extern "C" char* sbrk(int incr);

extern bool re_setup;
//Konifguration
extern bool USBDRIVE;
extern bool wlan;
extern bool fixed_wifi;
extern bool enable_login;
extern bool startup_config_enabled;

#endif
