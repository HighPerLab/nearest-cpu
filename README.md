Find Nearest GPU (NVIDIA) Device
================================

Little program that makes use of HWLOC bindings into CUDA-driver/-runtime
to find the nearest CPU (NUMA) to the GPU device(s).

Can be useful for configuring systems that bind to CPUs/Cores using some
index value.

Compile
-------

Requires CUDA 10+ and HWLOC v2, uses CMake to generate the build system:

```
$ mkdir build && cd build
$ cmake ..
$ make
$ ./nearest-gpu
```

Where Used
----------

### SLURM

Use to set the `gres.conf` for each GPU device, example of what this looks
like:

```
TODO
```
