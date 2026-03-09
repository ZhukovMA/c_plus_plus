#!/usr/bin/env bash
set -euo pipefail

export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HEADER_DIR="$ROOT_DIR/6_01"
MODULE_DIR="$ROOT_DIR/6_02"

CXX="g++-15"

run_time() {
  echo
  echo ">>> $*"
  if command -v gtime >/dev/null 2>&1; then
    gtime -f 'real %e s' "$@"
  else
    /usr/bin/time -p "$@"
  fi
}

file_size() {
  stat -f%z "$1"
}

clean_header_build() {
  rm -f "$HEADER_DIR"/*.o "$HEADER_DIR"/header_app
}

clean_module_build() {
  rm -f "$MODULE_DIR"/*.o "$MODULE_DIR"/module_app
  rm -rf "$MODULE_DIR"/gcm.cache
}

build_header_step_by_step() {
  echo
  echo "6_01 Launch"
  cd "$HEADER_DIR"
  clean_header_build

  run_time "$CXX" -std=c++20 -Wall -Wextra -pedantic -c rational.cpp -o rational.o
  run_time "$CXX" -std=c++20 -Wall -Wextra -pedantic -c main.cpp -o main.o
  run_time "$CXX" rational.o main.o -o header_app

  echo
  echo "Size of rational.o: $(file_size rational.o) bytes"
  echo "Size of main.o: $(file_size main.o) bytes"
  echo "Size of header_app: $(file_size header_app) bytes"

  echo
  echo "execute header_app"
  ./header_app
}

build_header_clean_total() {
  echo
  echo "6_01 clean total build"
  clean_header_build
  run_time bash -lc "cd '$HEADER_DIR' && '$CXX' -std=c++20 -Wall -Wextra -pedantic -c rational.cpp -o rational.o && '$CXX' -std=c++20 -Wall -Wextra -pedantic -c main.cpp -o main.o && '$CXX' rational.o main.o -o header_app"
}

build_module_step_by_step() {
  echo
  echo "6_02 Launch"
  cd "$MODULE_DIR"
  clean_module_build

  echo
  echo "build standard header units"
  run_time bash -lc "cd '$MODULE_DIR' && for h in array cassert cmath compare ios iosfwd iostream istream limits numeric ostream sstream stdexcept tuple utility; do '$CXX' -std=c++20 -fmodules-ts -x c++-system-header \"\$h\" >/dev/null 2>&1; done"

  run_time "$CXX" -std=c++20 -fmodules-ts -Wall -Wextra -pedantic -x c++ -c rational.cxx -o rational_iface.o
  run_time "$CXX" -std=c++20 -fmodules-ts -Wall -Wextra -pedantic -c rational.cpp -o rational_impl.o
  run_time "$CXX" -std=c++20 -fmodules-ts -Wall -Wextra -pedantic -c main.cpp -o main.o
  run_time "$CXX" rational_iface.o rational_impl.o main.o -o module_app

  echo
  echo "Size of rational_iface.o: $(file_size rational_iface.o) bytes"
  echo "Size of rational_impl.o: $(file_size rational_impl.o) bytes"
  echo "Size of main.o: $(file_size main.o) bytes"
  echo "Size of module_app: $(file_size module_app) bytes"

  if [ -d gcm.cache ]; then
    echo "Size of gcm.cache: $(du -sk gcm.cache | awk '{print $1}') KiB"
  fi

  echo
  echo "execute module_app"
  ./module_app
}

build_module_clean_total() {
  echo
  echo "6_02 clean total build"
  clean_module_build
  run_time bash -lc "cd '$MODULE_DIR' && for h in array cassert cmath compare ios iosfwd iostream istream limits numeric ostream sstream stdexcept tuple utility; do '$CXX' -std=c++20 -fmodules-ts -x c++-system-header \"\$h\" >/dev/null 2>&1; done && '$CXX' -std=c++20 -fmodules-ts -Wall -Wextra -pedantic -x c++ -c rational.cxx -o rational_iface.o && '$CXX' -std=c++20 -fmodules-ts -Wall -Wextra -pedantic -c rational.cpp -o rational_impl.o && '$CXX' -std=c++20 -fmodules-ts -Wall -Wextra -pedantic -c main.cpp -o main.o && '$CXX' rational_iface.o rational_impl.o main.o -o module_app"
}

build_header_step_by_step
build_header_clean_total

build_module_step_by_step
build_module_clean_total