#!/usr/bin/env bash
set -e

BUILD_DIR="build"
BUILD_TYPE="Debug"
TARGET="PikselDesktop"
JOBS=$(nproc 2>/dev/null || echo 1)

usage() {
  echo "Usage: $0 {build|run|clean}"
  exit 1
}

build() {
  echo "🔧 Building ($BUILD_TYPE)"
  mkdir -p "$BUILD_DIR"
  cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  cmake --build "$BUILD_DIR" -j"$JOBS"
}

run() {
  EXE="$BUILD_DIR/$TARGET"
  if [[ ! -x "$EXE" ]]; then
    echo "❌ Executable not found. Run './scripts/dev.sh build' first."
    exit 1
  fi

  echo "▶️ Running $EXE"
  "$EXE"
}

clean() {
  echo "🧹 Cleaning build directory"
  rm -rf "$BUILD_DIR"
}

case "${1:-}" in
  build)
    build
    ;;
  run)
    run
    ;;
  clean)
    clean
    ;;
  *)
    usage
    ;;
esac
