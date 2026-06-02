#ifndef MODEMS_QUECTEL_EG06_H
#define MODEMS_QUECTEL_EG06_H

#include <transport.h>
#include <cjson/cJSON.h>
#include <modem.h>

#define SCANF_BUF_SIZE 128
#define OPERATOR_ID_SIZE 17
#define APN_BUF_SIZE 128
#define STATE_BUF_SIZE 17
#define GENERATION_BUF_SIZE 17
#define RAT_BUF_SIZE 8
#define ID_BUF_SIZE 32

extern struct modem quectel_eg06;

#endif