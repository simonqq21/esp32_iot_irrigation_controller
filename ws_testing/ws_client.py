# connect to wifi

# load wifis
# load connection
# load datetime 
# load relay_state 
# load config

# save datetime
# load datetime to confirm the save
# save relay_state 
# load relay_state to confirm the save
# save config
# load config to confirm the save

# save relay_state to revert back 
# load relay_state to confirm the save
# save config to revert back 
# load config to confirm the save

# save connection with wrong wifi credentials to disconnect from the wifi 
#     and start the AP.


# start AP mode

# load wifis
# load connection
# load datetime 
# load relay_state 
# load config

# save datetime
# load datetime to confirm the save
# save relay_state 
# load relay_state to confirm the save
# save config
# load config to confirm the save

# save relay_state to revert back 
# load relay_state to confirm the save
# save config to revert back 
# load config to confirm the save

# save connection with wrong wifi credentials to stop the AP 
#     and reconnect to the wifi.

import asyncio
from websockets.asyncio.server import serve
from websockets.asyncio.client import connect
from time import sleep 
import json 
from datetime import datetime, time

class Client():
    def __init__(self, ip, port, wsRoute):
        self.ip = ip
        self.port = port
        self.wsRoute = wsRoute
        self.websocketURL = f"ws://{self.ip}:{self.port}/{self.wsRoute}"
        self.endloop = False;
    
    async def startWS(self):
        self.websocket = await connect(self.websocketURL)

    def endLoop(self):
        self.endloop = True 

    async def receiveWSMessages(self):
        while not self.endloop:
            try:
                # with connect(self.websocketURL) as websocket:
                message = await self.websocket.recv()
                jsonmessage = json.loads(message)
                print(f"JSON message = {json.dumps(jsonmessage, indent=True)}\n")
            except:
                pass

    async def loadWifis(self):
        print("load wifis")
        # list wifis
        data = {}
        data["cmd"] = "request"
        data["type"] = "wifis"
        data["payload"] = {}
        await self.websocket.send(json.dumps(data))

    async def loadConnection(self):
        print("load connection")
        data = {}
        data["cmd"] = "request"
        data["type"] = "connection"
        data["payload"] = {}
        await self.websocket.send(json.dumps(data))

    async def loadDateTime(self):
        print("load datetime")
        data = {}
        data["cmd"] = "request"
        data["type"] = "datetime"
        data["payload"] = {}
        await self.websocket.send(json.dumps(data))

    async def loadRelayState(self):
        print("load relay_state")
        data = {}
        data["cmd"] = "request"
        data["type"] = "relay_state"
        data["payload"] = {}
        await self.websocket.send(json.dumps(data))

    async def loadConfig(self):
        print("load config")
        data = {}
        data["cmd"] = "request"
        data["type"] = "config"
        data["payload"] = {}
        await self.websocket.send(json.dumps(data))

    async def saveConnection(self, ssid, password, ipIndex, port):
        print("save connection")
        data = {}
        data["cmd"] = "save"
        data["type"] = "connection"
        data["payload"] = {}
        data["payload"]["ssid"] = ssid
        data["payload"]["pass"] = password
        data["payload"]["ipIndex"] = ipIndex
        data["payload"]["port"] = port
        # save connection
        await self.websocket.send(json.dumps(data))

    async def saveDateTime(self, datetimeObj):
        print("save datetime")
        data = {}
        data["cmd"] = "save"
        data["type"] = "datetime"
        data["payload"] = {}
        data["payload"]["datetime"] = datetimeObj.isoformat()
        await self.websocket.send(json.dumps(data))
        print("Saved datetime to ESP32.")

    async def saveRelayState(self, relay_state):
        print("save relay_state")
        data = {}
        data["cmd"] = "save"
        data["type"] = "relay_state"
        data["payload"] = {}
        data["payload"]["relay_state"] = relay_state
        await self.websocket.send(json.dumps(data))
        print("Saved relay_state to ESP32.")

    # timeSlots is list of dicts with format:
    # {
    #     "index": int,
    #     "onStartTime": datetime.time,
    #     "onEndTime": datetime.time,
    #     "enabled": bool
    # }
    async def saveConfig(self, name, ntpEnabledSetting, gmtOffsetSetting, timerEnabledSetting, \
                         ledSetting, timeSlots):
        print("save config")
        data = {}
        data["cmd"] = "save"
        data["type"] = "config"
        data["payload"] = {}
        data["payload"]["name"] = name
        data["payload"]["ntpEnabledSetting"] = ntpEnabledSetting 
        data["payload"]["gmtOffsetSetting"] = gmtOffsetSetting
        data["payload"]["timerEnabledSetting"] = timerEnabledSetting
        data["payload"]["ledSetting"] = ledSetting
        data["payload"]["timeSlots"] = timeSlots
        for i in range(len(timeSlots)):
            data["payload"]["timeSlots"][i]["onStartTime"] = data["payload"]["timeSlots"][i]["onStartTime"].isoformat()
            data["payload"]["timeSlots"][i]["onEndTime"] = data["payload"]["timeSlots"][i]["onEndTime"].isoformat()
        await self.websocket.send(json.dumps(data))

