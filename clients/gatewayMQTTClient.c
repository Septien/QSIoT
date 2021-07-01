#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <pthread.h>
#include <signal.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <errno.h>

struct clientInfo {
    char *address;
    char *clientid;
    char *topic;
    char *payload;
    int qos;
    unsigned long int timeout;
};

/* Global variables and mutex */
char data[15];                 // Recieved data from Arduino
int withData = 0;               // Does the array has data?
int out = 0;                    // Exit thread
pthread_mutex_t dataMutex;      // Set data mutex.
pthread_mutex_t withDataM;       // Acces data availability
pthread_mutex_t outMutex;       // Out mutex
int finishP = 0;
volatile MQTTClient_deliveryToken deliveredToken;

/* Handle keyboard interruption Ctrl+C */
void handles(int sig)
{
    printf("Finishing program.\n");
    finishP = 1;
}

void *readArduino(void *dataN)
{
    // Open connection to serial port
    int serialPort;
    int finish = 0;
    int n, i;

    // https://www.electronicwings.com/raspberry-pi/raspberry-pi-uart-communication-using-python-and-c
    printf("Connecting to arduino on port: /dev/ttyACM0.\n");
    if ((serialPort = serialOpen("/dev/ttyACM0", 9600)) < 0)
    {
        fprintf(stderr, "Unable to open serial device: %s.\n", strerror(errno));
        pthread_exit((void*)0);
    }

    if (wiringPiSetup() == -1)
    {
        fprintf(stderr, "Unable to start wiringPi: %s\n", strerror(errno));
        pthread_exit((void*)0);
    }

    serialFlush(serialPort);
    while(!finish)
    {
        n = serialDataAvail(serialPort);
        if (n > 0)
        {
            // Lock mutex and copy data to array
            pthread_mutex_lock(&dataMutex);
            for (i = 0; i < 13; i++)
                data[i] = serialGetchar(serialPort);
            data[14] = '\0';
            pthread_mutex_unlock(&dataMutex);
            serialFlush(serialPort);
            pthread_mutex_lock(&withDataM);
            withData = 1;
            pthread_mutex_unlock(&withDataM);
        }
        // Check if thread should finish
        pthread_mutex_lock(&outMutex);
        if (out)
            finish = 1;
        pthread_mutex_unlock(&outMutex);
    }

    serialClose(serialPort);
    printf("Finishing arduino thread.\n");
    pthread_exit((void*)0);
}

// https://computing.llnl.gov/tutorials/pthreads/#Management
void createThread(pthread_t *thread)
{
    pthread_attr_t attr;
    int rc;

    // Create mutex
    pthread_mutex_init(&dataMutex, NULL);
    pthread_mutex_init(&outMutex, NULL);
    pthread_mutex_init(&withDataM, NULL);

    // Initialize attribute and set to joinable
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Create thread
    rc = pthread_create(thread, &attr, readArduino, NULL);
    if (rc)
    {
        printf("ERROR; return code from pthread_create() is %d.\n", rc);
        exit(-1);
    }
}

void destroyThread(pthread_t *thread)
{
    void *status;
    int rc;

    // Destroy mutex
    pthread_mutex_destroy(&dataMutex);
    pthread_mutex_destroy(&outMutex);
    pthread_mutex_destroy(&withDataM);
    
    rc = pthread_join(*thread, &status);
    if (rc)
    {
        printf("ERROR; return code from pthread_join() is %d.\n", rc);
        exit(-1);
    }
    printf("Completed thread join with status %ld.\n", (long)status);
}

/* Asynchronous call */
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed.\n", dt);
    deliveredToken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf("Message arrived.\n");
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost.\n");
    printf("\tCause: %s.\n", cause);
}

/*
*   Create the client structure with necessary data.
*/
void createClient(MQTTClient *client, struct clientInfo *cf, char *capath, char *group)
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
    int rc;

    /* Configure SSL client options */
    ssl_opts.enableServerCertAuth = 0;
    ssl_opts.enabledGroups = group; /* Key exchange */
    ssl_opts.CApath = capath;
    conn_opts.ssl = &ssl_opts;

    MQTTClient_create(client, cf->address, cf->clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(*client, NULL, connlost, msgarrvd, delivered);
    printf("Creating client.\n");
    if ((rc = MQTTClient_connect(*client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d.\n", rc);
        exit(EXIT_FAILURE);
    }
}

void delivereMessage(MQTTClient *client, struct clientInfo *cf)
{
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    pubmsg.payload = cf->payload;
    pubmsg.payloadlen = strlen(cf->payload);
    pubmsg.qos = cf->qos;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(*client, cf->topic, &pubmsg, &token);
    rc = MQTTClient_waitForCompletion(*client, token, cf->timeout);
    printf("Message: %s sent.\n", (char *)pubmsg.payload);
    printf("\nMessage with delivery token %d delivered.\n", token);
}

void run(MQTTClient *client, struct clientInfo *cf)
{
    int read = 0;
    printf("Publishing to topic %s, with client %s.\n", cf->topic, cf->clientid);
    while (!finishP)
    {
        // Is there data available
        pthread_mutex_lock(&withDataM);
        if (withData == 1)
            read = 1;
        else
            read = 0;
        pthread_mutex_unlock(&withDataM);
        if (read == 1)
        {
            // Send data to broker
            if (cf->payload)
                free((void*)cf->payload);
            pthread_mutex_lock(&dataMutex);
            cf->payload = strdup(data);
            pthread_mutex_unlock(&dataMutex);
            pthread_mutex_lock(&withDataM);
            withData = 0;
            pthread_mutex_unlock(&withDataM);
            delivereMessage(client, cf);
        }   
    }
    printf("Sending signal to finish thread.\n");
    pthread_mutex_lock(&outMutex);
    out = 1;
    pthread_mutex_unlock(&outMutex);
    printf("Finishing main program.");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Error: include the key exchange method.\n");
        return 0;
    }
    /* Check if valid key exchange method */
    char *kems[] = {"kyber512", "lightsaber", "ntruhps2048509", "P-256", "X25519"};
    int n = 5;
    int valid = 0;
    for (int i = 0; i < n; i++)
    {
        if (!strcmp(kems[i], argv[1]))
            valid = 1;
    }
    if (valid == 0)
    {
        printf("Invalid key exchange method: %s.", argv[1]);
        return 0;
    }

    //Set signal handler
    signal(SIGINT, handles);
    struct clientInfo cf;
    MQTTClient client;
    pthread_t thread;
    int rc;

    /* Configure client options */
    cf.address = "ssl://13.65.102.222:8883";
    cf.clientid = "Test";
    cf.topic = "test/topic";
    cf.payload = NULL;
    cf.qos = 1;
    cf.timeout = 10000L;

    char *capath = "/home/pi/Documents/certs/azure3/ca.crt";
    char *group = argv[1];
    createClient(&client, &cf, capath, group);
    //createThread(&thread);
    //run(&client, &cf);
    //destroyThread(&thread);
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    pthread_exit(NULL);
}

