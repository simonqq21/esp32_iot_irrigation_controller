from ws_client import Client
from time import sleep 
import json 
from datetime import datetime, time
import asyncio


# test script for the websocket module 
async def main():
    apMode = False
    if (apMode):
        ip = "192.168.4.1"
    else:
        ip = "192.168.5.70"
    port = 5555
    wsRoute = "ws"

    client = Client(ip, port, wsRoute)
    await client.startWS()
    # receiveMessageTask = asyncio.create_task(client.receiveWSMessages())
    # testSequenceTask = asyncio.create_task()
    # print(await asyncio.gather(receiveMessageTask, testSequenceTask))
    message = await client.loadConfig(recv=True)
    
    jsonMessage = json.loads(message)
    print(jsonMessage)
    jsonMessage["cmd"] = "save"
    payload = jsonMessage["payload"]
    payload["ledSetting"] = 2
    payload["ntpEnabledSetting"] = False
    payload["timerEnabledSetting"] = True
    payload["timeSlots"][3]["index"] = 3
    payload["timeSlots"][3]["enabled"] = True 
    payload["timeSlots"][3]["onStartTime"] = time(12,0,0).isoformat()
    payload["timeSlots"][3]["onEndTime"] = time(13,0,0).isoformat()
    payload["timeSlots"][2]["index"] = 2
    payload["timeSlots"][2]["enabled"] = True 
    payload["timeSlots"][2]["onStartTime"] = time(6,30,0).isoformat()
    payload["timeSlots"][2]["onEndTime"] = time(8,0,0).isoformat()
    print(jsonMessage)
    # await client.saveDateTime(datetime.now())
    await asyncio.sleep(0.5)
    # await asyncio.sleep(0.5)
    await client.saveConfig(jsonMessage)
    await client.saveDateTime(datetime(2024,1,2,7,0,0))

if __name__ == "__main__":
    asyncio.run(main())