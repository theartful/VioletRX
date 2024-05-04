#!/usr/bin/env bash
set -eu
script_path=$(cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)
root_path=$(realpath "$script_path/../")

cd "$root_path/rust/egui-client/"

# Pre-requisites:
rustup target add wasm32-unknown-unknown

# For generating JS bindings:
cargo install --quiet wasm-bindgen-cli --version 0.2.90
