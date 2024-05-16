#!/bin/sh
./PuTTie/build.sh --dbg-svr ./putty.exe >/dev/null 2>&1 &
./PuTTie/build.sh --dbg-cli ./putty.exe "${@}";
