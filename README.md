# Game of Life (written in OpenCL)

## Summary

- [General](#general)
- [Usage](#usage)
- [Implementation](#implementation)
- [Expand and Where Expand Kernel](#expand-and-where-expand-kernel)
- [Generation Kernel](#generation-kernel)
- [Performance](#performance)

## General

The _**Game of Life**_, also known simply as _**Life**_, is a cellular automaton devised by the British mathematician _**John Horton Conway**_ in 1970.
The game is a **_zero-player game_**, meaning that its evolution is determined by its initial state, requiring no further input. One interacts with the Game of Life by creating an initial configuration and observing how it evolves, or, for advanced players, by creating patterns with particular properties.

### Rules

The universe of the Game of Life is an infinite, two-dimensional orthogonal grid of square cells, each of which is in one of two possible states, _alive_ or _dead_. Every cell interacts with its eight neighbours, which are the cells that are horizontally, vertically, or diagonally adjacent. At each step in time, the following transitions occur:

1.  Any live cell with fewer than two live neighbors dies, as if by underpopulation.
2.  Any live cell with two or three live neighbors lives on to the next generation.
3.  Any live cell with more than three live neighbors dies, as if by overpopulation.
4.  Any dead cell with exactly three live neighbors becomes a live cell, as if by reproduction.

The initial pattern constitutes the _seed_ of the system. The first generation is created by applying the above rules simultaneously to every cell in the seed; births and deaths occur simultaneously, and the discrete moment at which this happens is sometimes called a _tick_. Each generation is a pure function of the preceding one. The rules continue to be applied repeatedly to create further generations.

### Used Seeds

Many different types of patterns occur in the _Game of Life_, which are classified according to their behaviour. In this implementation were used the following patterns:

##### Gosper Glider Gun

the Gosper _glider gun_ produces its first glider on the 15th generation, and another glider every 30th generation from then on.

![](./doc/gosper.svg)

##### DieHard

_Diehard_ is a pattern that eventually disappears, rather than stabilizing, after 130 generations, which is conjectured to be maximal for patterns with seven or fewer cells.

![](./doc/diehard.svg)

##### Acorn

_Acorn_ takes 5206 generations to generate 633 cells, including 13 escaped gliders.

![](./doc/acorn.png)

## Usage

Compile project:

    cd gameoflife_opencl
    make

Run game of life with specific options:

    ./main seed rows cols generations lws [i] [p]

Seed option specific which configuration use:

- g - Gosper
- a - Acorn
- d - DieHard

lws option must be considerated as lws^2^, i option move around a bug found using Intel Graphics Gen6 graphic card and p option disable grid visualitation.

## Implementation

This implementation of the Game of life is written using ANSI C for the host(CPU) code and OpenCL for the device(GPU) code. Four kernels have been implemented:

- **_init_** kernel for initialization of grid with specified seed.
- **_where_expand_** kernel calculates on which sides the grid will have to expand.
- **_expand_** kernel creates a new grid where sides indicated by where_expand are expanded.
- **_generation_** kernel is the core of the project and execute rules for forward the generation.

## Expand and Where Expand Kernel

The **_where_expand_** kernel examines the matrix along the edges and checks if, for each of its sides, there is at least one automaton, and writes the result of the scan into a buffer. This buffer is passed by parameter to the **_expand_** kernel which will initiate a new matrix by adding the rows and columns needed for expansion. The values in the matrix will be the same as in the previous while the cells in the additional rows and columns will be initialized with the value 0.

#### Bug found

To make the expansion procedure more optimized, the **_clEnqueueCopyBufferRect_** method was used, which initializes the whole new matrix with zero values and then, by choosing the sub-array of the just initialized matrix, writes the values of the old, smaller matrix inside of the submatrix. However, the _clEnqueueCopyBufferRect_ method presents a bug when allocating memory on the _Intel HD Graphics 520_ graphics card and has the segmentation fault. To work around this problem, the option **_i_** has been made available which initializes the new matrix using a method that writes the values within the new matrix one by one instead of using the _clEnqueueCopyBufferRect_ method.

## Generation Kernel

This is the core of the algorithm, as it computes the generation change.

The automaton works locally for each cell, analyzing it and its 8 neighbors and deciding for each step of the loop to which state it should to hire.

There are 2 different versions of the automaton, each one being more suited for different hardware:

- A **_global memory_** implementation, without any particular optimizations (target: newer GPUs with hardware caching, devices without a local memory like CPUs)
- A **_local memory_** caching implementation, theoretically more optimized (target: older GPUs without hardware caching)

## Performance

The performance tests were performed on two completely different graphics cards: the **Nvidia GT940M** and the **Intel Graphic Card 520**. From the tests emerges as on the Nvidia card we have better results using the hardware cache or use a very large lws while on the intel card the best performances are only with a large lws.

The tests were done with a 2000x2000 matrix and the method to calculate throughput is:

    //Generation Kernel
    (10.0*memsize)/runtime_ns(generation_evt)

    //Expand Kernel
    (2.0*memsize)/runtime_ns(generation_evt)

    //Where Expand Kernel
    (2.0*memsize)/runtime_ns(generation_evt)

### Generation Kernel

#### Global Version

##### Time(ms)

![](./doc/time_g.png)

| LWS    | Nvidia | Intel |
| ------ | ------ | ----- |
| No LWS | 3ms    | 15ms  |
| 2      | 5ms    | 7ms   |
| 4      | 7ms    | 6ms   |
| 6      | 5ms    | 13ms  |
| 8      | 3ms    | 6ms   |
| 16     | 3ms    | 8ms   |

##### Memory Throughput(GB/s)

![](./doc/memory_g.png)

| LWS    | Nvidia | Intel  |
| ------ | ------ | ------ |
| No LWS | 15GB/s | 33GB/s |
| 2      | 9GB/s  | 29GB/s |
| 4      | 9GB/s  | 32GB/s |
| 6      | 9GB/s  | 32GB/s |
| 8      | 20GB/s | 29GB/s |
| 16     | 12GB/s | 33GB/s |

#### Local Version

##### Time(ms)

![](./doc/time_l.png)

| LWS    | Nvidia | Intel |
| ------ | ------ | ----- |
| No LWS | 3ms    | 15ms  |
| 2      | 39ms   | 39ms  |
| 4      | 10ms   | 11ms  |
| 6      | 5ms    | 8ms   |
| 8      | 3ms    | 7ms   |
| 16     | 3ms    | 5ms   |

##### Memory Throughput(GB/s)

![](./doc/memory_l.png)

| LWS    | Nvidia | Intel  |
| ------ | ------ | ------ |
| No LWS | 3GB/s  | 3GB/s  |
| 2      | 3GB/s  | 4GB/s  |
| 4      | 9GB/s  | 14GB/s |
| 6      | 15GB/s | 24GB/s |
| 8      | 18GB/s | 39GB/s |
| 16     | 18GB/s | 39GB/s |

### Where Expand Kernel

##### Time(ms)

![](./doc/time_e.png)

| LWS    | Nvidia | Intel |
| ------ | ------ | ----- |
| No LWS | 2.9ms  | 0.4ms |
| 2      | 3ms    | 0.4ms |
| 4      | 3ms    | 0.4ms |
| 6      | 2.8ms  | 0.4ms |
| 8      | 2.6ms  | 0.4ms |
| 16     | 2.9ms  | 0.4ms |

##### Memory Throughput(GB/s)

![](./doc/memory_e.png)

| LWS    | Nvidia | Intel  |
| ------ | ------ | ------ |
| No LWS | 10GB/s | 65GB/s |
| 2      | 11GB/s | 65GB/s |
| 4      | 11GB/s | 64GB/s |
| 6      | 11GB/s | 65GB/s |
| 8      | 10GB/s | 65GB/s |
| 16     | 10GB/s | 64GB/s |

### Expand Kernel

#### Version with clEnqueueCopyBufferRect

##### Time(ms)

![](./doc/time_e2.png)

| LWS    | Nvidia | Intel |
| ------ | ------ | ----- |
| No LWS | 18ms   | 3ms   |
| 2      | 2ms    | 3.8ms |
| 4      | 1.7ms  | 3.5ms |
| 6      | 2.1ms  | 3.5ms |
| 8      | 3ms    | 3.7ms |
| 16     | 2.7ms  | 3.7ms |

##### Memory Throughput(GB/s)

![](./doc/memory_e2.png)

| LWS    | Nvidia | Intel |
| ------ | ------ | ----- |
| No LWS | 17GB/s | 8GB/s |
| 2      | 16GB/s | 8GB/s |
| 4      | 17GB/s | 9GB/s |
| 6      | 15GB/s | 9GB/s |
| 8      | 17GB/s | 8GB/s |
| 16     | 16GB/s | 8GB/s |

#### Version without clEnqueueCopyBufferRect

##### Time(ms)

![](./doc/time_w.png)

| LWS    | Nvidia | Intel |
| ------ | ------ | ----- |
| No LWS | 3ms    | 1.7ms |
| 2      | 3.2ms  | 1.9ms |
| 4      | 3ms    | 2ms   |
| 6      | 3.2ms  | 1.8ms |
| 8      | 3.5ms  | 1.8ms |
| 16     | 3ms    | 1.9ms |

##### Memory Throughput(GB/s)

![](./doc/memory_w.png)

| LWS    | Nvidia | Intel  |
| ------ | ------ | ------ |
| No LWS | 10GB/s | 17GB/s |
| 2      | 11GB/s | 16GB/s |
| 4      | 10GB/s | 16GB/s |
| 6      | 10GB/s | 17GB/s |
| 8      | 19GB/s | 17GB/s |
| 16     | 10GB/s | 16GB/s |
