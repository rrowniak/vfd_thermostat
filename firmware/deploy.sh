#!/bin/bash

sudo stm32flash  -w build/temp_meter.hex -v -g 0x0 /dev/ttyUSB0 
