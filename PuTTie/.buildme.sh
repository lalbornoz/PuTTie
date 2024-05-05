#!/bin/sh
make "-j$(grep processor /proc/cpuinfo | wc -l)" "${@}";
