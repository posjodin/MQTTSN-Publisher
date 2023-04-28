# SimPing: Example UDP Ping Application for SIM7000 Series

RIOT application using the SIM7000 series cellular modules. It has a ping-like application over UDP, "uping".

## Installation

The applications has the RIOT OS in a submodule:
* **RIOT-OS** The project version of RIOT
  ([https://github.com/posjodin/RIOT](https://github.com/posjodin/RIOT)) 
    
After cloning this repository, pull in the submodules:

```
$ git submodule init
$ git submodule update
```

## Building and Flashing

Build the image by running make

`$ make`

To flash the image to the AVR-RSS2 board, use the `avrdude.sh` shell
script. The shell script requires that the shell variable `PORT` is the name of the USB port to which
the AVR-RSS2 board is attached. Locate the USB port to which the
AVR-RSS2 board is attached, and assign the path to the device to
`PORT`. For exampe, in Bash, assuming that the board is on `/dev/ttyUSB0`:

```
$ export PORT=/dev/ttyUSB0
$ bash avrdude.sh
```

Make sure to press the reset button on the board right before you run the shell
script!

## Uping

The UDP ping application `uping`, has two parts. A server, which is a Python script, and a RIOT application.

first start the server the computer where you want the server to run:

```
$ python uping.py <port number>
```

Then run the `uping` command in the RIOT command shell:

```
> uping <hostname> <port number> <count>
```

Uping is located in the parent directory of this directory.

