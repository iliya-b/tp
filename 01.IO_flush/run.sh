#!/bin/bash


gcc -o io_flush io_flush.c
echo "Running without new lines and fflush"
./io_flush
echo -e "\nRunning with new lines"
./io_flush --newlines
echo "Running with fflush"
./io_flush --fflush
echo -e "\nRunning with newlines and fflush"
./io_flush --newlines-fflush

