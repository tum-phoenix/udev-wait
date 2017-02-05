# udev-wait

[![Build Status](https://travis-ci.org/tum-phoenix/udev-wait.svg?branch=master)](https://travis-ci.org/tum-phoenix/udev-wait)

Waits until a USB device is connected.

## Installation

```
mkdir build
cd build
cmake ..
make
sudo make install
```

## Usage

```
udev-wait <SUBSYSTEM> <VENDOR> <PRODUCT>
```

Example:

```
udev-wait hidraw 0738 1705
```
