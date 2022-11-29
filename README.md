# Sharp_ce140f_emul
Sharp CE-140F disk drive emulator with an ST-Nucleo board

This is an attempt of emulating the Sharp CE-140F disk drive with an ST-Nucleo board, a L053R8 in this implementation, 
attached to a Sharp Pocket Computer (PC-1403) through the Sharp proprietary 11-pin interface. 

This is a demo video I made, just to show the emulator processing for example the Sharp FILES command:

https://youtu.be/5GLLVkL09qo

I started from here:

<img src="https://user-images.githubusercontent.com/659557/180180992-6d9be30f-607c-4927-bcbf-eb3c7a3ea95e.jpg" width=100% height=100%>

and a proto-board version, still based on the L053R8, presently looks like this:

<img src="https://user-images.githubusercontent.com/659557/197160147-e2b9a441-d164-4adc-97ca-d35f92db984d.jpg" width=100% height=100%>

A move to a more compact (and powerful) L432KC board is in progress. This is how it appears at present:

<img src="https://user-images.githubusercontent.com/659557/202128859-ecb3f3f4-8933-494c-a026-fa893b7ce3a1.jpg" width=100% height=100%>

I'm also wondering about the realization of a custom PCB...

## Hardware interface notes!

Interface schematics (this one is the pinout of the L053R8 board):

<img src="https://user-images.githubusercontent.com/659557/197160553-7b8a961b-1c60-4c4f-9ef8-4a4cbc0db2b4.png" width=100% height=100%>

Since the Sharp PC uses a CMOS 5v logic, while the Nucleo board is a 3.3v device, some level-shifting is required in between the two. Nucleo inputs are is 5v-tolerant, so the board inputs could easily accept the Sharp outputs without the need any converter, but the board 3.3v output isn't enough to drive the 5v input on the Sharp. The level converter I choose is one of this type: https://www.sparkfun.com/products/12009 (actually, one of its many clones).

I initially struggled a lot, before I got the Nucleo board properly receive the Device Code from the PC, which is the first step of the communication handshake. At first, using the level converter on each data line, I always got a 0xFF (0x41 is expected instead, when a FILES command for example is issued on the Sharp-PC, to invoke the Disk Drive). In spite the converters are in principle bi-directional, after a number of trials and errors I found out that they kept a constant high value on Nucleo inputs, regardless of the Sharp setting a low, because of the normally high impedance of Sharp-PC outputs, I think.

So, I decided to use the level converters only in the Nucleo-to-Sharp direction. I also configured Nucleo internal pull-down on each input line (45K resistor, as per datasheet) and added a 10K in series, as in the schematics above, to achieve the 5-to-3.3 divide in the opposite direction. This way, I reached a stage where the correct 0x41 device code, as well as the follow-up command sequence was received, but it also forced me to use different pins of the Nucleo board for the return lines (Nucleo-to-Sharp), converting them to 5v and issuing to the 11-pin connector through diodes to isolate them from the inputs (Sharp-to-Nucleo). See schematics above. Output and input stages are time-separated, and during output, input pins on Nucleo needs to be set to a PullNone (i.e. high impedance) mode.

About power, the Sharp and the Nucleo do not share the 5v power line, just gnd. This is to prevent the relatively low capacity internal coin cells to be drained by the Nucleo board. At present, the board is powered through its USB plug, but I plan to make it battery powered, maybe rechargeable.

## Emulation software description
This emulator tries to respond as closely as possible (given the information we have) to the commands issues by the main Sharp-PC (I'll call it "PC" in the following), over the 11-pin interface. The low-level protocol starts with a 1-bit serial "device select" code byte sent from the PC to Disk drive. This is common to all of the devices attached to the PC, with the CE-140F Disk Drive being identified by a device code of 0x41, as said before. Once the drive (or the emulator) has acknowledged this code, the PC is going to send the disk commands with a string of one or more bytes, plus a checksum, each composed of two 4-bit nibbles (1 byte), first byte is the command code, last is a checksum. 

As a response to each command, the disk drive (or emulator) should respond with a string of one or more bytes which "makes sense" to the PC receiving them. That's the most tricky part of the reverse engineering stage, as we don't know how to respond to each and every commmand, having figured out only a few of them, until now. E.g. DSKF (free disk space), LOAD (load a BASIC file, from disk to sharp PC), or SAVE (store a BASIC program as a file on disk), among the others. The CE-140F Service Manual describes most of the hardware-level protocol (although not clearly, to me at least), but doesn't provide any information about the disk commands format.

The emulator software stores the received command string in the Nucleo board memory, before decoding and processing the command. Results are then stored in memory as well, and sent back to the PC only after the processing is complete. For example, during LOAD of a BASIC (tokenized, hence binary) file, this is entirely read from the SD-card to memory and only at the end it's sent to the PC. Other commands split large data in chunks, like the ASCII-type LOAD which sends one line at a time, or SAVE where data is received in 256-bytes chunks.

Such a synchronous, sequential approach (receive-process-send) is made possible because of the relatively quick SD response times and the large amount of memory in the L432KC Nucleo board. Infact, with the L053R8 board, which has a smaller memory, the file size during LOAD is limited. A more sophisticated, asynchronous, approach could be possible in principle, to overcome the memory limitations, with two threads (read and send) and a ring buffer in between, but the development is more complex, both to write and to test - worth it?

## Software build notes
Board firmware is built using the standard methods offered by the online MBed Keil Studio IDE - https://studio.keil.arm.com/ - importing this GitHub repository and selecting the NUCLEO-L053R8, or NUCLEO-L432KC, depending on the actual target hardware, as the target. Refer to MBed Keil Studio documentation for details about how to proceed.

The MBed library included within this repository is the (now formally unsupported) version 2. This choice is imposed by the small footprint it offers, compared to v6 (even with a "bare metal" build profile). If and when I move to a larger board (e.g. a L432KC), I might upgrade to latest versions.

The SD File System library is a small revision of the version found here: https://os.mbed.com/cookbook/SD-Card-File-System (the original didn't work out of the box, to me). By the way, it doesn't compile on the latest revision of the MBed library, so I had to rollback MBed (still v2) to revision #137. In any case, all of this is already included in present repo, which should compile out of the box.

The board software is still under development, as each command from the Sharp (DSKF, FILES, SAVE, LOAD, etc.) needs data retrieved from the SD to be properly formatted back to be accepted by the Sharp. This is a rather lengthy process, invoving a big deal of reverse engineering.

## Acknowledgements
Remy, author of the https://pockemul.com/ emulator, who reverse engineered the CE-140F protocol.

Walter (http://www.cavefischer.at/spc/index.htm) who helped on the hardwware interface front.

The entire community of Sharp-PC enthusiasts.

The MBed forums.
