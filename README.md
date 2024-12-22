# FIMD: Fast Isolated Marker Detection

This repository contains implementations of the FIMD for CPUs, GPUs and FPGAs. The CPU and GPU implementations can be compiled and run on Linux-based systems.

| Folder | Description                                                                            |
|--------|----------------------------------------------------------------------------------------|
| cpu    | FIMD-CPU: C implementation optimised for low-power microcontrollers and embedded CPUs. |
| gpu    | FIMD-GPU: C implementation for embedded GPUs (OpenGL 3.1 compute shader).              |
| fpga   | FIMD-FPGA: VHDL implementation for low-end FPGA devices (ActiveHDL project).           |

## Usage (FIMD-CPU and FIMD-GPU)

Clone this repository (a CLion project) and add its top directory as a subdirectory to your CMake project. Compile the top CMake project. The shared libraries will be built as `libfimd_cpu.so` and `libfimd_gpu.so`, with absolute paths in CMake variables `FIMD_CPU_LIB` and `FIMD_GPU_LIB`, respectively. See the `CMakeLists.txt` files and executable examples in the related directories for more details.

## Dataset

The related dataset of raw camera frames is available at [our MRS NAS server](https://nasmrs.felk.cvut.cz/index.php/s/AtyqwaS4VOC1EYG). It contains 83914 camera frames of 752x480 resolution in 8-bit binary format (grayscale).

## Citation
(To be added.)

