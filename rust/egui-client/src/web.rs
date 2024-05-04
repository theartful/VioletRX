#![allow(clippy::mem_forget)] // False positives from #[wasm_bindgen] macro

use crate::app::App;
use eframe::wasm_bindgen::{self, prelude::*};
use violetrx_traits::{FftFrame, Timestamp};

/// Our handle to the web app from JavaScript.
#[derive(Clone)]
#[wasm_bindgen]
pub struct WebHandle {
    runner: eframe::WebRunner,
}

#[wasm_bindgen]
impl WebHandle {
    /// Installs a panic hook, then returns.
    #[allow(clippy::new_without_default)]
    #[wasm_bindgen(constructor)]
    pub fn new() -> Self {
        // Redirect [`log`] message to `console.log` and friends:
        eframe::WebLogger::init(log::LevelFilter::Debug).ok();

        Self {
            runner: eframe::WebRunner::new(),
        }
    }

    /// Call this once from JavaScript to start your app.
    #[wasm_bindgen]
    pub async fn start(&self, canvas_id: &str) -> Result<(), wasm_bindgen::JsValue> {
        self.runner
            .start(
                canvas_id,
                eframe::WebOptions::default(),
                Box::new(|cc| Box::<App>::new(App::new(cc.egui_ctx.clone()))),
            )
            .await
    }

    #[wasm_bindgen]
    pub fn destroy(&self) {
        self.runner.destroy();
    }

    /// Example on how to call into your app from JavaScript.
    #[wasm_bindgen]
    pub fn example(&self) {
        if let Some(_app) = self.runner.app_mut::<App>() {
            // _app.example();
        }
    }

    /// The JavaScript can check whether or not your app has crashed:
    #[wasm_bindgen]
    pub fn has_panicked(&self) -> bool {
        self.runner.has_panicked()
    }

    #[wasm_bindgen]
    pub fn panic_message(&self) -> Option<String> {
        self.runner.panic_summary().map(|s| s.message())
    }

    #[wasm_bindgen]
    pub fn panic_callstack(&self) -> Option<String> {
        self.runner.panic_summary().map(|s| s.callstack())
    }

    #[wasm_bindgen]
    pub fn new_fft_frame(
        &self,
        data: &[f32],
        seconds: u64,
        nanos: u32,
        freq: i64,
        sample_rate: i32,
    ) {
        if let Some(mut app) = self.runner.app_mut::<App>() {
            app.new_fft_frame(FftFrame {
                data: data.iter().cloned().collect(),
                timestamp: Timestamp { seconds, nanos },
                freq,
                sample_rate,
            });
        }
    }
}
