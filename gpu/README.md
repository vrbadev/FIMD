# FIMD-GPU: Fast Image Marker Detection using OpenGL Compute Shaders

This directory contains the GPU implementation of the FIMD algorithm. The compute shader file `shader.comp` is compiled and executed using the OpenGL API. The shared library exposes a detection function for the GPU implementation.

CMake compiles two targets:

* `fimd_gpu` - A shared library exposing a detection function in the header file `fimd_gpu.h`. 
* `fimd_gpu_example` - An executable for testing the detection function with source code in the `example.c` file.

## Detection output example
Detection output with (x, y) coordinates for a dataset sample `1619240573769481609.bin` and sequentially tested radii 2, 3, and 4:

```txt
FIMD-GPU: detected 11 marker(s), 5653 sun pixel(s).
Marker(s): [(266,194),(272,316),(257,318),(283,325),(221,353),(228,357),(520,381),(515,384),(411,407),(377,415),(385,415)]
```