async def testSequence(client: Client, apMode, changeWiFi=False):
    print("ESP32 websocket module testing sequence")
    await client.loadWifis()
    await asyncio.sleep(3)
    await client.loadConnection()
    await asyncio.sleep(3)
    await client.loadDateTime()
    await asyncio.sleep(3)
    await client.loadRelayState()
    await asyncio.sleep(3)
    await client.loadConfig()
    await asyncio.sleep(3)

    await client.saveDateTime(datetime(2024,1,2,7,0,0))
    await client.loadDateTime()
    await asyncio.sleep(3)
    await client.saveRelayState(True)
    await client.loadRelayState()
    await asyncio.sleep(3)
    await client.saveConfig("mint", True, 8, True, 1, [
        {
            "index": 0,
            "onStartTime": time(6,0,0),
            "onEndTime": time(8,0,0),
            "enabled": True
        },
        {
            "index": 1,
            "onStartTime": time(8,0,0),
            "onEndTime": time(9,0,0),
            "enabled": True
        },
        {
            "index": 2,
            "onStartTime": time(10,0,0),
            "onEndTime": time(15,0,0),
            "enabled": False
        }
    ])
    await client.loadConfig()
    await asyncio.sleep(3)

    await client.saveDateTime(datetime.now())
    await client.loadDateTime()
    await asyncio.sleep(3)
    await client.saveRelayState(False)
    await client.loadRelayState()
    await asyncio.sleep(3)
    await client.saveConfig("basil", True, 8, False, 2, [
        {
            "index": 0,
            "onStartTime": time(6,0,0),
            "onEndTime": time(8,0,0),
            "enabled": False
        },
        {
            "index": 1,
            "onStartTime": time(8,0,0),
            "onEndTime": time(9,0,0),
            "enabled": False
        },
        {
            "index": 2,
            "onStartTime": time(9,0,0),
            "onEndTime": time(10,0,0),
            "enabled": False
        }
    ])
    await client.loadConfig()
    await asyncio.sleep(3)

    if (changeWiFi):
        if (apMode):
            await client.saveConnection("QUE-STARLINK", "Quefamily01259", 70, 5555)
        else:
            await client.saveConnection("QUE-STARLINK", "wrongpassword", 70, 5555)
    client.endLoop()

async def main():
    apMode = True 
    apMode = False
    changeWiFi = False
    
    if (apMode):
        client = Client("192.168.4.1", 5555, "ws")
    else:
        client = Client("192.168.5.70", 5555, "ws")
    await client.startWS()
    # task1 = asyncio.create_task(func1())
    receiveMessageTask = asyncio.create_task(client.receiveWSMessages())
    testSequenceTask = asyncio.create_task(testSequence(client, apMode))
    await asyncio.gather(receiveMessageTask, testSequenceTask)

if __name__ == "__main__":
    asyncio.run(main())