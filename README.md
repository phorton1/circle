My Fork of Circle
=====================

Version
-------

This is a fork of the Circle is a C++ bare metal programming environment for the Raspberry Pi.

Changes to the core Circle source code have been minimized and carefully documented
to allow for future updates to the Circle codebase, but otherwise, this is a completely
detatched and separate fork of Circle.

All code changes within existing Circle source files are bracketed by #ifdef PRH_MODS.

Changes to Rules.mk *should* be backwards compatible, so this Circle source *should*
generally work with Linux and/or MacOS builds, but the changes are only tested
in my Windows based develpment environment.


Please See
----------

Please see https://github.com/rsta2/circle for the original Circle source code.

Please see the repository history for a complete list of the changes since the fork.

Please see my **circle-prh** repository at
[https://github.com/phorton1/circle-prh](https://github.com/phorton1/circle-prh)
for my development work based on Circle. I have made a number of what I believe
are significant extensions to the system, particularly for audio device development,
including a port of the Teensy Audio library to bare metal rPi.

That general audio development system boasts sub 10ms latency, and with the
proper soundcard, professional level audio quality.

Based on that, I am in the process of developing a number of projects.
I have made the first of those public.

Please see my repository at
[https://github.com/phorton1/circle-prh-apps-Looper](https://github.com/phorton1/circle-prh-apps-Looper)
for the implementation of a multi-track **Audio Looper** which is based on this circle fork, and
my extensions. You *might* also be interested in my Teensy based (Arduino development environent)
**[teensyExpression Pedal](https://github.com/phorton1/Arduino-teensyExpression)** which
connects to, and controls, that Looper (as well as my iPad based guitar rig).

All of this is part of my long term ongoing
**[rPi bare metal vGuitar rig](https://hackaday.io/project/165696-rpi-bare-metal-vguitar-rig)**
project at hackaday.



Development Environment
-----------------------

There is a Windows batch file in the root directory **makeall.bat** that
merely changes to each interesting subdirectory and performs a "make" within
that directory.  It builds the main Circle libraries, a subset of the addon libraries,
and in my case, the prh/bootloader folder.  You may edit this file freely to build those
parts of Circle and addons that you need.  Makeall.bat also accepts an argument of **clean**
to remove build artifacts and do a clean build.

The difficult part was getting the correct MinGW and GCC toolchain setup onto the
Windows 10 machine. It is difficult for me to document accurately as I've had the
same MinGW setup on my machine(s) for the last 10 years or so, since XP, and I have
downloaded and installed a number of gcc toolchains over the years, in conjunction
with Arduino, rPi, and Android devleopment evironments.

I believe the following are all that is required to build on Windows:

* a version of **MinGW** with MSys
* the arm-none-eabi **GCC toolchain** or equivilant
* the latest version of **Make.exe** copied into the MinGW/MSys/1.0/bin directory

The latest version of **MinGW** *should* work.  You can generally find MinGW at
http://www.mingw.org/ and I believe the latest version at
https://osdn.net/projects/mingw/downloads/68260/mingw-get-setup.exe/ will work.

I am using the "official" **GNU Arm Embedded Toolchain Version 7-2018-q2-update**,
released on June 27, 2018. I remember having some kind of a problem with the latest
release at the time (8-2018-q4-major) and solving it by reverting back one version
to the 7-2018-q2 update.  GNU Arm toolchains are generally available at
https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads

I downloaded make-4.2.tar.gz from https://www.gnu.org/software/make/ and
http://ftp.gnu.org/gnu/make/ and copied the **Make.exe** from it into my
MinGW/MSys/1.0/bin folder.

I believe the source tree for circle can be put anywhere.
Mine is in C:\src\circle. All path references within the Circle
project itself are relative. If the setup is working, you can also
run "make" from any Circle folder.


Changes to Rules.mk
--------------------

There were very few changes needed to the make system (Rules.mk) to
get it to build correctly the first time.  All I did was set the RASPPI
according to rst's original instructions and it worked the first time!

I subsequently added some minor extra capabilities to **Rules.mk** that
are now used, particularly to build my bootloader, and more generally to
allow recursive top-down makes in my development environment.

    TARGET_ROOT ?= kernel
    TARGET_SUFFIX ?= img

**TARGET_ROOT** and **TARGET_SUFFIX** allow one to specify a different
final linked filename besides kernelN.img in top level Makefile(s).
This is used in the bootloader, which is built as *recoveryN.img*,
which is called by the stock boot_code.bin **before** kernelN.img.
This allows the bootloader to run on every boot and yet allows the
familiar kernelN.img filenames for (your, regular, other) uploaded
executables.

    PLUS3B   ?= 1
        # set this to zero on rPi3 (not plus)

The **PLUS3B** definition is added to the existing Circle **RASPPI**
definition and is passed to the compiler as a define.  This is used by
my Circle Bluetooth stack (not included in this initial publication of c
ircle) which needs to send a different HCD file to the rPi onboard
BT HCI controller for initialization based on the pi model.
You *may* set this appropriatly when you change RASPPI if you want.
You **must** set this appropriately to use my Circle BT stack (later).


    MAKE_LIBS = \
        ../std_kernel.mark \
        ../../libaudio.mark \

**MAKE_LIBS** may be defined in a top level makefile to specify a set of
subprojects that will be built, top-down, when the top level makefile is built.
You specify a list of subdirectories as *targets* with the file extension **".mark"**.
It allows you to run one build regardless of what you change in any libraries that
you specify, and also supports the **clean** parameter.   Note that H file
dependencies are **NOT** supported in the Circle makefile scheme, so if you
change main Circle header files, you **must** do a clean build of the entire
Circle source tree.

The **MAKE_LIBS** macro simply allows you to build, and/or clean, a number of
subprojects from the main, top level, makefile.  The above example is from an
audio test program that builds a standard kernel and the audio library as needed.

Please see the various makefiles in the system for examples of
how these additional capabilities are used.



Credits
-------

**rst** for the whole thing

**dwelch** for dropping breadcrumbs along the way https://github.com/dwelch67/raspberrypi

**Paul Stoffregen** for just being brilliant.  Oh yeah, and the Teensy
https://www.pjrc.com/, and the Teensy Audio Library,
https://github.com/PaulStoffregen/Audio and so ...much ... more ...


---------

**Raspberry Pi** is a trademark of the *Raspberry Pi Foundation*.
