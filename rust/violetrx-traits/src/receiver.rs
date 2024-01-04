use crate::error::VioletResult;
use num_derive::FromPrimitive;

#[derive(Debug, FromPrimitive, Clone, Copy)]
pub enum WindowType {
    Hamming = 0,
    Hann = 1,
    Blackman = 2,
    Rectangular = 3,
    Kaiser = 4,
    BlackmanHarris = 5,
    Bartlett = 6,
    FlatTop = 7,
    Nuttall = 8,
    NuttallCFD = 9,
    Welch = 10,
    Parzen = 11,
    Exponential = 12,
    Riemann = 13,
    Gaussian = 14,
    Tukey = 15,
}

#[derive(Clone, Debug)]
pub struct GainStage {
    pub name: String,
    pub start: f64,
    pub stop: f64,
    pub step: f64,
    pub value: f64,
}

#[derive(Default, Debug, Copy, Clone)]
pub struct Timestamp {
    pub seconds: u64,
    pub nanos: u32,
}

#[derive(Clone, Debug)]
pub enum ReceiverEventData {
    Unsubscribed,
    SyncStart,
    SyncEnd,
    Started,
    Stopped,
    InputDeviceChanged(String),
    AntennaChanged(String),
    InputRateChanged(i32),
    InputDecimChanged(i32),
    IqSwapChanged(bool),
    DcCancelChanged(bool),
    IqBalanceChanged(bool),
    RfFreqChanged(i64),
    GainStagesChanged(Vec<GainStage>),
    AntennasChanged(Vec<String>),
    AutoGainChanged(bool),
    GainChanged(String, f64),
    FreqCorrChanged(f64),
    FftSizeChanged(i32),
    FftWindowChanged(WindowType),
    VfoAdded,   // TODO
    VfoRemoved, // TODO
    Unknown,
}

#[derive(Clone, Debug)]
pub struct ReceiverEvent {
    pub id: i64,
    pub timestamp: Timestamp,
    pub data: ReceiverEventData,
}

#[derive(Default, Clone)]
pub struct FftFrame {
    pub data: Vec<f32>,
    pub timestamp: Timestamp,
    pub freq: i64,
    pub sample_rate: i32,
}

impl std::fmt::Debug for FftFrame {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "FftFrame {{ data: [f32; {}], timestamp: {:?}, freq: {}, sample_rate: {} }}",
            self.data.len(),
            self.timestamp,
            self.freq,
            self.sample_rate
        )
    }
}

#[async_trait::async_trait]
pub trait AsyncReceiver {
    fn new() -> Self;
    async fn subscribe(&self) -> VioletResult<()>;
    async fn start(&self) -> VioletResult<()>;
    async fn stop(&self) -> VioletResult<()>;
    async fn set_input_dev(&self, dev: &str) -> VioletResult<()>;
    async fn set_antenna(&self, antenna: &str) -> VioletResult<()>;
    async fn set_input_rate(&self, rate: i32) -> VioletResult<i32>;
    async fn set_input_decim(&self, rate: i32) -> VioletResult<i32>;
    async fn set_rf_freq(&self, freq: i64) -> VioletResult<i64>;
    async fn set_iq_swap(&self, enable: bool) -> VioletResult<()>;
    async fn set_dc_cancel(&self, enable: bool) -> VioletResult<()>;
    async fn set_iq_balance(&self, enable: bool) -> VioletResult<()>;
    async fn set_auto_gain(&self, enable: bool) -> VioletResult<()>;
    async fn set_gain(&self, name: &str, value: f64) -> VioletResult<f64>;
    async fn set_freq_corr(&self, ppm: f64) -> VioletResult<f64>;
    async fn set_fft_size(&self, fftsize: i32) -> VioletResult<()>;
    async fn set_fft_window(&self, window: WindowType) -> VioletResult<()>;
    async fn get_fft_data(&self, frame: &mut FftFrame) -> VioletResult<()>;

    fn connect(&self) -> Box<dyn futures::stream::Stream<Item = ReceiverEvent> + Send>;
}
