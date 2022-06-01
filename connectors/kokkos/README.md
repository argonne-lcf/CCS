# CCS Kokkos Connector

The CCS Kokkos connector is a library leveraging the Kokkos tuning interface
(see: https://github.com/kokkos/kokkos/blob/master/doc/TuningDesign.md) to
provide autotuning capabilities to Kokkos using a CCS tuner.

## Testing the CCS Kokkos connector

This workflow requires a recent c/c++ compiler, CMake, autotools, and the GNU Scientific library.

### Building Kokkos

In order to test the connector, an installation of Kokkos with tuning features enabled
is required (version 3.1 or greater):

```sh
WORK_DIR=$PWD
INSTALL_BASE_PATH=$HOME/opt
KOKKOS_INSTALL=$INSTALL_BASE_PATH/kokkos
git clone git@github.com:kokkos/kokkos.git
cd kokkos
git checkout 3.5.00
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=ON -DKokkos_ENABLE_TUNING=ON -DCMAKE_INSTALL_PREFIX=$KOKKOS_INSTALL
make
make install
```

### Building CCS

Building CCS with the Kokkos connector enabled (the default):

```sh
CCS_INSTALL=$INSTALL_BASE_PATH/ccs
cd $WORK_DIR
git clone git@github.com:argonne-lcf/CCS.git
cd CCS
./autogen.sh
mkdir build
cd build
../configure --prefix=$CCS_INSTALL --disable-samples
make
make install
```

### Building the Kokkos Tuning Playground

Kokkos tuning examples are gathered in the Kokko Tuning Playground repository.
Some examples might require CUDA, and may need to be deactivated in https://github.com/DavidPoliakoff/tuning-playground/blob/main/benchmarks/CMakeLists.txt
(`simple_mdrange`), see: https://github.com/DavidPoliakoff/tuning-playground/pull/2

```sh
cd $WORK_DIR
git clone git@github.com:DavidPoliakoff/tuning-playground.git
cd tuning-playground
mkdir build
cd build
cmake .. -DKokkos_ROOT=$KOKKOS_INSTALL -DTUNINGPLAYGROUND_ENABLE_BENCHMARKS=ON
make
```

### Running the benchmarks

The different benchmarks can be ran from their respective directories:
```sh
cd $WORK_DIR
cd tuning-playground/build/benchmarks/two_var
time ./two_var.exe
export KOKKOS_PROFILE_LIBRARY=$CCS_INSTALL/lib/cconfigspace/ccs-kokkos-connector.so KOKKOS_TUNE_INTERNALS=ON
time ./two_var.exe
unset KOKKOS_PROFILE_LIBRARY KOKKOS_TUNE_INTERNALS
```
The second time should be significantly smaller then the first one. Be aware that `simple_features` without autotuning can more than an hour to run, while it should take a few minutes to run using CCS random tuner.
