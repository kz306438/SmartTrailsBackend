#!/usr/bin/env bash
set -euo pipefail

PACKAGE_NAME="myproject"
BUILD_DIR="build"
COMPILER="gcc"
COMPILER_VERSION=""
OS=""
ARCH="x86_64"
BUILD_TYPE="Debug"
GENERATOR=""
PROFILE=""
ASAN="False"
UBSAN="False"
EXTRA_WARNINGS="True"
EXTRA_LINK_OPTS="False"
RUN_TESTS="False"

CLEAN=0
JOBS=""
CONAN_BUILD_EXTRA_ARGS=()
CONAN_CONFIGS=()

usage() {
  cat <<EOF
Usage: $0 [options]

This script always drives the build via Conan: it runs
  conan install . --output-folder <build_dir> ...
  conan build .   --output-folder <build_dir> ...

Options:
  -c, --compiler <gcc|clang|msvc|mingw>     Compiler (default: gcc)
  -v, --compver <version>                   Compiler version (e.g. 12, 14, 19.29)
  -o, --os <Linux|Windows|Macos>            Target OS (default: autodetect)
  -a, --arch <x86_64|x86|armv8>             Target architecture (default: x86_64)
  -b, --build-type <Debug|Release|RelWithDebInfo>
                                            Build type (default: Debug)
  -g, --generator "<cmake-generator>"       Preferred CMake generator for the CMakeToolchain (will be
                                            forwarded to Conan as a config: -c tools.cmake.cmaketoolchain:generator=<GENERATOR>)
  --conf "key=value"                        Pass arbitrary Conan config entries (can be used multiple times)
                                            Example: --conf "tools.cmake.cmaketoolchain:sysroot=/path"
  --profile <profile>                       Use conan profile (optional)
  --asan / --no-asan                        Enable/disable AddressSanitizer (default: off)
  --ubsan / --no-ubsan                      Enable/disable UndefinedBehaviorSanitizer (default: off)
  --extra-warnings / --no-extra-warnings   Enable/disable extra warnings (default: on)
  --extra-link-opts / --no-extra-link-opts Enable/disable extra linker opts (default: off)
  --run-test / --run-tests / --no-run-tests Enable or disable running tests after build (default: disabled)
  --build-dir <dir>                         Build directory (default: build)
  --clean                                   Remove build dir before building
  -j, --jobs <N>                            Number of build jobs (defaults to CPU cores)
  --conan-build-args "<args>"               Extra arguments to append to `conan build` (quoted)
  -h, --help                                Show this help and exit

Examples:
  $0 --compiler gcc -v 12 -b Release --profile release_profile
  $0 -c msvc -v 19.36 -g "Visual Studio 17 2022" --conf "tools.cmake.cmaketoolchain:generator=Visual Studio 17 2022" \
     --conan-build-args "--build-requires"

  # Run tests after build
  $0 --run-test

EOF
}

# detect cpu cores
detect_cores() {
  if command -v nproc >/dev/null 2>&1; then
    nproc
  else
    sysctl -n hw.ncpu 2>/dev/null || echo 2
  fi
}

# parse args
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    -c|--compiler) COMPILER="$2"; shift 2;;
    -v|--compver) COMPILER_VERSION="$2"; shift 2;;
    -o|--os) OS="$2"; shift 2;;
    -a|--arch) ARCH="$2"; shift 2;;
    -b|--build-type) BUILD_TYPE="$2"; shift 2;;
    -g|--generator) GENERATOR="$2"; shift 2;;
    --conf) CONAN_CONFIGS+=("$2"); shift 2;;
    --profile) PROFILE="$2"; shift 2;;
    --asan) ASAN="True"; shift 1;;
    --no-asan) ASAN="False"; shift 1;;
    --ubsan) UBSAN="True"; shift 1;;
    --no-ubsan) UBSAN="False"; shift 1;;
    --extra-warnings) EXTRA_WARNINGS="True"; shift 1;;
    --no-extra-warnings) EXTRA_WARNINGS="False"; shift 1;;
    --extra-link-opts) EXTRA_LINK_OPTS="True"; shift 1;;
    --no-extra-link-opts) EXTRA_LINK_OPTS="False"; shift 1;;
    --run-test|--run-tests) RUN_TESTS="True"; shift 1;;
    --no-run-tests) RUN_TESTS="False"; shift 1;;
    --build-dir) BUILD_DIR="$2"; shift 2;;
    --clean) CLEAN=1; shift 1;;
    -j|--jobs) JOBS="$2"; shift 2;;
    --conan-build-args) 
        # split provided string into array
        read -r -a _arr <<< "$2"
        for v in "${_arr[@]}"; do CONAN_BUILD_EXTRA_ARGS+=("$v"); done
        shift 2;;
    -h|--help) usage; exit 0;;
    *) echo "Unknown option: $1"; usage; exit 1;;
  esac
done

# Autodetect OS if not specified
if [[ -z "$OS" ]]; then
  uname_out="$(uname -s)"
  case "${uname_out}" in
    Linux*)     OS="Linux";;
    Darwin*)    OS="Macos";;
    MINGW*|MSYS*|CYGWIN*) OS="Windows";;
    *) OS="$uname_out";;
  esac
