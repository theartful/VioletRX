use std::env;

fn main() {
    println!(
        r"cargo:rustc-link-search=/home/theartful/Programming/violetrx/build/cpp/async_core_c/"
    );
    println!(r"cargo:rustc-link-search=/home/theartful/Programming/violetrx/build/cpp/async_core/");
    println!(r"cargo:rustc-link-search=/home/theartful/Programming/violetrx/build/cpp/core/");
    println!(r"cargo:rustc-link-search=/home/theartful/Programming/violetrx/build/cpp/dsp/");
}
