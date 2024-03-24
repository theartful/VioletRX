use libc::{c_char, c_float, c_int, c_void};
use num_derive::FromPrimitive;
use num_traits::FromPrimitive;
use std::ffi::{c_double, CStr, CString};
use std::ptr::null;
use std::sync::Mutex;
use violetrx_traits::receiver::{
    AsyncReceiver, FftFrame, GainStage, ReceiverEvent, ReceiverEventData, Timestamp, WindowType,
};
use violetrx_traits::{VioletError, VioletResult};

type CVioletReceiverHandle = *const c_void;

#[repr(C)]
struct CVioletTimestamp {
    seconds: u64,
    nanos: u32,
}

#[repr(C)]
struct CVioletGainStage {
    name: *const c_char,
    start: c_double,
    stop: c_double,
    step: c_double,
    value: c_double,
}

#[allow(dead_code)]
#[repr(C)]
#[derive(Clone, Copy, FromPrimitive)]
enum CVioletWindowType {
    Hamming = 0,
    Hann = 1,
    Blackman = 2,
    Rectangular = 3,
    Kaiser = 4,
    BlackmanHarris = 5,
    Bartlett = 6,
    Flattop = 7,
    Nuttall = 8,
    NuttallCFD = 9,
    Welch = 10,
    Parzen = 11,
    Exponential = 12,
    Riemann = 13,
    Gaussian = 14,
    Tukey = 15,
}

#[allow(dead_code)]
#[repr(C)]
#[derive(Clone, Copy, FromPrimitive)]
enum CVioletFilterShape {
    Off = 0,
    Raw = 1,
    Am = 2,
    AmSync = 3,
    Lsb = 4,
    Usb = 5,
    Cwl = 6,
    Cwu = 7,
    Nfm = 8,
    WfmMono = 9,
    WfmStereo = 10,
    WfmStereoOirt = 11,
    Last = 12,
}

#[allow(dead_code)]
#[repr(C)]
#[derive(Clone, Copy, FromPrimitive)]
enum CVioletDemod {
    Soft = 0,
    Normal = 1,
    Sharp = 2,
}

#[allow(dead_code)]
#[repr(C)]
#[derive(Clone, Copy, PartialEq, Debug)]
enum CVioletEventType {
    // receiver events
    Unsubscribed,
    SyncStart,
    SyncEnd,
    Started,
    Stopped,
    InputDeviceChanged,
    AntennaChanged,
    InputRateChanged,
    InputDecimChanged,
    IqSwapChanged,
    DcCancelChanged,
    IqBalanceChanged,
    RfFreqChanged,
    GainStagesChanged,
    AntennasChanged,
    AutoGainChanged,
    GainChanged,
    FreqCorrChanged,
    FftSizeChanged,
    FftWindowChanged,
    IqRecordingStarted,
    IqRecordingStopped,

    // vfo events
    VfoSyncStart,
    VfoSyncEnd,
    VfoAdded,
    VfoRemoved,
    VfoDemodChanged,
    VfoOffsetChanged,
    VfoCwOffsetChanged,
    VfoFilterChanged,
    VfoNoiseBlankerOnChanged,
    VfoNoiseBlankerThresholdChanged,
    VfoSqlLevelChanged,
    VfoSqlAlphaChanged,
    VfoAgcOnChanged,
    VfoAgcHangChanged,
    VfoAgcThresholdChanged,
    VfoAgcSlopeChanged,
    VfoAgcDecayChanged,
    VfoAgcManualGainChanged,
    VfoFmMaxdevChanged,
    VfoFmDeemphChanged,
    VfoAmDcrChanged,
    VfoAmSyncDcrChanged,
    VfoAmSyncPllBwChanged,
    VfoRecordingStarted,
    VfoRecordingStopped,
    VfoSnifferStarted,
    VfoSnifferStopped,
    VfoUdpStreamingStarted,
    VfoUdpStreamingStopped,
    VfoRdsDecoderStarted,
    VfoRdsDecoderStopped,
    VfoRdsParserReset,
    VfoAudioGainChanged,

    EventUnknown,
}

#[repr(C)]
struct CVioletEventCommon {
    event_type: CVioletEventType,
    id: i64,
    timestamp: CVioletTimestamp,
}

#[repr(C)]
struct CVioletVfoEventCommon {
    event_type: CVioletEventType,
    id: i64,
    timestamp: CVioletTimestamp,
    handle: u64,
}

#[repr(C)]
struct CVioletSyncStart {
    base: CVioletEventCommon,
}

#[repr(C)]
struct CVioletSyncEnd {
    base: CVioletEventCommon,
}

#[repr(C)]
struct CVioletStarted {
    base: CVioletEventCommon,
}

