#!/usr/bin/env bash
# Builds the fuzz harnesses (sanitized libcore + libdrivers + codec modules), their standalone
# reproducers, and a plain (unsanitized) core doctest runner for mayhem/test.sh.
set -euo pipefail

[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
: "${SRC:=/mayhem}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

cd "$SRC"
OUT=/mayhem
MODS="bmp jpg regex svg tga webp"
SUF=linuxbsd.template_debug.x86_64.llvm

# Headless core-only configuration: only libcore, libdrivers and the codec/regex modules are built.
# tests=yes keeps TESTS_ENABLED consistent with the oracle build; GDScript/scene/servers stay out of
# the link (mayhem/stubs.cpp covers the handful of references core keeps into them).
COMMON=(platform=linuxbsd target=template_debug arch=x86_64 use_llvm=yes linker=lld tests=yes
	optimize=debug debug_symbols=no scu_build=yes deprecated=no disable_3d=yes disable_advanced_gui=yes
	disable_xr=yes x11=no wayland=no vulkan=no opengl3=no alsa=no pulseaudio=no dbus=no speechd=no
	fontconfig=no udev=no libdecor=no touch=no modules_enabled_by_default=no module_regex_enabled=yes
	module_text_server_fb_enabled=yes module_jpg_enabled=yes module_webp_enabled=yes
	module_svg_enabled=yes module_bmp_enabled=yes module_tga_enabled=yes
	use_static_cpp=no verbose=no progress=no)

lib_targets() {
	local suffix=$1
	printf '%s\n' "bin/obj/core/libcore.$SUF.$suffix.a" "bin/obj/drivers/libdrivers.$SUF.$suffix.a"
	for m in $MODS; do printf '%s\n' "bin/obj/modules/libmodule_$m.$SUF.$suffix.a"; done
}

# Preprocessor defines/includes the engine builds its sources with (taken from a verbose scons run).
DEFS=(-std=gnu++20 -fno-exceptions -ffp-contract=off -msse4.2 -mpopcnt -DDEBUG_ENABLED -DNDEBUG
	-DDISABLE_DEPRECATED -DSOWRAP_ENABLED -DXKB_ENABLED -DSDL_ENABLED -DLINUXBSD_ENABLED -DUNIX_ENABLED
	-D_FILE_OFFSET_BITS=64 -DACCESSKIT_DYNAMIC -DACCESSKIT_ENABLED -DCRASH_HANDLER_ENABLED -D_3D_DISABLED
	-DADVANCED_GUI_DISABLED -DPHYSICS_3D_DISABLED -DNAVIGATION_3D_DISABLED -DXR_DISABLED -DMINIZIP_ENABLED
	-DBROTLI_ENABLED -DTHREADS_ENABLED -DCLIPPER2_ENABLED -DZSTD_STATIC_LINKING_ONLY -DTESTS_ENABLED)
INCS=(-Ithirdparty/libpng -Ithirdparty/accesskit/include -Ithirdparty/zstd -Ithirdparty/zlib
	-Ithirdparty/clipper2/include -Ithirdparty/brotli/include -Ithirdparty/linuxbsd_headers
	-Iplatform/linuxbsd -I.)

# 1) Sanitized libraries (the fuzzed code is instrumented). -fno-sanitize=function: the engine calls
#    type-erased Variant helpers through mismatched function-pointer types by design.
SAN="$SANITIZER_FLAGS -fno-sanitize=function -fsanitize=fuzzer-no-link $DEBUG_FLAGS"
mapfile -t FUZZ_LIBS < <(lib_targets fuzz)
scons -j"$MAYHEM_JOBS" "${COMMON[@]}" extra_suffix=fuzz \
	ccflags="$SAN" linkflags="$SANITIZER_FLAGS -fsanitize=fuzzer-no-link" "${FUZZ_LIBS[@]}"

# 2) Harnesses: fuzzer + standalone reproducer each.
$CC $SANITIZER_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o /tmp/standalone_main.o
$CXX $SANITIZER_FLAGS -c mayhem/lsan_off.cc -o /tmp/lsan_off.o
$CXX "${DEFS[@]}" "${INCS[@]}" $SAN -c mayhem/stubs.cpp -o /tmp/stubs.o
GROUP=()
for l in "${FUZZ_LIBS[@]}"; do GROUP+=("$l"); done
TARGETS=$(cd mayhem/harnesses && ls fuzz_*.cpp | sed 's/\.cpp$//')
for t in $TARGETS; do
	$CXX "${DEFS[@]}" "${INCS[@]}" $SAN -c "mayhem/harnesses/$t.cpp" -o "/tmp/$t.o"
	$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -fuse-ld=lld $LIB_FUZZING_ENGINE "/tmp/$t.o" /tmp/lsan_off.o /tmp/stubs.o \
		-Wl,--start-group "${GROUP[@]}" -Wl,--end-group -lpthread -ldl -o "$OUT/$t"
	$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -fuse-ld=lld "/tmp/$t.o" /tmp/standalone_main.o /tmp/lsan_off.o /tmp/stubs.o \
		-Wl,--start-group "${GROUP[@]}" -Wl,--end-group -lpthread -ldl -o "$OUT/$t-standalone"
done

# 3) Test oracle: upstream's own doctest cases for the parsers, built with the project's normal flags.
mapfile -t ORACLE_LIBS < <(lib_targets oracle)
scons -j"$MAYHEM_JOBS" "${COMMON[@]}" extra_suffix=oracle ccflags="$COVERAGE_FLAGS" linkflags="$COVERAGE_FLAGS" "${ORACLE_LIBS[@]}"
$CXX "${DEFS[@]}" "${INCS[@]}" -Ithirdparty/doctest -O0 $COVERAGE_FLAGS -c mayhem/oracle_main.cpp -o /tmp/oracle_main.o
$CXX "${DEFS[@]}" "${INCS[@]}" -O0 $COVERAGE_FLAGS -c mayhem/stubs.cpp -o /tmp/stubs_oracle.o
$CXX -fuse-ld=lld $COVERAGE_FLAGS /tmp/oracle_main.o /tmp/stubs_oracle.o \
	-Wl,--start-group "${ORACLE_LIBS[@]}" -Wl,--end-group -lpthread -ldl -o "$OUT/redot-core-tests"
