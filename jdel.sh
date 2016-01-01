#!/bin/bash

. jbus.sh
printf "$($jbus jdel at $# $@)\n"
