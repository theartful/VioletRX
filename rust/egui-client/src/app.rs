use crate::{
    freqctrl::FreqCtrl,
    plotter::{fftaverager::FftAverager, Plotter},
};
use eframe::{
    egui::{self, Widget},
    epaint::{Pos2, Rect, Vec2},
};

use violetrx_traits::{
    receiver::{ReceiverEvent, ReceiverEventData},
    FftFrame,
};

#[cfg(target_arch = "wasm32")]
use core::any::Any;

#[derive(Default)]
pub struct ReceiverState {
    pub is_playing: bool,
    pub input_rate: i32,
    pub decim: i32,
    pub center_freq: i64,
}

pub enum Event {
    ReceiverEvent(ReceiverEvent),
    NewFftFrame,
}

pub struct App {
    plotter: Plotter,
    egui_ctx: egui::Context,
    events_sender: std::sync::mpsc::Sender<Event>,
    events_receiver: std::sync::mpsc::Receiver<Event>,

    receiver_state: ReceiverState,

    fft_averager: FftAverager,
    fft_frame_gui: FftFrame,
}

impl App {
    pub fn new(ctx: egui::Context) -> Self {
        egui_extras::install_image_loaders(&ctx);

        let (events_sender, events_receiver) = std::sync::mpsc::channel();

        Self {
            plotter: Plotter::new(),
            egui_ctx: ctx,
            events_sender,
            events_receiver,

            receiver_state: ReceiverState::default(),

            fft_averager: FftAverager::new(30, 0.5),
            fft_frame_gui: FftFrame::default(),
        }
    }

    fn freq_from(&self) -> i64 {
        self.receiver_state.center_freq - self.receiver_state.input_rate as i64 / 2
    }

    fn freq_to(&self) -> i64 {
        self.receiver_state.center_freq + self.receiver_state.input_rate as i64 / 2
    }

    fn handle_rx_event(&mut self, rx_event: &ReceiverEvent) {
        match rx_event.data {
            ReceiverEventData::Started => {
                // TODO
            }
            ReceiverEventData::Stopped => {
                // TODO
            }
            ReceiverEventData::RfFreqChanged(freq) => {
                self.receiver_state.center_freq = freq;

                // update plotter
                let scene_viewport = self.plotter.scene_viewport;

                self.plotter.set_max_viewport(Rect::from_min_max(
                    Pos2::new(self.freq_from() as f32, scene_viewport.top()),
                    Pos2::new(self.freq_to() as f32, scene_viewport.bottom()),
                ));

                let old_center = scene_viewport.center().x;
                self.plotter
                    .translate_viewport(Vec2::new(freq as f32 - old_center, 0.0));
            }
            ReceiverEventData::InputRateChanged(rate) => {
                self.receiver_state.input_rate = rate;

                // update plotter
                let scene_viewport = self.plotter.scene_viewport;

                self.plotter.set_max_viewport(Rect::from_min_max(
                    Pos2::new(self.freq_from() as f32, scene_viewport.top()),
                    Pos2::new(self.freq_to() as f32, scene_viewport.bottom()),
                ));
            }
            ReceiverEventData::InputDecimChanged(decim) => {
                self.receiver_state.decim = decim;
            }
            _ => {}
        }
    }

    pub fn new_fft_frame(&mut self, fft_frame: FftFrame) {
        self.fft_frame_gui = fft_frame;
        self.fft_averager.step(&self.fft_frame_gui);
    }

    fn step(&mut self, event: &Event) {
        match event {
            Event::NewFftFrame => {}
            Event::ReceiverEvent(ref rx_event) => {
                self.handle_rx_event(rx_event);
            }
        }
    }

    fn draw_play_btn(&self, ui: &mut egui::Ui, _ctx: &egui::Context) {
        if self.receiver_state.is_playing {
            let play_btn_response = ui.add_sized(
                egui::Vec2::new(30.0, 30.0),
                egui::ImageButton::new(egui::Image::new(egui::include_image!(
                    "../resources/pause.svg"
                ))),
            );

            if play_btn_response.clicked() {
                // TODO
            }
        } else {
            let play_btn_response = ui.add_sized(
                egui::Vec2::new(30.0, 30.0),
                egui::ImageButton::new(egui::Image::new(egui::include_image!(
                    "../resources/play.svg"
                ))),
            );

            if play_btn_response.clicked() {
                // TODO
            }
        }
    }
}

impl eframe::App for App {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        while let Ok(event) = self.events_receiver.try_recv() {
            self.step(&event);
        }

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.vertical(|ui| {
                ui.horizontal(|ui| {
                    self.draw_play_btn(ui, ctx);
                    FreqCtrl::new(0, egui::Vec2::new(400.0, 30.0)).ui(ui);
                });

                let response = ui.add(&mut self.plotter);
                self.plotter.draw_fft(
                    &response,
                    &ui.painter_at(response.rect),
                    &self
                        .fft_averager
                        .decimate(response.rect, self.plotter.scene_viewport),
                );
            })
        });
    }

    #[cfg(target_arch = "wasm32")]
    fn as_any_mut(&mut self) -> Option<&mut dyn Any> {
        Some(&mut *self)
    }
}
