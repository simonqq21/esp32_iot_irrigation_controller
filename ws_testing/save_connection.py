from ws_client import Client
from time import sleep 
import json 
from datetime import datetime, time
import asyncio

# test script for the websocket module 
async def main():
    apMode = True
    if (apMode):
        ip = "192.168.4.1"
        newIP = "192.168.5.70"
        ssid = "QUE-STARLINK"
        password = "Quefamily01259"
    else:
        ip = "192.168.5.70"
        newIP = "192.168.4.1"
        ssid = "default-ssid"
        password = "password123"
    ipIndex = 70
    port = 5555
    wsRoute = "ws"

    client = Client(ip, port, wsRoute)
    await client.startWS()
    await client.saveConnection(ssid, password, ipIndex, port)
        

if __name__ == "__main__":
    asyncio.run(main())