"""
Measure the Wi-Fi usage for the selected mechanisms and elliptic curves.
"""

import os
import time
import sys, getopt
import csv
import numpy as np

def runClient(clientcmd, killcmd):
    """
    """
    os.system(clientcmd)
    time.sleep(15)
    os.system(killcmd)
    time.sleep(30)

def configStrings(argv):
    """
    Set all the necessary configuration
    """
    ip = ""
    clientRoute = ""
    try:
        opts, args = getopt.getopt(argv, "")
    except getopt.GetoptError:
        print("python3 measurePacketPerformance.py -c clientRoute -ip IP")
        sys.exit(2)
    for opt, arg in opts:
        if opt == "-c":
            clientRoute = arg
        if opt == "-ip":
            ip = arg
    kems = ["kyber512", "lightsaber", "ntru_hps2048509", "P-256", " X25519"]
    # For tshark
    host = "host " + ip
    interface = "-i wlan0 "
    options = " -t ad -w "
    captureFRoute = "./packetPerformance"
    baseFilename = "_capture.pcapng "

    # For the client
    client = clientRoute
    # Send kill signal to the process with name client
    kill = "kill -9 $(ps -C client -o pid=)"
    killtshark = "kill -9 $(ps -C tshark -o pid=)"

    for kem in kems:
        filename = kem + baseFilename
        tsharkcmd = "sudo tshark " + interface + options + captureFRoute + filename + host + " &"
        clientcmd = client + kem + " &"
        # Execute tshark command
        os.system(tsharkcmd)
        time.sleep(10)
        runClient(clientcmd, kill)
        os.system(killtshark)

def mergeFiles(outputfile, delimiter):
    """
    Load the files containing the package data, gather the required fields (indicated on top),
    and save them on a single file.
    """
    kems = ["kyber512", "lightSaber", "p256", "ntru", "x25519"]
    packet = ".csv"
    # Open the file containing the information
    with open(outputfile, "w") as out:
        writer = csv.writer(out, delimiter=delimiter)
        # Write the information of the columns (from right to left -> up to down)
        writer.writerow(["Packets","Bytes","Packets A → B","Bytes A → B","Packets B → A","Bytes B → A","Duration","Bits/s A → B","Bits/s B → A"])
        for k in kems:
            # Write what kem is the data from
            writer.writerow([k])
            file = "packetPerformance/" + k + packet
            with open(file, "r") as f:
                reader = csv.reader(f, delimiter=delimiter)
                # Get the header row
                r = next(reader)
                # Read all the file and the required data
                data = []
                for row in reader:
                    r = row[4:10] + row[11:]
                    data.append(r)
                m = np.array(data, dtype=object)
                mT = m.transpose()
                writer.writerows(mT)

if __name__ == '__main__':
    configStrings(sys.argvs[1:])
    mergeFiles("packetPerformance.csv", ',')
