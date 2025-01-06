# Minimal C++ implementation of the Fast Isolated Marker Detection (FIMD)

This directory contains a minimal C++ implementation of the FIMD approach (currently only for FIMD-CPU). The required C++ standard is C++20. The header file(s) should be included directly in the user's project. 

The primary benefit of this approach is that Bresenham circles can be effortlessly generated using only C++20 templates, thereby eliminating the need for any Python pre-processing before compilation. However, since the circles are generated recursively, the recursion depth quickly exceeds the maximum for larger radii. The limits can be increased using the `-ftemplate-depth=` and `-fconstexpr-depth=` compiler flags.

## Usage example

The following block of code demonstrates how to use the implementation in a C++ project:

```c++
#include <fstream>
#include <vector>
#include "fimd_cpu.hpp"

// Load sample binary frame data
unsigned im_width = 752;
unsigned im_height = 480;
std::ifstream input("./1619240573769481609.bin", std::ios::binary);
std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
if (buffer.empty()) { /* error when reading the file */ }

// Select some parameters
constexpr unsigned radius = 3; // must be known at compile time
unsigned char threshold_center = 120;
unsigned char threshold_diff = 60;
unsigned char threshold_sun = 240;
auto termination = std::array<unsigned char, 2>{0xFF, 0x00};
unsigned max_markers_count = 30;
unsigned max_sun_points_count = 300;

// Prepare lists for detection results
std::list<fimd::Point2D> markers;
std::list<fimd::Point2D> sun_points;

// Initialize the FIMD-CPU detector
fimd::FIMD_CPU<radius> detector(im_width, im_height, threshold_center, threshold_diff, threshold_sun, termination, max_markers_count, max_sun_points_count);
detector.print_boundary(std::cout);
detector.print_interior(std::cout);

// Detect markers and sun points
bool make_copy = true; // if no copy is created, the frame buffer will be modified
detector.detect(buffer.data(), markers, sun_points, make_copy);
```
