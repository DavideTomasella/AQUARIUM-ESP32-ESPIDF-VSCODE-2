#ifndef InOut_h
#define InOut_h

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

void UpdateINOUTData(bool dolog);
void SetINOUTData(void *args);

#endif