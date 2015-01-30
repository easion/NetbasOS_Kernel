#!/bin/bash

SOURCE=/opt/devsys/src
GDB=gdb-5.2.1

patch ${SOURCE}/gdb/${GDB}/sim/mcore/interp.c interp.c.diff
patch ${SOURCE}/gdb/${GDB}/gdb/mcore-rom.c mcore-rom.c.diff
patch ${SOURCE}/gdb/${GDB}/gdb/mcore-tdep.c mcore-tdep.c.diff
patch ${SOURCE}/gdb/${GDB}/gdb/config/mcore/tm-mcore.h tm-mcore.h.diff
