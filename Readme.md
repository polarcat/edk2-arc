# EDK2 ARC platform (Work in progress)

This repository provides reference implementation of ARC platform for EDK2. Functionally all presented UEFI boot stages are very limited at the moment (some do not even exist yet, e.g. DXE and BDS). Also, SEC, PEI and PEIMs development is carried with certain goals in mind, such as:

* No dynamic memory allocations.
* No relocations i.e. all pre-EFI modules are aimed for execution-in-place (XIP).
* Fixed configuration i.e. as many things as possible should be defined during build time (e.g. including PEIMs and PPIs).

> Listed above restrictions impact some design choices for all pre-EFI modules.

TODO:
- [ ] Implement missing CPU specific libraries.
- [ ] Run DXE environment with console output enabled.
- [ ] Build and run UEFI shell.
- [ ] Build BDS.
- [ ] Either boot Linux kernel or launch some standalone binary in QEMU via BDS.

# How to build

## Prerequisites

- [x] ARC GNU toolchain is installed.
> Check [https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain.git](https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain.git) for how to build toolchain from sources.

## Build QEMU-ARC.fd image

> QEMU-ARC.fd image is a flash device (FD) container. FD file can incapsulate one or multiple firmware volumes (FV). Visit [https://tianocore-docs.github.io](https://tianocore-docs.github.io) to find out more.

### Clone repositories

```sh
# Create a folder that will contain all required repositories, e.g.:
#
~/> mkdir -p edk2-sources
~/> cd edk2-sources

# Clone EDK2 main repository with a branch that contains required changes to
# support ARC CPU
#
~/> git clone https://github.com/polarcat/edk2.git -b edk2-arc

# Clone EDK2 dependencies
#
~/> git clone https://github.com/tianocore/edk2-non-osi.git
~/> git clone https://github.com/tianocore/edk2-basetools
~/> cd edk2
~/> git submodule update -i MdePkg/Library/MipiSysTLib/mipisyst
~/> git submodule update -i MdeModulePkg/Library/BrotliCustomDecompressLib/brotli
~/> cd ..

# Clone EDK2 ARC platform repository
#
~/> git clone https://github.com/polarcat/edk2-arc.git
```
### Run build script

There is helper script under `edk2-arc/scripts` folder that automates some steps required for image building. What does it take care of:
* Setting up EDK2 environment (where workspace directory is located outside of source tree).
* Deploying Python virtual environment based on `edk2/pip-requirements.txt` in workspace directory.
* Building EDK2 binaries.

```sh
# It is essential to compile build tools before building FD images.
#
~/> edk2-arc/scripts/build-qemu-fd.sh build-tools

# Choose actions from help output.
#
~/> edk2-arc/scripts/build-qemu-fd.sh
```
> When build completes the resulting FD image location is printed onto console.

# How to test

## Using QEMU

### Build QEMU ARC

Unfortunately mainstream QEMU ARC does not support booting UEFI firmware from EDK2 FD files. Luckily it was relatively easy to hack it. Patched version of QEMU ARC is available at [https://github.com/polarcat/qemu.git](https://github.com/polarcat/qemu.git) within branch `arc-uefi`.

```sh
~/>  git clone https://github.com/polarcat/qemu.git -b arc-uefi
```

To build QEMU ARC binaries select target as in example below:

```sh
~/> ./configure --target-list=arc-softmmu
```

> For more more information about building QEMU please refer to README.md file in [QEMU repository](https://github.com/polarcat/qemu.git).

### Test QEMU-ARC.fd image

Once `qemu-system-arc` binary is successfully built and installed it can be run as in example below:

```sh
# Replace <...> with actual paths to corresponding files.
# -kernel parameter requires some ELF32 ARC binary executable.
# -bios parameter requires FD image.
# FD image for e.g. `hs4x` board can normally be found at
# $WORKSPACE/Build/hs4x/DEBUG_GCC/FV/QEMU-ARC.fd
#
qemu-system-arc -m 4G -M virt -nographic -kernel <...> -bios <...>
```

## Using ARC HS4xD Development Kit

TODO

> [ARC HS4xD Development Kit](https://www.synopsys.com/dw/ipdir.php?ds=arc-hs-development-kit)
