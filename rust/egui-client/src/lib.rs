pub mod app;
pub mod freqctrl;
pub mod plotter;

#[cfg(target_arch = "wasm32")]
mod web;

#[cfg(target_arch = "wasm32")]
pub use web::*;