fi

# Summary
echo "=== Build summary ==="
echo "package:        $PACKAGE_NAME"
echo "compiler:       $COMPILER ${COMPILER_VERSION:+(version $COMPILER_VERSION)}"
echo "os:             $OS"
echo "arch:           $ARCH"
echo "build_type:     $BUILD_TYPE"
echo "asan:           $ASAN"
echo "ubsan:          $UBSAN"
echo "extra_warnings: $EXTRA_WARNINGS"
echo "extra_link_opts:$EXTRA_LINK_OPTS"
echo "run_tests:      $RUN_TESTS"
echo "cmake generator:$GENERATOR"
echo "conan profile:  $PROFILE"
echo "build dir:      $BUILD_DIR"
echo "clean:          $CLEAN"
echo "jobs:           ${JOBS:-(auto)}"
echo "conan configs:  ${CONAN_CONFIGS[*]:-(none)}"
echo "extra conan build args: ${CONAN_BUILD_EXTRA_ARGS[*]:-(none)}"
echo "====================="

# Preconditions
if ! command -v conan >/dev/null 2>&1; then
  echo "Error: conan is not in PATH. Install Conan 2.x and try again." >&2
  exit 2
fi
if ! command -v cmake >/dev/null 2>&1; then
  echo "Error: cmake is not in PATH. Install CMake and try again." >&2
  exit 2
fi

# clean
if [[ $CLEAN -eq 1 ]]; then
  echo "Cleaning build dir: $BUILD_DIR"
  rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"

# prepare conan settings
CONAN_SETTINGS=()
CONAN_SETTINGS+=( -s "compiler=${COMPILER}" )
if [[ -n "$COMPILER_VERSION" ]]; then
  CONAN_SETTINGS+=( -s "compiler.version=${COMPILER_VERSION}" )
fi
CONAN_SETTINGS+=( -s "os=${OS}" )
CONAN_SETTINGS+=( -s "arch=${ARCH}" )
CONAN_SETTINGS+=( -s "build_type=${BUILD_TYPE}" )

PROFILE_ARG=()
if [[ -n "$PROFILE" ]]; then
  PROFILE_ARG=( --profile "$PROFILE" )
fi

# package options (Conan option format)
O_ASAN="${PACKAGE_NAME}/*:asan=${ASAN}"
O_UBSAN="${PACKAGE_NAME}/*:ubsan=${UBSAN}"
O_EXTRA_WARN="${PACKAGE_NAME}/*:extra_warnings=${EXTRA_WARNINGS}"
O_EXTRA_LINK="${PACKAGE_NAME}/*:extra_link_time_opts=${EXTRA_LINK_OPTS}"
O_RUN_TESTS="${PACKAGE_NAME}/*:run_tests=${RUN_TESTS}"

# Build conan config args (-c)
CONAN_CONF_ARGS=()
if [[ -n "$GENERATOR" ]]; then
  CONAN_CONF_ARGS+=( -c "tools.cmake.cmaketoolchain:generator=${GENERATOR}" )
fi
for c in "${CONAN_CONFIGS[@]}"; do
  CONAN_CONF_ARGS+=( -c "$c" )
done

# detect jobs
if [[ -z "$JOBS" ]]; then
  JOBS=$(detect_cores)
fi

# Run conan install
echo "Running conan install..."
conan install . \
  --output-folder "$BUILD_DIR" \
  --build=missing \
  "${PROFILE_ARG[@]}" \
  "${CONAN_SETTINGS[@]}" \
  "${CONAN_CONF_ARGS[@]}" \
  -o "$O_ASAN" -o "$O_UBSAN" -o "$O_EXTRA_WARN" -o "$O_EXTRA_LINK" -o "$O_RUN_TESTS"

# Run conan build (always)
echo "Running conan build (Conan recipe build):"
CONAN_BUILD_CMD=( conan build . --output-folder "$BUILD_DIR" )
if [[ -n "${PROFILE_ARG[*]}" ]]; then
  CONAN_BUILD_CMD+=( "${PROFILE_ARG[@]}" )
fi
if [[ -n "${CONAN_CONF_ARGS[*]}" ]]; then
  CONAN_BUILD_CMD+=( "${CONAN_CONF_ARGS[@]}" )
fi
CONAN_BUILD_CMD+=( "${CONAN_SETTINGS[@]}" )
CONAN_BUILD_CMD+=( -o "$O_ASAN" -o "$O_UBSAN" -o "$O_EXTRA_WARN" -o "$O_EXTRA_LINK" -o "$O_RUN_TESTS" )
if [[ ${#CONAN_BUILD_EXTRA_ARGS[@]} -gt 0 ]]; then
  CONAN_BUILD_CMD+=( "${CONAN_BUILD_EXTRA_ARGS[@]}" )
fi

echo "${CONAN_BUILD_CMD[*]}"
"${CONAN_BUILD_CMD[@]}"

echo "Build finished. Artifacts in: $BUILD_DIR"

# copy compile_commands.json if produced
if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
    echo ">>> compile_commands.json found in $BUILD_DIR; copying to project root"
    cp "$BUILD_DIR/compile_commands.json" . || true
fi