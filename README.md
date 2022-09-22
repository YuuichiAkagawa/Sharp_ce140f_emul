# Sharp_ce140f_emul
Sharp CE-140F disk drive emulator with an ST-Nucleo board

This is an attempt of emulating the Sharp CE-140F disk drive with an ST-Nucleo board, a L053R8 in this implementation, 
attached to a Sharp Pocket Computer (PC-1403) through the Sharp proprietary 11-pin interface. 

This is a demo video I made, just to show the emulator processing for example the Sharp FILES command:

https://youtu.be/5GLLVkL09qo

The working prototype looks like this at present:

![20220721_090621](https://user-images.githubusercontent.com/659557/180180992-6d9be30f-607c-4927-bcbf-eb3c7a3ea95e.jpg)

but I have in mind to move it to a better looking proto-board build, sooner or later, maybe also switching to a slightly more compact (and powerful) STM Nucleo board, like the L432KC for example. This will happen as soon as I reach a sufficiently stable version, mostly on the software side.

## Hardware interface notes

Interface schematics (subject to further revisions):

![sharp-ce140f-emul](https://user-images.githubusercontent.com/659557/191747166-a117dd97-7f3e-4a3e-a02c-2a77e7dadd89.png)

Since the Sharp PC uses a CMOS 5v logic, while the Nucleo board is a 3.3v device, some level-shifting is required in between the two. Nucleo inputs are is 5v-tolerant, so the board inputs could easily accept the Sharp outputs without the need any converter, but the board 3.3v output isn't enough to drive the 5v input on the Sharp. The level converter I choose is one of this type: https://www.sparkfun.com/products/12009 (actually, one of its many clones), with each line being like:

![image](https://user-images.githubusercontent.com/659557/166907967-b0771314-bf71-4cde-9ebd-4cc6bff93868.png)

I initially struggled a lot, before I got the Nucleo board properly receive the Device Code from the PC, which is the first step of the communication handshake. At first, using the level converter on each data line, I always got a 0xFF (0x41 is expected instead, when a FILES command for example is issued on the Sharp-PC, to invoke the Disk Drive). In spite the converters are in principle bi-directional, after a number of trials and errors I found out that they kept a constant high value on Nucleo inputs, regardless of the Sharp setting a low, because of the normally high impedance of Sharp-PC outputs, I think.

So, I decided to use the level converters only in the Nucleo-to-Sharp direction. I also configured Nucleo internal pull-down on each input line (45K resistor, as per datasheet) and added a 10K in series, as in the schematics above, to achieve the 5-to-3.3 divide in the opposite direction. This way, I reached a stage where the correct 0x41 device code, as well as the follow-up command sequence was received, but it also forced me to use different pins of the Nucleo board for the return lines (Nucleo-to-Sharp), converting them to 5v and issuing to the 11-pin connector through diodes to isolate them from the inputs (Sharp-to-Nucleo). See schematics above. Output and input stages are time-separated, and during output, input pins on Nucleo needs to be set to a PullNone (i.e. high impedance) mode.

About power, the Sharp and the Nucleo do not share the 5v power line, just gnd. This is to prevent the relatively low capacity internal coin cells to be drained by the Nucleo board. At present, the board is powered through its USB plug, but I plan to make it battery powered, maybe rechargeable.

## Software build notes
Board firmware is built using the standard methods offered by the online MBed compiler (https://os.mbed.com/), importing this GitHub repository and selecting the NUCLEO-L053R8 as the target.

The MBed library included within this repository is the (now formally unsupported) version 2. This choice is imposed by the small footprint it offers, compared to v6 (even with a "bare metal" build profile). If and when I move to a larger board (e.g. a L432KC), I might upgrade to latest versions.

The SD File System library is a small revision of the version found here: https://os.mbed.com/cookbook/SD-Card-File-System (the original didn't work out of the box, to me). By the way, it doesn't compile on the latest revision of the MBed library, so I had to rollback MBed (still v2) to revision #137.

The board software is still under development, as each command from the Sharp (DSKF, FILES, SAVE, LOAD, etc.) needs data retrieved from the SD to be properly formatted back to be accepted by the Sharp. This is a rather lengthy process, invoving a big deal of reverse engineering.

## Acknowledgements
Remy, author of the https://pockemul.com/ emulator, who reverse engineered the CE-140F protocol.
Walter (http://www.cavefischer.at/spc/index.htm) who helped on the hardwware interface front.
The entire community of Sharp-PC enthusiasts.
The MBed forums.
