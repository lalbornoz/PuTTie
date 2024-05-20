#!/bin/sh
case "${1}" in
--print-program)
	shift 1;
	printf "putty.exe\n";
	return 0;
	;;

--start-server)
	shift 1;
	./PuTTie/build.sh --dbg-svr ./putty.exe &
	return "${?}";
	;;
esac;
