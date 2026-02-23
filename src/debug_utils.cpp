#include "debug_utils.h"
#include <PubSubClient.h>
#include "config.h"

void debugPrint(const char *message, PubSubClient &mqtt_client)
{
    mqtt_client.publish(TOPIC_DEBUG_LOG, message);
}
