use num_derive::FromPrimitive;

pub type VioletResult<T> = std::result::Result<T, VioletError>;

#[derive(Debug, FromPrimitive)]
pub enum VioletError {
    Ok = 0,
    WorkerBusy = 1,
    GainNotFound = 2,
    AlreadyRecording = 3,
    AlreadyNotRecording = 4,
    InvalidInputDevice = 5,
    InvalidFilter = 6,
    InvalidFilterOffset = 7,
    InvalidCwOffset = 8,
    InvalidDemod = 9,
    VfoNotFound = 10,
    DemodIsOff = 11,
    NotRunning = 12,
    CouldntCreateFile = 13,
    SnifferAlreadyActive = 14,
    SnifferAlreadyInactive = 15,
    InsufficientBufferSize = 16,
    RdsAlreadyActive = 17,
    RdsAlreadyInactive = 18,
    UnknownError = 99999,
}

impl std::fmt::Display for VioletError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        <Self as std::fmt::Debug>::fmt(&self, f)
    }
}

impl std::error::Error for VioletError {}
