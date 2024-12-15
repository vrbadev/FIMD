# FIMD: Fast Isolated Marker Detection

This repository contains implementations of the FIMD for CPUs, GPUs and FPGAs. The CPU and GPU implementations can be compiled and run on Linux-based systems.

| Folder | Description                                                                                |
|--------|--------------------------------------------------------------------------------------------|
| cpu    | FIMD-CPU: C/ASM implementation optimised for low-power microcontrollers and embedded CPUs. |
| gpu    | FIMD-GPU: C implementation for embedded GPUs (OpenGL 3.1 compute shader).                  |
| fpga   | FIMD-FPGA: VHDL implementation for low-end FPGA devices (ActiveHDL project).               |


## Dataset

The related dataset of raw camera frames is available at [our MRS NAS server](https://nasmrs.felk.cvut.cz/index.php/s/AtyqwaS4VOC1EYG). It contains 83914 camera frames of 752x480 resolution in 8-bit binary format (grayscale).


