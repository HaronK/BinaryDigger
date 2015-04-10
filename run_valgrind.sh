#!/bin/bash

valgrind --log-file="vg_%p.log" $@

