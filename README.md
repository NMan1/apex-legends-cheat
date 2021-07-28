# apex legends external cheat

external cheat utilizing kernel gdi rendering & kernel key input

esp, chams, broken rcs

## bypass

syscall is hooked then called, once called hook is restored and enters a while loop. Usermode thread is trapped in kernel loop where cheat runs.

thread is attempted to be hidden by removing it from eprocess ThreadListHead, as well as the thread being hijacked from a legit process to have a start address backed by a signed process (probaly doesn't matter but can't hurt). where u get fucked is the anticheats stack walk, traversing the stack for address reissuing outside of a legit module. perhaps this isn't a big deal because the thread was created in usermode land so they may not stackwalk it but I have read they do. The threads irp also is in the kernel with its cpl being 0 as well which is suspicious (u can check threads privilege level by checking SegCs register, SegCs & 3 i suppose u could maybe spoof this too? idk maybe use PspGetContextThreadInternal).

Anyways ive been using it for a week and a half on apex and haven't been banned yet (its probably coming soon[!!!](https://cdn.discordapp.com/attachments/784596712060092427/800163253161820181/491.gif)). There is a lot of things that can be detected here and a lot of things u can do to hide the thread better

## usage
1. load the apex_driver.sys with kdmapper or whatever u want to use
2. run apex_loader.exe
    - this executable loads the apex_client.dll into the specified process (default EpicGamesLauncher.exe) [change your path and hijackled process here](https://github.com/NMan1/apex-legends-cheat/blob/6332e968db690c5285683bbc49815f880ba83827/apex_loader/core/main.cpp#L5)
3. Use Dbgview to verify successfully loading of driver.
    - if driver failed to initialize itself (couldn't get rendering functions, failed to unlink thread) it should return STATUS_UNSUCCESSFUL and a messagebox will popup from hijacked process indicating something failed

notes:
- change screen resolution for proper w2s, [replace all 1920 x 1080](https://github.com/NMan1/apex-legends-cheat/blob/6332e968db690c5285683bbc49815f880ba83827/apex_driver/core/sdk/sdk.cpp#L76)
- END key unloads driver
- F1 toggles esp
- F2 toggles rcs, if I remember correct this was broken, so I wouldn't use it

<img src="https://i.imgur.com/6QZ4fBS.png"/>

https://user-images.githubusercontent.com/44145464/126057251-4de04761-00d1-4718-b074-6f2bc634f97c.mp4


credits:

quick dll thread hijack paste
https://github.com/hrt/ThreadJect-x64

unknown apex thread for offsets
