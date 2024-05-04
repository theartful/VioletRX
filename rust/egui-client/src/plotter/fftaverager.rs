use eframe::epaint::Rect;
use violetrx_traits::FftFrame;

#[derive(Default)]
pub struct DecimatedFftFrame {
    pub data: Vec<f32>,
    pub left: f32,
    pub right: f32,
}

// TODO: change the name of this struct
pub struct FftAverager {
    pub iir_fft_frame: FftFrame,
    pub rate: i32,
    pub alpha: f32,
    decimated_frame: DecimatedFftFrame,
}

impl FftAverager {
    pub fn new(rate: i32, alpha: f32) -> Self {
        Self {
            iir_fft_frame: FftFrame::default(),
            rate,
            alpha,
            decimated_frame: DecimatedFftFrame::default(),
        }
    }

    pub fn step(&mut self, fft_frame: &FftFrame) {
        // reset iir_fft_frame if this is a new frame from different sample rate
        // or center frequency
        if fft_frame.sample_rate != self.iir_fft_frame.sample_rate
            || fft_frame.freq != self.iir_fft_frame.freq
            || fft_frame.data.len() != self.iir_fft_frame.data.len()
        {
            self.reset();
            self.iir_fft_frame.data.resize(fft_frame.data.len(), 0.0);
            self.iir_fft_frame.sample_rate = fft_frame.sample_rate;
            self.iir_fft_frame.freq = fft_frame.freq;
        }

        let fftsize = fft_frame.data.len();
        let pwr_scale = 1.0 / (fftsize as f32 * fftsize as f32);

        let a = (self.rate as f32).powf(-1.75 * (1.0 - self.alpha));
        let gamma = 0.7;
        let a = a.powf(gamma);

        for i in 0..fftsize {
            let v = 10.0
                * (fft_frame.data[i] * pwr_scale)
                    .max(f32::MIN_POSITIVE)
                    .log10();
            let iir = self.iir_fft_frame.data[i];

            self.iir_fft_frame.data[i] = iir * (1.0 - a) + v * a;
        }
    }

    pub fn set_rate(&mut self, rate: i32) {
        self.rate = rate;
    }

    pub fn set_fft_avg(&mut self, alpha: f32) {
        self.alpha = alpha;
    }

    pub fn reset(&mut self) {
        self.iir_fft_frame.data.clear();
    }

    pub fn decimate(&mut self, pixel_viewport: Rect, scene_viewport: Rect) -> &DecimatedFftFrame {
        self.decimated_frame.data.clear();

        let data = &self.iir_fft_frame.data;
        if data.is_empty() {
            return &self.decimated_frame;
        }

        let freq = self.iir_fft_frame.freq;
        let sample_rate = self.iir_fft_frame.sample_rate;

        let fftsize = data.len();
        let freq_begin = freq as f32 - sample_rate as f32 / 2.0;
        let rbw = sample_rate as f32 / fftsize as f32;
        let pixel_per_freq = pixel_viewport.width() as f32 / scene_viewport.width() as f32;

        let begin_index = (((scene_viewport.left() - freq_begin) / rbw).floor() as isize)
            .clamp(0, fftsize as isize - 1) as usize;

        let end_index = (((scene_viewport.right() - freq_begin) / rbw).ceil() as isize)
            .clamp(0, fftsize as isize - 1) as usize;

        let get_pixel =
            |freq: f32| ((freq - scene_viewport.left()) * pixel_per_freq).max(0.0) as usize;

        let mut cur_x = get_pixel(freq_begin + begin_index as f32 * rbw);
        let mut cur_val = f32::MIN;

        for i in begin_index..=end_index {
            let x = get_pixel(freq_begin + i as f32 * rbw);

            if x == cur_x {
                cur_val = cur_val.max(data[i]);
            } else {
                self.decimated_frame.data.push(cur_val);

                cur_val = data[i];
                cur_x = x;
            }
        }

        // last point
        self.decimated_frame.data.push(cur_val);

        self.decimated_frame.left = freq_begin + begin_index as f32 * rbw;
        self.decimated_frame.right = freq_begin + end_index as f32 * rbw;

        &self.decimated_frame
    }
}
