use eframe::{
    egui::{self, Widget},
    epaint::{Pos2, Rect, Vec2},
};
use egui_client::{
    freqctrl::FreqCtrl,
    plotter::{fftaverager::FftAverager, Plotter},
};
use std::sync::Arc;
use tokio::{runtime, sync::Mutex, task::JoinHandle};

use egui_extras::image::RetainedImage;
use futures::stream::StreamExt;
use violetrx_c::CAsyncReceiver;
use violetrx_traits::{
    receiver::{ReceiverEvent, ReceiverEventData},
    AsyncReceiver, FftFrame,
};

fn main() -> Result<(), eframe::Error> {
    let options = eframe::NativeOptions {
        initial_window_size: Some(egui::vec2(320.0, 240.0)),
        ..Default::default()
    };

    eframe::run_native(
        "My egui App",
        options,
        Box::new(|cc| {
            let mut app = Box::<MyApp>::new(MyApp::new(cc.egui_ctx.clone()));
            app.set_input_dev("file=/home/theartful/IQData/interesting,freq=100e6,rate=14.122e6,repeat=true,throttle=true");
            app.subscribe();
            app
        }),
    )
}

#[derive(Default)]
pub struct ReceiverState {
    pub is_playing: bool,
    pub input_rate: i32,
    pub decim: i32,
    pub center_freq: i64,
}

enum Event {
    ReceiverEvent(ReceiverEvent),
    NewFftFrame,
}

struct MyApp {
    plotter: Plotter,
    tokio_rt: runtime::Runtime,
    egui_ctx: egui::Context,
    violet_receiver: CAsyncReceiver,
    events_sender: std::sync::mpsc::Sender<Event>,
    events_receiver: std::sync::mpsc::Receiver<Event>,
    play_icon: egui_extras::image::RetainedImage,
    pause_icon: egui_extras::image::RetainedImage,

    receiver_state: ReceiverState,

    fft_averager: FftAverager,
    fft_fetcher_handle: Option<JoinHandle<()>>,
    fft_frame_shared: Arc<Mutex<FftFrame>>,
    fft_frame_gui: FftFrame,
}

impl MyApp {
    pub fn new(ctx: egui::Context) -> Self {
        let play_icon: RetainedImage =
            RetainedImage::from_svg_bytes("play_icon", include_bytes!("../resources/play.svg"))
                .unwrap();

        let pause_icon: RetainedImage =
            RetainedImage::from_svg_bytes("paused_icon", include_bytes!("../resources/pause.svg"))
                .unwrap();

        let (events_sender, events_receiver) = std::sync::mpsc::channel();

        Self {
            plotter: Plotter::new(),
            tokio_rt: runtime::Builder::new_multi_thread()
                .enable_all()
                .build()
                .unwrap(),
            egui_ctx: ctx,
            violet_receiver: CAsyncReceiver::new(),
            events_sender,
            events_receiver,
            play_icon,
            pause_icon,

            receiver_state: ReceiverState::default(),

            fft_averager: FftAverager::new(30, 0.5),
            fft_fetcher_handle: Option::None,
            fft_frame_shared: Arc::new(Mutex::new(FftFrame::default())),
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
                self.receiver_state.is_playing = true;

                self.fft_fetcher_handle = Some(self.periodically_fetch_fft_data(
                    std::time::Duration::from_millis(1000 / self.fft_averager.rate as u64),
                ));
            }
            ReceiverEventData::Stopped => {
                self.receiver_state.is_playing = false;

                if let Some(ref mut handle) = self.fft_fetcher_handle.take() {
                    handle.abort();
                }
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

    fn step(&mut self, event: &Event) {
        match event {
            Event::NewFftFrame => {
                // we get the new fft data from the tokio task, while replacing
                // it with the gui fft frame to reuse memory
                {
                    let mut fft_frame = self.fft_frame_shared.blocking_lock();
                    std::mem::swap(&mut self.fft_frame_gui, &mut *fft_frame);
                }
                self.fft_averager.step(&self.fft_frame_gui);
            }
            Event::ReceiverEvent(ref rx_event) => {
                self.handle_rx_event(rx_event);
            }
        }
    }

    pub fn subscribe(&mut self) -> JoinHandle<()> {
        let violet_receiver = self.violet_receiver.clone();
        let egui_ctx = self.egui_ctx.clone();

        let events_sender = self.events_sender.clone();

        self.tokio_rt.spawn(async move {
            let mut stream = Box::into_pin(violet_receiver.connect());

            while let Some(event) = stream.next().await {
                println!("event = {:?}", event);
                let _ = events_sender.send(Event::ReceiverEvent(event));

                // request repaint
                egui_ctx.request_repaint();
            }
        })
    }

    pub fn set_input_dev(&self, dev: &str) -> JoinHandle<()> {
        let violet_receiver = self.violet_receiver.clone();
        let dev = String::from(dev);

        self.tokio_rt.spawn(async move {
            let _ = violet_receiver.set_input_dev(&dev).await;
        })
    }

    fn start(&self) -> JoinHandle<()> {
        let violet_receiver = self.violet_receiver.clone();
        self.tokio_rt.spawn(async move {
            let _ = violet_receiver.start().await;
        })
    }

    fn stop(&self) -> JoinHandle<()> {
        let violet_receiver = self.violet_receiver.clone();
        self.tokio_rt.spawn(async move {
            let _ = violet_receiver.stop().await;
        })
    }

    fn periodically_fetch_fft_data(&self, period: std::time::Duration) -> JoinHandle<()> {
        let fft_frame_shared = self.fft_frame_shared.clone();
        let violet_receiver = self.violet_receiver.clone();
        let egui_ctx = self.egui_ctx.clone();
        let events_sender = self.events_sender.clone();

        self.tokio_rt.spawn(async move {
            // we do triple buffering for the fft frames
            // this is the first buffer, which is sent to the c side of things
            // then we lock the second buffer which is shared between the gui
            // and this tokio task, and replace it with the first buffer
            // then the egui thread holds the lock and replaces the shared buffer
            // with its buffer
            //
            // this ensures that the lock is held for very little time
            let mut other_fft_frame = FftFrame::default();
            loop {
                match violet_receiver.get_fft_data(&mut other_fft_frame).await {
                    Ok(()) => {
                        if let Ok(_) = events_sender.send(Event::NewFftFrame) {
                            // holding the lock for the least amount of time possible
                            {
                                let mut fft_frame = fft_frame_shared.lock().await;
                                std::mem::swap(&mut other_fft_frame, &mut *fft_frame);
                            }
                            // request repaint
                            egui_ctx.request_repaint();
                        }
                    }
                    Err(err) => {
                        eprintln!("ERROR periodically_fetch_fft_data: {:?}", err);
                    }
                }
                // sleep while not holding the lock!
                tokio::time::sleep(period).await;
            }
        })
    }

    fn draw_play_btn(&self, ui: &mut egui::Ui, ctx: &egui::Context) {
        if self.receiver_state.is_playing {
            let play_btn_response = egui::ImageButton::new(
                self.pause_icon.texture_id(ctx),
                egui::Vec2::new(30.0, 30.0),
            )
            .ui(ui);

            if play_btn_response.clicked() {
                self.stop();
            }
        } else {
            let play_btn_response =
                egui::ImageButton::new(self.play_icon.texture_id(ctx), egui::Vec2::new(30.0, 30.0))
                    .ui(ui);

            if play_btn_response.clicked() {
                self.start();
            }
        }
    }
}

impl eframe::App for MyApp {
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
}
