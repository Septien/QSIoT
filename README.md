# Quantum Safe IoT

In the article [1], we present a study for adapting the post-quantum cryptosystems to IoT devices, especially
those capable of using the OpenSSL library. In this repository, all the code used for testing is available.

The organization of this repository is as follows:
    - The files main.c, performance.h, and performance.c, is the code for measuring the CPU usage and the RAM usage.
    - The script measureCPUPerformance.py automates the process of measuring the CPU usage.
    - The script measureRAMPerformance.py automates the process of measuring the RAM usage.
    - The script measurePacketPerformance.py automates the process of measuring the Wi-Fi usage.
    - The script plotPerformanceData.py generates computes the summary statistics and generates the graphs used for analysis.
    - In the folder clients/ is the code for running the MQTT clients. The code gatewayMQTTClient.c is intended to run on a Raspberry Pi. The mqttclientSub.c is intended to run on a regular computer, and its only function is to listen for incomming packets from the broker.
    - The folders with name "kem", with kem being one of the mechanisms analyzed, contains the code available at the NIST competition process.
    - The folder arduino/ contains the code for the sensor nodes and the readio controller of the gateway. The loraClientrh/ folder contains the code for the nodes, and rf69_server/ contains the code for the radio controller.

The required libraries are:
    - The OpenSSL library with the post-quantum cryptosystems integrated, available [here](https://github.com/open-quantum-safe/openssl)
    - The Mosquitto broker modified for use with the post-quantum cryptosystems, available [here](https://github.com/Septien/mosquitto/tree/tls1_3)
    - The Paho C MQTT library modified for use with the post-quantum cryptosystems, available [here](https:github.com/Septien/paho.mqtt.c)

To compile mosquitto, change to the branch tls1_3 and run:
'''
cmake .
make
sudo make install
'''

To compiles Paho C, run:
'''
make
sudo make install
'''

To compile the gateway client run:
'''
gcc gatewayMQTTClient.c -o test -lpaho-mqtt3cs -lpthread -lwiringPi
'''

## Reference
1.- Septién-Hernández J.A, Arellano-Vazquez M., Contreras-Cruz M.A., and Ramírez-Paredes J.P.I, *A Comparative Study of Post-Quantum Cryptosystems for Internet-of-Things Applications*.