find_package(Qt6 REQUIRED COMPONENTS Core Widgets SvgWidgets Svg Network)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_PORTAUDIO REQUIRED IMPORTED_TARGET "portaudio-2.0")
pkg_check_modules(PC_SNDFILE REQUIRED IMPORTED_TARGET "sndfile")

qt_add_resources(
RESOURCES_LIST
    resources/icons.qrc
    resources/textfiles.qrc
)

qt_wrap_ui(
UI_SRCS_LIST
    agc_options.ui
    audio_options.ui
    demod_options.ui
    dockaudio.ui
    dockfft.ui
    dockinputctl.ui
    dockrds.ui
    vfoopt.ui
    ioconfig.ui
    nb_options.ui
    mainwindow.ui
)

add_executable(
qtclient
    plotter/gridgraphicsitem.h
    plotter/gridgraphicsitem.cpp
    plotter/fftgraphicsitem.h
    plotter/fftgraphicsitem.cpp
    plotter/vfographicsitem.h
    plotter/vfographicsitem.cpp
    plotter/plottergraphicsview.h
    plotter/plottergraphicsview.cpp
    plotter/plotter.h
    plotter/plotter.cpp
    plotter/waterfall.h
    plotter/waterfall.cpp
    plotter/fftaverager.h
    plotter/fftaverager.cpp
    plotter/waterfall_colormaps.h
    plotter/waterfall_colormaps.cpp
    plotter/bookmarksgraphicsitem.h
    plotter/bookmarksgraphicsitem.cpp
    agc_options.cpp
    agc_options.h
    audio_options.cpp
    audio_options.h
    ctk/ctkPimpl.h
    ctk/ctkRangeSlider.cpp
    ctk/ctkRangeSlider.h
    demod_options.cpp
    demod_options.h
    dockaudio.cpp
    dockaudio.h
    dockfft.cpp
    dockfft.h
    dockinputctl.cpp
    dockinputctl.h
    dockrds.cpp
    dockrds.h
    vfoopt.cpp
    vfoopt.h
    vfosopt.cpp
    vfosopt.h
    freqctrl.cpp
    freqctrl.h
    ioconfig.cpp
    ioconfig.h
    meter.cpp
    meter.h
    nb_options.cpp
    nb_options.h
    qtcolorpicker.cpp
    qtcolorpicker.h
    expandablewidget.cpp
    expandablewidget.h
    audioplayer.h
    audioplayer.cpp
    receiver_model.h
    receiver_model.cpp
    mainwindow.h
    mainwindow.cpp
    main.cpp
    ${RESOURCES_LIST}
    ${UI_SRCS_LIST}
)

set_target_properties(qtclient PROPERTIES AUTOMOC TRUE)

target_include_directories(
qtclient
    PUBLIC "${SOURCE_DIRECTORY}"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}" 
)

include(FetchContent)
FetchContent_Declare(
    bindable_properties
    GIT_REPOSITORY    https://github.com/theartful/bindable_properties
    GIT_TAG           master
)
FetchContent_MakeAvailable(bindable_properties)

target_link_libraries(
qtclient
PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Network
    PkgConfig::PC_PORTAUDIO
    PkgConfig::PC_SNDFILE
    bindable_properties
    async_core
    grpc_client

PRIVATE
    spdlog::spdlog
)
