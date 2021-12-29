#!/usr/bin/env python

import sys
import os

print("Now flashing the show...")

idf_path = os.environ["IDF_PATH"]  # get value of IDF_PATH from environment
parttool_dir = os.path.join(idf_path, "components", "partition_table")  # parttool.py lives in $IDF_PATH/components/partition_table

sys.path.append(parttool_dir)  # this enables Python to find parttool module
from parttool import *  # import all names inside parttool module

target = ParttoolTarget("/dev/ttyUSB0")

target.erase_partition(PartitionName("show"))

target.write_partition(PartitionName("show"), "shows/ship.bpf")

print("Done Flashing the Show")
