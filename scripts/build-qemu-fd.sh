#!/bin/bash

set -e

workspace_=$HOME/tmp/edk2
numthreads_=$(getconf _NPROCESSORS_ONLN)

gen_target_txt()
{
	local tools_in=../edk2-arc/Platform/ARC/snps-tools.txt
	local tools_out=Conf/snps-tools.txt

	cat BaseTools/Conf/tools_def.template > $tools_out
	cat $tools_in >> $tools_out

	cat > Conf/target.txt << EOF
ACTIVE_PLATFORM = EmulatorPkg/EmulatorPkg.dsc
TOOL_CHAIN_CONF = Conf/snps-tools.txt
BUILD_RULE_CONF = Conf/build_rule.txt
EOF
}

print_help()
{
	echo
	printf "\033[1m> Usage: $(basename $0) <options>\033[0m\n"
	printf "\033[2m"
	printf "|\n| Options:\n"
	printf "|   build-fd     build firmware binaries\n"
	printf "|   make-tools   make required base tools binaries\n"
	printf "|   clean        delete $workspace_/Build folder\n"
	printf "|   update-venv  update Python virtual environment\n"
	printf "|   list-venv    list Python virtual environment\n"
	printf "|   make-kernel  make dummy kernel image for ARC\n"
        printf "\033[0m\n"
}

update_python_venv()
{
	pip install -r pip-requirements.txt
}

list_python_venv()
{
	pip list
}

build_dummy_kernel()
{
        cat > $HOME/tmp/kernel.c << EOF
void main(void)
{
	asm volatile("mov r0,0xbeef");
	asm volatile("sleep");
}
EOF
	set -x
	arc-snps-elf-gcc -o $HOME/tmp/kernel.img $HOME/tmp/kernel.c
}

build_target()
{
	build -n $numthreads_ $@
	printf "\033[1;32m>\033[0m done $WORKSPACE/Build\n"
}

setup_build_env()
{
	. $workspace_/venv/bin/activate

	PACKAGES_PATH=$PWD/edk2:$PWD/edk2-arc
	export PACKAGES_PATH

	export WORKSPACE=$workspace_
	export PATH=$PATH:$HOME/x-tools/arc-snps-elf/bin
	export GCC_ARC_PREFIX=arc-snps-elf-
	export GCC_AARCH64_PREFIX=aarch64-linux-gnu-
	export GCC_RISCV64_PREFIX=riscv64-linux-gnu-

	cd edk2
	. ./edksetup.sh BaseTools
}

mkdir -p $workspace_
setup_build_env

case $1 in
build-fd)
	gen_target_txt
	build_target -b DEBUG -t GCC -a ARC2 -p Platform/ARC/Hs4x/Hs4x.dsc
	;;
make-tools)
	make -C BaseTools/Source/C
	;;
clean)
	printf "\033[1;33m>\033[0m clean $WORKSPACE/Build\n"
	rm -fr $HOME/tmp/edk2/Build
	;;
update-venv)
	update_python_venv
	;;
list-venv)
	list_python_venv
	;;
make-kernel)
	build_dummy_kernel
	;;
*)
	print_help
	;;
esac