#[repr(C)]
struct CVioletStopped {
    base: CVioletEventCommon,
}

#[repr(C)]
struct CVioletInputDeviceChanged {
    base: CVioletEventCommon,
    device: *const c_char,
}

#[repr(C)]
struct CVioletAntennasChanged {
    base: CVioletEventCommon,
    size: i32,
    antennas: *const *const c_char,
}

#[repr(C)]
struct CVioletAntennaChanged {
    base: CVioletEventCommon,
    antenna: *const c_char,
}

#[repr(C)]
struct CVioletInputRateChanged {
    base: CVioletEventCommon,
    input_rate: i32,
}

#[repr(C)]
struct CVioletInputDecimChanged {
    base: CVioletEventCommon,
    decim: i32,
}

#[repr(C)]
struct CVioletIqSwapChanged {
    base: CVioletEventCommon,
    enabled: bool,
}

#[repr(C)]
struct CVioletDcCancelChanged {
    base: CVioletEventCommon,
    enabled: bool,
}

#[repr(C)]
struct CVioletIqBalanceChanged {
    base: CVioletEventCommon,
    enabled: bool,
}

#[repr(C)]
struct CVioletRfFreqChanged {
    base: CVioletEventCommon,
    freq: i64,
}

#[repr(C)]
struct CVioletGainStagesChanged {
    base: CVioletEventCommon,
    size: i32,
    stages: *const CVioletGainStage,
}

#[repr(C)]
struct CVioletAutoGainChanged {
    base: CVioletEventCommon,
    enabled: bool,
}

#[repr(C)]
struct CVioletGainChanged {
    base: CVioletEventCommon,
    name: *const c_char,
    value: c_double,
}

#[repr(C)]
struct CVioletFreqCorrChanged {
    base: CVioletEventCommon,
    ppm: c_double,
}

#[repr(C)]
struct CVioletFftSizeChanged {
    base: CVioletEventCommon,
    size: i32,
}

#[repr(C)]
struct CVioletFftWindowChanged {
    base: CVioletEventCommon,
    window: CVioletWindowType,
}

#[repr(C)]
struct CVioletIqRecordingStarted {
    base: CVioletEventCommon,
    path: *const c_char,
}

#[repr(C)]
struct CVioletIqRecordingStopped {
    base: CVioletEventCommon,
}

