#!/bin/bash

sed -i  '/^\/Src\/system_stm32f1xx.c/d' Makefile

make -j4
