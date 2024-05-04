#!/usr/bin/env bash
set -eu
script_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
root_path=$(realpath "$script_path/../")

"${script_path}/setup_web.sh"

# This is required to enable the web_sys clipboard API which eframe web uses
# https://rustwasm.github.io/wasm-bindgen/api/web_sys/struct.Clipboard.html
# https://rustwasm.github.io/docs/wasm-bindgen/web-sys/unstable-apis.html
export RUSTFLAGS=--cfg=web_sys_unstable_apis

OPTIMIZE=false
BUILD=debug
BUILD_FLAGS=""
WASM_OPT_FLAGS="-O2 --fast-math"

while test $# -gt 0; do
  case "$1" in
    -h|--help)
      echo "build_demo_web.sh [--release]"
      echo ""
      echo "  -g:        Keep debug symbols even with --release."
      echo "             These are useful profiling and size trimming."
      echo ""
      echo "  --open:    Open the result in a browser."
      echo ""
      echo "  --release: Build with --release, and then run wasm-opt."
      echo "             NOTE: --release also removes debug symbols, unless you also use -g."
      exit 0
      ;;

    -g)
      shift
      WASM_OPT_FLAGS="${WASM_OPT_FLAGS} -g"
      ;;

    --release)
      shift
      OPTIMIZE=true
      BUILD="release"
      BUILD_FLAGS="--release"
      ;;

    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

OUT_FILE_NAME="egui_client"

FINAL_WASM_DIR="${root_path}/typescript/webclient/src/gen"
FINAL_WASM_PATH="${FINAL_WASM_DIR}/egui_client_bg.wasm"

# Clear output from old stuff:
rm -f "${FINAL_WASM_PATH}"

echo "Building rust…"

(cd "$root_path/rust/egui-client/" &&
  cargo build \
    ${BUILD_FLAGS} \
    --quiet \
    --lib \
    --target wasm32-unknown-unknown \
    --no-default-features \
)

echo "Generating JS bindings for wasm…"
WASM_PATH="${root_path}/rust/target/wasm32-unknown-unknown/$BUILD/egui_client.wasm"
wasm-bindgen "${WASM_PATH}" --out-dir "${FINAL_WASM_DIR}" --out-name ${OUT_FILE_NAME} --omit-default-module-path --target web

# if this fails with "error: cannot import from modules (`env`) with `--no-modules`", you can use:
# wasm2wat target/wasm32-unknown-unknown/release/egui_demo_app.wasm | rg env
# wasm2wat target/wasm32-unknown-unknown/release/egui_demo_app.wasm | rg "call .now\b" -B 20 # What calls `$now` (often a culprit)
# Or use https://rustwasm.github.io/twiggy/usage/command-line-interface/paths.html#twiggy-paths

if [[ "${OPTIMIZE}" = true ]]; then
  echo "Optimizing wasm…"

  # to get wasm-strip:  apt/brew/dnf/pacman install wabt
  wasm-strip "${FINAL_WASM_PATH}"

  # to get wasm-opt:  apt/brew/dnf install binaryen
  wasm-opt "${FINAL_WASM_PATH}" ${WASM_OPT_FLAGS} -o "${FINAL_WASM_PATH}"
fi

echo "Finished ${FINAL_WASM_PATH}"
