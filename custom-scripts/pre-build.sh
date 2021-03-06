#!/bin/sh

cp $BASE_DIR/../custom-scripts/S41network-config $BASE_DIR/target/etc/init.d
chmod +x $BASE_DIR/target/etc/init.d/S41network-config

#Compile the syscall_test.c
BUILDROOT_DIR=$BASE_DIR/..
COMPILER=$BUILDROOT_DIR/output/host/bin/i686-buildroot-linux-uclibc-gcc
$COMPILER -o $BUILDROOT_DIR/output/target/bin/syscall_test $BUILDROOT_DIR/custom-scripts/syscall_test.c

make -C $BASE_DIR/../../hello_world_driver/

#Compile sector_read.c
$COMPILER -o $BUILDROOT_DIR/output/target/bin/sector_read $BUILDROOT_DIR/custom-scripts/sector_read.c

#Compile thread_runner.c
$COMPILER -o $BUILDROOT_DIR/output/target/bin/thread_runner $BUILDROOT_DIR/custom-scripts/thread_runner.c
