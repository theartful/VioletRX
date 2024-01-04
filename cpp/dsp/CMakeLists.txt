add_library(
dsp
	filter/fir_decim.cpp
	filter/fir_decim.h
	filter/fir_decim_coef.h
	rds/api.h
	rds/constants.h
	rds/decoder_impl.cc
	rds/decoder_impl.h
	rds/decoder.h
	rds/parser_impl.cc
	rds/parser_impl.h
	rds/parser.h
	rds/tmc_events.h
	agc_impl.cpp
	agc_impl.h
	correct_iq_cc.cpp
	correct_iq_cc.h
	downconverter.cpp
	downconverter.h
	fm_deemph.cpp
	fm_deemph.h
	lpf.cpp
	lpf.h
	resampler_xx.cpp
	resampler_xx.h
	rx_agc_xx.cpp
	rx_agc_xx.h
	rx_demod_am.cpp
	rx_demod_am.h
	rx_demod_fm.cpp
	rx_demod_fm.h
	rx_fft.cpp
	rx_fft.h
	rx_filter.cpp
	rx_filter.h
	rx_meter.cpp
	rx_meter.h
	rx_noise_blanker_cc.cpp
	rx_noise_blanker_cc.h
	rx_rds.cpp
	rx_rds.h
	sniffer_f.cpp
	sniffer_f.h
	stereo_demod.cpp
	stereo_demod.h
	multichannel_downconverter.h
	multichannel_downconverter.cpp
)

target_include_directories(
dsp
    PUBLIC "${SOURCE_DIRECTORY}"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_property(TARGET dsp PROPERTY CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_FFTW3F REQUIRED IMPORTED_TARGET "fftw3f")
pkg_check_modules(PC_SNDFILE REQUIRED IMPORTED_TARGET "sndfile")

# FIXME: do we really need all these components?
find_package(Gnuradio REQUIRED COMPONENTS
    analog audio blocks digital filter fft network)

target_link_libraries(
dsp
PUBLIC
  gnuradio::gnuradio-analog
  gnuradio::gnuradio-blocks
  gnuradio::gnuradio-digital
  gnuradio::gnuradio-filter
  gnuradio::gnuradio-audio

PRIVATE
  PkgConfig::PC_SNDFILE
  PkgConfig::PC_FFTW3F
)
