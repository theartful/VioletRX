use num_derive::FromPrimitive;

#[derive(Debug, FromPrimitive)]
pub enum Demod {
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
}

#[async_trait::async_trait]
pub trait AsyncVfo {

}
