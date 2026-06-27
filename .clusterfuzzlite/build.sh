#!/bin/bash
set -eu
cd "${SRC:-.}"
: "${OUT:?OUT must be set}"
CXX="${CXX:-clang++}"
CXXFLAGS="${CXXFLAGS:-} -std=c++17 -Iinclude"
LIBS="src/core/slice.cpp src/core/string_util.cpp src/core/time.cpp src/core/checksum.cpp src/core/rule_catalog.cpp src/ingest/event.cpp src/ingest/parsers.cpp src/index/store.cpp src/query/query.cpp src/stream/decoder.cpp"
for target in ingest_fuzzer index_fuzzer query_fuzzer stream_fuzzer; do
  "$CXX" $CXXFLAGS ${LIB_FUZZING_ENGINE:-} fuzz/${target}.cc $LIBS -o "$OUT/${target}"
done
