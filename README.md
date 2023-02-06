# Triangles' intersection visualisation on Vulkan

## How to build
### Linux
```sh
git submodule init
git submodule update
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release
cd build/
make -j12 install
```

## How to run program
```sh
cd build
# Available options:
#  -h [ --help ]         Print this help message
#  --broad arg (=octree) Algorithm for broad phase (bruteforce, octree, uniform-grid)
./triangles --broad=uniform-grid < ../hw3d/test/intersect/resources/large0.dat
```

## Preview
<!-- Some beautiful screenshotes there -->
