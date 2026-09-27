# SIMD Matrix Benchmark: x86-64 Assembly vs. C

A real-time 3D wireframe visualizer and performance benchmark comparing handwritten x86-64 SSE vector-matrix multiplication (`math_core.asm`) against standard scalar C (`main.c`) using [raylib](https://www.raylib.com/).

---

## Overview

The project runs an automated stress test performing 100,000 matrix-vector transformations per frame across 8 cube vertices to measure computational throughput

* **ASM Core (`multiply_vector_asm`)**: Implemented in NASM using 128-bit SSE instructions (`movaps`, `mulps`, `haddps`, `unpcklps`, `movlhps`) to parallelize row-vector multiplication across 16-byte aligned data
* **C Core (`multiply_vector_c`)**: Standard scalar implementation calculating dot products component-by-component
* **Real-Time Visualizer**: Renders dual rotating 3D cubes side-by-side alongside a live history chart plotting execution time (in milliseconds) and calculated speedup.

---
## Prerequisites

* **C Compiler**: GCC or Clang (supporting C99 or later)
* **Assembler**: [NASM](https://www.nasm.us/)
* **Library**: [raylib](https://www.raylib.com/)
---

