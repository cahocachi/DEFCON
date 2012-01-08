#!/bin/sh

# Shell script to switch between debug/release versions

# step 1 : check command line

release_commandline()
{
	debugprofile=
	releaseprofile=yes
	for i in $@
	do
		value=${i#--debug}
		if [ ${#i} -ne ${#value} ]
		then
			debugprofile=yes
			releaseprofile=
		fi
		value=${i#--release}
		if [ ${#i} -ne ${#value} ]
		then
			debugprofile=
			releaseprofile=yes
		fi
	done
}

# step 2 : react

release_go()
{
	if [ -n "$debugprofile" ]
	then
		echo "DEBUGSYMBOLS=yes" >> config.me
		echo "OPTIMIZE=no" >> config.me
	fi
	if [ -n "$releaseprofile" ]
	then
		echo "DEBUGSYMBOLS=no" >> config.me
		echo "OPTIMIZE=yes" >> config.me
	fi
}

# step 0 : give help

release_help()
{
	echo -e "\t--debug"
	echo -e "\t\tinclude debugging symbols"
	echo -e "\t--release [default]"
	echo -e "\t\tdon't include debugging symbols and optimize"
}
