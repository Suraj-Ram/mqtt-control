#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <PubSubClient.h>

void debugPrint(const char *message, PubSubClient &mqtt_client);

#endif