#[repr(C)]
struct CVioletVfoSyncStart {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletVfoSyncEnd {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletVfoAdded {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletVfoRemoved {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletDemodChanged {
    base: CVioletVfoEventCommon,
    demod: CVioletDemod,
}

#[repr(C)]
struct CVioletOffsetChanged {
    base: CVioletVfoEventCommon,
    offset: i32,
}

#[repr(C)]
struct CVioletCwOffsetChanged {
    base: CVioletVfoEventCommon,
    offset: i32,
}

#[repr(C)]
struct CVioletFilterChanged {
    base: CVioletVfoEventCommon,
    shape: CVioletFilterShape,
    low: i32,
    high: i32,
}

#[repr(C)]
struct CVioletNoiseBlankerOnChanged {
    base: CVioletVfoEventCommon,
    nb_id: i32,
    enabled: bool,
}

#[repr(C)]
struct CVioletNoiseBlankerThresholdChanged {
    base: CVioletVfoEventCommon,
    nb_id: i32,
    threshold: c_float,
}

#[repr(C)]
struct CVioletSqlLevelChanged {
    base: CVioletVfoEventCommon,
    level: c_double,
}

#[repr(C)]
struct CVioletSqlAlphaChanged {
    base: CVioletVfoEventCommon,
    alpha: c_double,
}

#[repr(C)]
struct CVioletAgcOnChanged {
    base: CVioletVfoEventCommon,
    enabled: bool,
}

#[repr(C)]
struct CVioletAgcHangChanged {
    base: CVioletVfoEventCommon,
    enabled: bool,
}

#[repr(C)]
struct CVioletAgcThresholdChanged {
    base: CVioletVfoEventCommon,
    threshold: i32,
}

#[repr(C)]
struct CVioletAgcSlopeChanged {
    base: CVioletVfoEventCommon,
    slope: i32,
}

#[repr(C)]
struct CVioletAgcDecayChanged {
    base: CVioletVfoEventCommon,
    decay: i32,
}

#[repr(C)]
struct CVioletAgcManualGainChanged {
    base: CVioletVfoEventCommon,
    gain: i32,
}

#[repr(C)]
struct CVioletFmMaxDevChanged {
    base: CVioletVfoEventCommon,
    maxdev: c_float,
}

#[repr(C)]
struct CVioletFmDeemphChanged {
    base: CVioletVfoEventCommon,
    tau: c_double,
}

#[repr(C)]
struct CVioletAmDcrChanged {
    base: CVioletVfoEventCommon,
    enabled: bool,
}

#[repr(C)]
struct CVioletAmSyncDcrChanged {
    base: CVioletVfoEventCommon,
    enabled: bool,
}

#[repr(C)]
struct CVioletAmSyncPllBwChanged {
    base: CVioletVfoEventCommon,
    bw: c_float,
}

#[repr(C)]
struct CVioletRecordingStarted {
    base: CVioletVfoEventCommon,
    path: *const c_char,
}

#[repr(C)]
struct CVioletRecordingStopped {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletSnifferStarted {
    base: CVioletVfoEventCommon,
    sample_rate: i32,
    buffsize: i32,
}

#[repr(C)]
struct CVioletSnifferStopped {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletRdsDecoderStarted {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletRdsDecoderStopped {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletRdsParserReset {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletUdpStreamingStarted {
    base: CVioletVfoEventCommon,
    host: *const c_char,
    port: i32,
    stereo: bool,
}

#[repr(C)]
struct CVioletUdpStreamingStopped {
    base: CVioletVfoEventCommon,
}

#[repr(C)]
struct CVioletAudioGainChanged {
    base: CVioletVfoEventCommon,
    gain: c_float,
}

type CVioletVoidCallback = extern "C" fn(c_int, *const c_void);
type CVioletIntCallback = extern "C" fn(c_int, c_int, *const c_void);
type CVioletInt64Callback = extern "C" fn(c_int, i64, *const c_void);
type CVioletDoubleCallback = extern "C" fn(c_int, c_double, *const c_void);
type CVioletFftDataCallback =
    extern "C" fn(c_int, CVioletTimestamp, i64, c_int, *mut c_float, c_int, *const c_void);
type CVioletEventCallback = extern "C" fn(*const CVioletEventCommon, *const c_void);
type CVioletVfoEventCallback = extern "C" fn(*const CVioletVfoEventCommon, *const c_void);

#[link(name = "async_core_c")]
extern "C" {
    fn violet_rx_init() -> *const c_void;
    fn violet_rx_new_ref(rx: CVioletReceiverHandle) -> *const c_void;
    fn violet_rx_destroy(rx: CVioletReceiverHandle);
    fn violet_rx_start(
        rx: CVioletReceiverHandle,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
    fn violet_rx_stop(
        rx: CVioletReceiverHandle,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_input_rate(
        rx: CVioletReceiverHandle,
        samplerate: c_int,
        callback: CVioletIntCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_input_decim(
        rx: CVioletReceiverHandle,
        decim: c_int,
        callback: CVioletIntCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_rf_freq(
        rx: CVioletReceiverHandle,
        freq: i64,
        callback: CVioletInt64Callback,
        userdata: *const c_void,
    );
    fn violet_rx_set_input_dev(
        rx: CVioletReceiverHandle,
        dev: *const c_char,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
    fn violet_rx_get_fft_data(
        rx: CVioletReceiverHandle,
        data: *mut c_float,
        size: c_int,
        callback: CVioletFftDataCallback,
        userdata: *const c_void,
    );
    fn violet_rx_subscribe(
        rx: CVioletReceiverHandle,
        callback: CVioletEventCallback,
        userdata: *const c_void,
    ) -> *const c_void;
    fn violet_rx_set_iq_swap(
        rx: CVioletReceiverHandle,
        enable: bool,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_dc_cancel(
        rx: CVioletReceiverHandle,
        enable: bool,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_iq_balance(
        rx: CVioletReceiverHandle,
        enable: bool,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_auto_gain(
        rx: CVioletReceiverHandle,
        enable: bool,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_gain(
        rx: CVioletReceiverHandle,
        gain_name: *const c_char,
        value: c_double,
        callback: CVioletDoubleCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_freq_corr(
        rx: CVioletReceiverHandle,
        ppm: c_double,
        callback: CVioletDoubleCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_fft_size(
        rx: CVioletReceiverHandle,
        fftsize: c_int,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
    fn violet_rx_set_fft_window(
        rx: CVioletReceiverHandle,
        window: CVioletWindowType,
        normalize_energy: bool,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
    fn violet_unsubscribe(connection: *const c_void);
    fn violet_rx_get_input_dev(rx: CVioletReceiverHandle) -> *mut c_char;
    fn violet_rx_get_input_rate(rx: CVioletReceiverHandle) -> c_int;
    fn violet_rx_get_input_decim(rx: CVioletReceiverHandle) -> c_int;
    fn violet_rx_get_rf_freq(rx: CVioletReceiverHandle) -> i64;
    fn violet_rx_get_cur_antenna(rx: CVioletReceiverHandle) -> *mut c_char;
    fn violet_rx_get_antennas_count(rx: CVioletReceiverHandle) -> c_int;
    fn violet_rx_get_antenna(rx: CVioletReceiverHandle, idx: c_int) -> *mut c_char;
    fn violet_rx_get_dc_cancel(rx: CVioletReceiverHandle) -> bool;
    fn violet_rx_get_iq_balance(rx: CVioletReceiverHandle) -> bool;
    fn violet_rx_get_iq_swap(rx: CVioletReceiverHandle) -> bool;
    fn violet_rx_get_gain_stages_count(rx: CVioletReceiverHandle) -> c_int;
    fn violet_rx_get_gain_stage(rx: CVioletReceiverHandle, idx: c_int) -> CVioletGainStage;
    fn violet_rx_get_auto_gain(rx: CVioletReceiverHandle) -> bool;
    fn violet_rx_get_freq_corr(rx: CVioletReceiverHandle) -> c_double;
    fn violet_rx_get_fft_size(rx: CVioletReceiverHandle) -> c_int;
    fn violet_rx_get_fft_window(rx: CVioletReceiverHandle) -> CVioletWindowType;
    fn violet_rx_set_antenna(
        rx: CVioletReceiverHandle,
        antenna: *const c_char,
        callback: CVioletVoidCallback,
        userdata: *const c_void,
    );
}

pub struct CAsyncReceiver {
    handle: CVioletReceiverHandle,
}

unsafe impl Send for CAsyncReceiver {}
unsafe impl Sync for CAsyncReceiver {}

impl Drop for CAsyncReceiver {
    fn drop(&mut self) {
        unsafe {
            violet_rx_destroy(self.handle);
        }
    }
}

impl Clone for CAsyncReceiver {
    fn clone(&self) -> Self {
        self.new_ref()
    }
}

extern "C" fn violet_void_callback(code: c_int, userdata: *const c_void) {
    let sender: Box<futures::channel::oneshot::Sender<c_int>> =
        unsafe { Box::from_raw(std::mem::transmute(userdata)) };

    if let Err(_) = sender.send(code) {
        eprintln!("ERROR violet_void_callback: receiver was dropped!");
    }
}

extern "C" fn violet_int_callback(code: c_int, data: c_int, userdata: *const c_void) {
    let sender: Box<futures::channel::oneshot::Sender<(c_int, i32)>> =
        unsafe { Box::from_raw(std::mem::transmute(userdata)) };

    if let Err(_) = sender.send((code, data)) {
        eprintln!("ERROR violet_int_callback: receiver was dropped!");
    }
}

extern "C" fn violet_int64_callback(code: c_int, data: i64, userdata: *const c_void) {
    let sender: Box<futures::channel::oneshot::Sender<(c_int, i64)>> =
        unsafe { Box::from_raw(std::mem::transmute(userdata)) };

    if let Err(_) = sender.send((code, data)) {
        eprintln!("ERROR violet_int64_callback: receiver was dropped!");
    }
}

extern "C" fn violet_double_callback(code: c_int, data: c_double, userdata: *const c_void) {
    let sender: Box<futures::channel::oneshot::Sender<(c_int, c_double)>> =
        unsafe { Box::from_raw(std::mem::transmute(userdata)) };

    if let Err(_) = sender.send((code, data)) {
        eprintln!("ERROR violet_double_callback: receiver was dropped!");
    }
}

struct FftCallbackMessageData {
    code: c_int,
    frame: FftFrame,
}

struct FftCallbackData {
    sender: futures::channel::oneshot::Sender<FftCallbackMessageData>,
    frame: FftFrame,
}

extern "C" fn violet_fft_callback(
    code: c_int,
    timestamp: CVioletTimestamp,
    freq: i64,
    sample_rate: c_int,
    _data: *mut c_float,
    size: c_int,
    userdata: *const c_void,
) {
    let mut data: Box<FftCallbackData> = unsafe { Box::from_raw(std::mem::transmute(userdata)) };

    // get frame out of the data to prepare to send it
    let mut frame = FftFrame::default();
    std::mem::swap(&mut frame, &mut data.frame);

    unsafe { frame.data.set_len(size as usize) };
    frame.freq = freq;
    frame.sample_rate = sample_rate;
    frame.timestamp = Timestamp {
        seconds: timestamp.seconds,
        nanos: timestamp.nanos,
    };

    let _ = data.sender.send(FftCallbackMessageData { code, frame });
}

#[async_trait::async_trait]
impl AsyncReceiver for CAsyncReceiver {
    fn new() -> Self {
        let handle = unsafe { violet_rx_init() };
        Self { handle }
    }

    async fn start(&self) -> VioletResult<()> {
        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);
        unsafe {
            violet_rx_start(
                self.handle,
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            );
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR start: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn stop(&self) -> VioletResult<()> {
        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);
        unsafe {
            violet_rx_stop(
                self.handle,
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            );
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR stop: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn set_input_dev(&self, dev: &str) -> VioletResult<()> {
        // FIXME: return meaningful error
        let dev_cstr = CString::new(dev).map_err(|_| VioletError::UnknownError)?;

        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_input_dev(
                self.handle,
                dev_cstr.as_ptr(),
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            );
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_input_dev: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn get_fft_data(&self, frame: &mut FftFrame) -> VioletResult<()> {
        // this is data-racey but who cares
        let fft_size = self.get_fft_size();

        if fft_size as usize > frame.data.len() {
            // unlike C++, reserve here takes additional space required
            frame.data.reserve(fft_size as usize - frame.data.len());
        }

        let (sender, receiver) = futures::channel::oneshot::channel::<FftCallbackMessageData>();

        // we box the data to make sure that it lives in the c-side even if
        // the future is dropped or forgotten
        let mut boxed_data = Box::new(FftCallbackData {
            sender, // to receive result
            frame: {
                let mut other_frame = FftFrame::default();
                std::mem::swap(frame, &mut other_frame);
                other_frame
            }, // move the user frame here to reuse his vec's allocated memory
        });

        unsafe {
            violet_rx_get_fft_data(
                self.handle,
                boxed_data.frame.data.as_mut_ptr(),
                fft_size,
                violet_fft_callback,
                Box::into_raw(boxed_data) as *const c_void,
            );
        };

        match receiver.await {
            // success
            Ok(FftCallbackMessageData {
                code: 0,
                frame: mut other_frame,
            }) => {
                std::mem::swap(frame, &mut other_frame);

                Ok(())
            }
            Ok(FftCallbackMessageData {
                code,
                frame: mut other_frame,
            }) => {
                std::mem::swap(frame, &mut other_frame);

                Err(VioletError::from_i32(code).unwrap_or_else(|| {
                    eprintln!("ERROR get_fft_data: Unknown error code {}", code);
                    VioletError::UnknownError
                }))
            }
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    fn connect(&self) -> Box<dyn futures::stream::Stream<Item = ReceiverEvent> + Send> {
        Box::new(EventsStream::new(self.clone()))
    }

    async fn set_antenna(&self, antenna: &str) -> VioletResult<()> {
        // FIXME: return meaningful error
        let antenna_cstr = CString::new(antenna).map_err(|_| VioletError::UnknownError)?;

        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_antenna(
                self.handle,
                antenna_cstr.as_ptr(),
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_antenna: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }
    async fn set_input_rate(&self, rate: i32) -> VioletResult<i32> {
        let (sender, receiver) = futures::channel::oneshot::channel::<(c_int, i32)>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_input_rate(
                self.handle,
                rate,
                violet_int_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok((0, rate)) => Ok(rate),
            Ok((code, _)) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_input_rate: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }
    async fn set_rf_freq(&self, freq: i64) -> VioletResult<i64> {
        let (sender, receiver) = futures::channel::oneshot::channel::<(c_int, i64)>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_rf_freq(
                self.handle,
                freq,
                violet_int64_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok((0, freq)) => Ok(freq),
            Ok((code, _)) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_rf_freq: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }
    async fn set_input_decim(&self, decim: i32) -> VioletResult<i32> {
        let (sender, receiver) = futures::channel::oneshot::channel::<(c_int, i32)>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_input_decim(
                self.handle,
                decim,
                violet_int_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok((0, decim)) => Ok(decim),
            Ok((code, _)) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_input_rate: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn set_iq_swap(&self, enable: bool) -> VioletResult<()> {
        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_iq_swap(
                self.handle,
                enable,
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_iq_swap: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn set_dc_cancel(&self, enable: bool) -> VioletResult<()> {
        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_dc_cancel(
                self.handle,
                enable,
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_dc_cancel: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn set_iq_balance(&self, enable: bool) -> VioletResult<()> {
        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_iq_balance(
                self.handle,
                enable,
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_dc_cancel: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn set_auto_gain(&self, enable: bool) -> VioletResult<()> {
        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_auto_gain(
                self.handle,
                enable,
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_auto_gain: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn set_gain(&self, name: &str, value: f64) -> VioletResult<f64> {
        // FIXME: return meaningful error
        let name_cstr = CString::new(name).map_err(|_| VioletError::UnknownError)?;

        let (sender, receiver) = futures::channel::oneshot::channel::<(c_int, c_double)>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_gain(
                self.handle,
                name_cstr.as_ptr(),
                value,
                violet_double_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok((0, gain)) => Ok(gain),
            Ok((code, _)) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_gain: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn set_freq_corr(&self, ppm: f64) -> VioletResult<f64> {
        let (sender, receiver) = futures::channel::oneshot::channel::<(c_int, c_double)>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_freq_corr(
                self.handle,
                ppm,
                violet_double_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok((0, ppm)) => Ok(ppm),
            Ok((code, _)) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_freq_corr: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn set_fft_size(&self, fftsize: i32) -> VioletResult<()> {
        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_fft_size(
                self.handle,
                fftsize,
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_fft_size: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }

    async fn set_fft_window(&self, window: WindowType) -> VioletResult<()> {
        // FIXME: return meaningful error
        let cwindow =
            CVioletWindowType::from_i32(window as i32).ok_or(VioletError::UnknownError)?;

        let (sender, receiver) = futures::channel::oneshot::channel::<c_int>();
        let boxed_sender = Box::new(sender);

        unsafe {
            violet_rx_set_fft_window(
                self.handle,
                cwindow,
                false,
                violet_void_callback,
                Box::into_raw(boxed_sender) as *const c_void,
            )
        };

        match receiver.await {
            Ok(0) => Ok(()),
            Ok(code) => Err(VioletError::from_i32(code).unwrap_or_else(|| {
                eprintln!("ERROR set_fft_size: Unknown error code: {}", code);
                VioletError::UnknownError
            })),
            Err(_) => Err(VioletError::UnknownError),
        }
    }
}

impl CAsyncReceiver {
    pub fn new_ref(&self) -> Self {
        let handle = unsafe { violet_rx_new_ref(self.handle) };
        Self { handle }
    }

    fn cstr_to_string(cstr: *const c_char) -> String {
        let str = {
            let cstr_cstr = unsafe { CStr::from_ptr(cstr) };
            cstr_cstr.to_string_lossy().into_owned()
        };

        unsafe { libc::free(cstr as *mut c_void) };

        str
    }

    fn get_input_device(&self) -> String {
        Self::cstr_to_string(unsafe { violet_rx_get_input_dev(self.handle) })
    }

    fn get_input_rate(&self) -> i32 {
        unsafe { violet_rx_get_input_rate(self.handle) }
    }

    fn get_input_decim(&self) -> i32 {
        unsafe { violet_rx_get_input_decim(self.handle) as i32 }
    }

    fn get_rf_freq(&self) -> i64 {
        unsafe { violet_rx_get_rf_freq(self.handle) }
    }

    fn get_antenna(&self) -> String {
        Self::cstr_to_string(unsafe { violet_rx_get_cur_antenna(self.handle) })
    }

    fn get_antennas(&self) -> Vec<String> {
        let count = unsafe { violet_rx_get_antennas_count(self.handle) };

        let mut result = Vec::new();
        result.reserve(count as usize);

        for i in 0..count {
            result.push(Self::cstr_to_string(unsafe {
                violet_rx_get_antenna(self.handle, i)
            }));
        }

        result
    }

    fn get_dc_cancel(&self) -> bool {
        unsafe { violet_rx_get_dc_cancel(self.handle) }
    }

    fn get_iq_balance(&self) -> bool {
        unsafe { violet_rx_get_iq_balance(self.handle) }
    }

    fn get_iq_swap(&self) -> bool {
        unsafe { violet_rx_get_iq_swap(self.handle) }
    }

    fn get_gain_stages(&self) -> Vec<GainStage> {
        let count = unsafe { violet_rx_get_gain_stages_count(self.handle) };

        let mut result = Vec::new();
        result.reserve(count as usize);

        for i in 0..count {
            let cstage = unsafe { violet_rx_get_gain_stage(self.handle, i) };

            result.push(GainStage {
                name: Self::cstr_to_string(cstage.name),
                start: cstage.start,
                stop: cstage.stop,
                step: cstage.step,
                value: cstage.value,
            });
        }

        result
    }

    fn get_fft_size(&self) -> i32 {
        unsafe { violet_rx_get_fft_size(self.handle) }
    }

    fn get_fft_window(&self) -> WindowType {
        // FIXME: no unwrap please
        WindowType::from_i32(unsafe { violet_rx_get_fft_window(self.handle) as i32 }).unwrap()
    }

    fn get_freq_corr(&self) -> f64 {
        unsafe { violet_rx_get_freq_corr(self.handle) }
    }

    fn get_auto_gain(&self) -> bool {
        unsafe { violet_rx_get_auto_gain(self.handle) }
    }
}

struct Connection {
    handle: *const c_void,
}

impl Drop for Connection {
    fn drop(&mut self) {
        unsafe { violet_unsubscribe(self.handle) };
    }
}

// EventsStream will not be Send or Sync if Connection wasn't
unsafe impl Send for Connection {}
unsafe impl Sync for Connection {}

struct EventsStream {
    connection: Connection,
    channel_receiver: std::sync::mpsc::Receiver<ReceiverEvent>,
    // we're boxing the EventsStreamCSide to ensure that it lives in the heap,
    // and so that if somehow we "std::mem::forget" the EventsStream, the c++
    // side will still use valid userdata that lives in the heap
    c_side: Box<EventsStreamCSide>,
    first_poll: bool,
    _phantom_pinned: std::marker::PhantomPinned,
}

struct EventsStreamCSide {
    receiver: CAsyncReceiver,
    channel_sender: Option<std::sync::mpsc::Sender<ReceiverEvent>>,
    waker: Mutex<Option<std::task::Waker>>,
}

impl EventsStream {
    pub fn new(receiver: CAsyncReceiver) -> Self {
        let (channel_sender, channel_receiver) = std::sync::mpsc::channel();

        let c_side = Box::new(EventsStreamCSide {
            receiver,
            channel_sender: Some(channel_sender),
            waker: Mutex::new(None),
        });

        Self {
            connection: Connection { handle: null() },
            channel_receiver,
            c_side,
            first_poll: true,
            _phantom_pinned: std::marker::PhantomPinned,
        }
    }

    fn init(&mut self) {
        let userdata: *const c_void = unsafe { std::mem::transmute(&*(self.c_side)) };

        self.connection.handle = unsafe {
            violet_rx_subscribe(
                self.c_side.receiver.handle,
                violet_rx_event_callback,
                userdata,
            )
        };
    }
}

fn get_violet_event_data(c_violet_event_ptr: *const CVioletEventCommon) -> ReceiverEventData {
    let c_violet_event: &CVioletEventCommon = unsafe { std::mem::transmute(c_violet_event_ptr) };

    match c_violet_event.event_type {
        CVioletEventType::Unsubscribed => ReceiverEventData::Unsubscribed,
        CVioletEventType::SyncStart => ReceiverEventData::SyncStart,
        CVioletEventType::SyncEnd => ReceiverEventData::SyncEnd,
        CVioletEventType::Started => ReceiverEventData::Started,
        CVioletEventType::Stopped => ReceiverEventData::Stopped,
        CVioletEventType::InputDeviceChanged => {
            let c_specific_event: &CVioletInputDeviceChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            let device = unsafe { CStr::from_ptr(c_specific_event.device) };
            ReceiverEventData::InputDeviceChanged(device.to_string_lossy().into_owned())
        }
        CVioletEventType::AntennaChanged => {
            let c_specific_event: &CVioletAntennaChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            let device = unsafe { CStr::from_ptr(c_specific_event.antenna) };
            ReceiverEventData::AntennaChanged(device.to_string_lossy().into_owned())
        }
        CVioletEventType::InputRateChanged => {
            let c_specific_event: &CVioletInputRateChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::InputRateChanged(c_specific_event.input_rate)
        }
        CVioletEventType::InputDecimChanged => {
            let c_specific_event: &CVioletInputDecimChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::InputDecimChanged(c_specific_event.decim)
        }
        CVioletEventType::IqSwapChanged => {
            let c_specific_event: &CVioletIqSwapChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::IqSwapChanged(c_specific_event.enabled)
        }
        CVioletEventType::DcCancelChanged => {
            let c_specific_event: &CVioletDcCancelChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::DcCancelChanged(c_specific_event.enabled)
        }
        CVioletEventType::IqBalanceChanged => {
            let c_specific_event: &CVioletIqBalanceChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::IqBalanceChanged(c_specific_event.enabled)
        }
        CVioletEventType::RfFreqChanged => {
            let c_specific_event: &CVioletRfFreqChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::RfFreqChanged(c_specific_event.freq)
        }
        CVioletEventType::GainStagesChanged => unsafe {
            let c_specific_event: &CVioletGainStagesChanged =
                std::mem::transmute(c_violet_event_ptr);
            let mut gain_stages = Vec::<GainStage>::new();
            for idx in 0..c_specific_event.size as isize {
                let c_gain_stage: &CVioletGainStage = &*c_specific_event.stages.offset(idx);
                gain_stages.push(GainStage {
                    name: CStr::from_ptr(c_gain_stage.name)
                        .to_string_lossy()
                        .into_owned(),
                    start: c_gain_stage.start,
                    stop: c_gain_stage.stop,
                    step: c_gain_stage.step,
                    value: c_gain_stage.value,
                });
            }
            ReceiverEventData::GainStagesChanged(gain_stages)
        },
        CVioletEventType::AntennasChanged => unsafe {
            let c_specific_event: &CVioletAntennasChanged = std::mem::transmute(c_violet_event_ptr);
            let mut antennas = Vec::<String>::new();
            for idx in 0..c_specific_event.size as isize {
                antennas.push(
                    CStr::from_ptr(*c_specific_event.antennas.offset(idx))
                        .to_string_lossy()
                        .into_owned(),
                );
            }

            ReceiverEventData::AntennasChanged(antennas)
        },
        CVioletEventType::AutoGainChanged => {
            let c_specific_event: &CVioletAutoGainChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::AutoGainChanged(c_specific_event.enabled)
        }
        CVioletEventType::GainChanged => unsafe {
            let c_specific_event: &CVioletGainChanged = std::mem::transmute(c_violet_event_ptr);
            ReceiverEventData::GainChanged(
                CStr::from_ptr(c_specific_event.name)
                    .to_string_lossy()
                    .into_owned(),
                c_specific_event.value,
            )
        },
        CVioletEventType::FreqCorrChanged => {
            let c_specific_event: &CVioletFreqCorrChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::FreqCorrChanged(c_specific_event.ppm)
        }
        CVioletEventType::FftSizeChanged => {
            let c_specific_event: &CVioletFftSizeChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::FftSizeChanged(c_specific_event.size)
        }
        CVioletEventType::FftWindowChanged => {
            let c_specific_event: &CVioletFftWindowChanged =
                unsafe { std::mem::transmute(c_violet_event_ptr) };
            ReceiverEventData::FftWindowChanged(
                WindowType::from_i32(c_specific_event.window as i32).unwrap(),
            )
        }
        CVioletEventType::IqRecordingStarted => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::IqRecordingStopped => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoSyncStart => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoSyncEnd => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAdded => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoRemoved => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoDemodChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoOffsetChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoCwOffsetChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoFilterChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoNoiseBlankerOnChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoNoiseBlankerThresholdChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoSqlLevelChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoSqlAlphaChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAgcOnChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAgcHangChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAgcThresholdChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAgcSlopeChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAgcDecayChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAgcManualGainChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoFmMaxdevChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoFmDeemphChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAmDcrChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAmSyncDcrChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAmSyncPllBwChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoRecordingStarted => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoRecordingStopped => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoSnifferStarted => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoSnifferStopped => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoUdpStreamingStarted => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoUdpStreamingStopped => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoRdsDecoderStarted => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoRdsDecoderStopped => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoRdsParserReset => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::VfoAudioGainChanged => {
            // TODO
            ReceiverEventData::Unknown
        }
        CVioletEventType::EventUnknown => {
            // TODO
            ReceiverEventData::Unknown
        }
    }
}

extern "C" fn violet_rx_event_callback(
    c_violet_event_ptr: *const CVioletEventCommon,
    userdata: *const c_void,
) {
    let events_stream: &mut EventsStreamCSide = unsafe { std::mem::transmute(userdata) };
    let c_violet_event: &CVioletEventCommon = unsafe { std::mem::transmute(c_violet_event_ptr) };

    let event = ReceiverEvent {
        id: c_violet_event.id,
        timestamp: Timestamp {
            seconds: c_violet_event.timestamp.seconds,
            nanos: c_violet_event.timestamp.nanos,
        },
        data: get_violet_event_data(c_violet_event_ptr),
    };

    if let Some(ref sender) = events_stream.channel_sender {
        if let Ok(()) = sender.send(event) {
            if let Ok(Some(waker)) = events_stream.waker.lock().map(|mut x| x.take()) {
                waker.wake();
            }
        }
    }

    if c_violet_event.event_type == CVioletEventType::Unsubscribed {
        events_stream.channel_sender = None;
    }
}

impl futures::stream::Stream for EventsStream {
    type Item = ReceiverEvent;

    fn poll_next(
        self: std::pin::Pin<&mut Self>,
        cx: &mut futures::task::Context<'_>,
    ) -> futures::task::Poll<Option<Self::Item>> {
        let this = unsafe { self.get_unchecked_mut() };

        if let Ok(mut waker_result) = this.c_side.waker.lock() {
            *waker_result = Some(cx.waker().clone());
        }

        if this.first_poll {
            this.init();
            this.first_poll = false;
        }

        match this.channel_receiver.try_recv() {
            Ok(event) => futures::task::Poll::Ready(Some(event)),
            Err(std::sync::mpsc::TryRecvError::Empty) => futures::task::Poll::Pending,
            Err(std::sync::mpsc::TryRecvError::Disconnected) => futures::task::Poll::Ready(None),
        }
    }
}
