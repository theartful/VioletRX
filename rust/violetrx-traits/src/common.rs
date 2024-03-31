#[derive(Default, Clone)]
pub struct FftFrame {
    pub data: Vec<f32>,
    pub timestamp: Timestamp,
    pub freq: i64,
    pub sample_rate: i32,
}

#[derive(Default, Debug, Copy, Clone)]
pub struct Timestamp {
    pub seconds: u64,
    pub nanos: u32,
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
