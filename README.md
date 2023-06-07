# iotsend

Send messages via the iothub service

## Overview

The iotsend utility uses the libiotclient library to send messages
to the iothub service for delivery to the cloud.

A message sent to the cloud has two components:
- Message Headers
- Message payload

The message headers can be specified using the -H command line option.
Message headers are key:value pairs separated by a semicolon.

The message body is ingested via the standard input of the iotsend application or via a filename specified on the command line.

## Command Line Arguments

```
usage: iotsend [-v] [-h] [<filename>]
 [-h] : display this help
 [-H headers]
 [-v] : verbose output
 ```

## Prerequisites

The iotsend utility requires the following components:

- libiotclient : IOT Client library ( https://github.com/tjmonk/libiotclient )
- iothub: IOTHub Service ( https://github.com/tjmonk/iothub )

## Build

```
./build.sh
```

## Examples

Before running the examples, make sure the iothub service is running and
connected to the IoT Hub using a valid device connection string.

For example:

```
iothub -c "HostName=my-iot-hub.azure-devices.net;DeviceId=device-001;SharedAccessKey=ROpU5sG+XRIFWJeJGWCm7xLv8VIYyGx6vKfyNjPduAs=" &
```

Send "Hello World" message

```
echo "Hello World" | iotsend
```

Send "Hello World" message with some custom headers

```
echo "Hello World" | iotsend -H "source:iotsend;messagetype:greeting"
```

Send a file as a payload

```
cat << EOF > payload.txt
This is an IOT payload
It will be sent to my Azure IoT Hub
using the iotsend utility.
EOF

iotsend payload.txt

iotsend -H "source:iotsend;from:file" payload.txt
```