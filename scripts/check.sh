#!/bin/bash

prog=check.sh
spl_module=../modules/spl/spl.ko
splat_module=../modules/splat/splat.ko
splat_cmd=../cmd/splat
verbose=

die() {
	echo "${prog}: $1" >&2
	exit 1
}

warn() {
	echo "${prog}: $1" >&2
}

if [ -n "$V" ]; then
	verbose="-v"
fi

if [ -n "$TESTS" ]; then
	tests="$TESTS"
else
	tests="-a"
fi

if [ $(id -u) != 0 ]; then
	die "Must run as root"
fi

if /sbin/lsmod | egrep -q "^spl|^splat"; then
	die "Must start with spl modules unloaded"
fi

if [ ! -f ${spl_module} ] || [ ! -f ${splat_module} ]; then
	die "Source tree must be built, run 'make'"
fi

echo "Loading ${spl_module}"
/sbin/insmod ${spl_module} || die "Failed to load ${spl_module}"

echo "Loading ${splat_module}"
/sbin/insmod ${splat_module} || die "Unable to load ${splat_module}"

while [ ! -c /dev/splatctl ]; do sleep 0.1; done
$splat_cmd $tests $verbose

echo "Unloading ${splat_module}"
/sbin/rmmod ${splat_module} || die "Failed to unload ${splat_module}"

echo "Unloading ${spl_module}"
/sbin/rmmod ${spl_module} || die "Unable to unload ${spl_module}"

exit 0
