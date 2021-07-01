/*
*   Subscription example.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MQTTAsync.h"

#define ADDRESS "ssl://13.65.102.222:8883"
#define CLIENTID "Test2"
#define TOPIC "test/topic"
#define PAYLOAD "Hello"
#define QOS 1
#define TIMEOUT 10000L

volatile MQTTAsync_token deliveredToken;

int disc_finished = 0;
int subscribed = 0;
int finished = 0;

void connlost(void *context, char *cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    printf("\nConnection lost.\n");
    printf("\tCause: %s.\n", cause);
    printf("Reconnecting.\n");

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}


int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int i;
    char *payloadptr;

    printf("Message arrived. Topic: %s. Message:\n", topicName);

    payloadptr = message->payload;
    for (i = 0; i < message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void onDisconnect(void *context, MQTTAsync_successData *response)
{
    printf("Disconnection successful\n");
    disc_finished = 1;
}

void onSubscribe(void *context, MQTTAsync_successData *respones)
{
    printf("Subscribe succeded.\n");
    subscribed = 1;
}

void onSubscribeFailure(void *context, MQTTAsync_failureData *response)
{
    printf("Subscribed failed, reason: %d.\n", response ? response->code : 0);
    finished = 1;
}

void onConnectFailure(void *context, MQTTAsync_failureData *response)
{
    printf("Connection failed, reason: %d.\n", response ? response->code : 0);
    finished = 1;
}

void onConnect(void *context, MQTTAsync_successData *response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_callOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    printf("Connection successful.\n");
    printf("Subscribing to topic %s.\n for client %s using QoS%d.\n\n Press Q<Enter> to exit.\n", TOPIC, CLIENTID, QOS);
    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;

    deliveredToken = 0;
    if ((rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to subscribe, return code %d.\n", rc);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_token token;
    int rc;
    int ch;

    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(client, NULL, connlost, msgarrvd, NULL);

    /* Configure SSL client options */
    ssl_opts.enableServerCertAuth = 0;
    ssl_opts.enabledGroups = "kyber512"; /* Key exchange */
    ssl_opts.CApath = "/home/jash/Documentos/Maestria/certs/azure2/ca.crt";
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    conn_opts.ssl = &ssl_opts;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to connect, return code: %d.\n", rc);
        exit(EXIT_FAILURE);
    }
    
    while (!subscribed)
    {
        sleep(10);
    }
    if (finished)
        goto exit;

    do
    {
        ch = getchar();
    } while (ch != 'Q' && ch != 'q');

    disc_opts.onSuccess = onDisconnect;
    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to disconnect, return code: %d.\n", rc);
        exit(EXIT_FAILURE);
    }
    while (!disc_finished)
        sleep(10);

exit:
    MQTTAsync_destroy(&client);
    return rc;
}