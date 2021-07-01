"""
"""

import os

def packetPerformance(N, kem):
    """
    Executes N times the command s_client from openssl using the specified KEM.
    We monitor the activitiy with Wireshark.
    """
    q = "echo \"Q\" | "
    route = "/home/jash/Documentos/Maestria/openssl/apps/openssl s_client -groups "
    cacrt = " -CAfile falcon512_CA.crt"
    cmd = q + route + kem + cacrt
    
    # Execute
    for i in range(N):
        os.system(cmd)

def cipherSelect():
    """
    """
    ciphers = ["frodo640shake", "kyber512", "ntru_hrss701", "lightsaber"]
    N = 2000
    for kem in ciphers:
        packetPerformance(N, kem)
        while 1:
            print("continue (Y/n)?")
            c = input()
            if c == 'Y' or c == 'y':
                break
            if c == 'n':
                return

if __name__ == '__main__':
    cipherSelect()
