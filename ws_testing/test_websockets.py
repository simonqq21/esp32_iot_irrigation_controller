import asyncio
from websockets.asyncio.server import serve

async def echo(websocket):
    async for message in websocket:
        print(f"received {message} from {websocket.request.path}");
        await websocket.send(message)

async def serveWS(*tasks):
    async with serve(echo, "", 5555):
        await asyncio.gather(*tasks, asyncio.get_running_loop().create_future())

async def func1():
    while True:
        print("s");
        await asyncio.sleep(5)


async def main():
    task1 = asyncio.create_task(func1())
    await(serveWS(task1))
    # await task1
    # async with serve(echo, "localhost", 8765):
    #     await asyncio.gather(task1, asyncio.get_running_loop().create_future())
        # await asyncio.get_running_loop().create_future()  # run forever
        
asyncio.run(main())