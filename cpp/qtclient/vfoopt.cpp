#include "vfoopt.h"
#include "ui_vfoopt.h"

#include <QMessageBox>
#include <QSettings>
#include <QVariant>

#include <cmath>
#include <iostream>
#include <optional>

#include "audioplayer.h"
#include "receiver_model.h"

#include "qtclient/expandablewidget.h"
#include <spdlog/fmt/fmt.h>

#define FILTER_PRESET_WIDE 0
#define FILTER_PRESET_NORMAL 1
#define FILTER_PRESET_NARROW 2
#define FILTER_PRESET_USER 3

#define FILTER_SHAPE_SOFT 0
#define FILTER_SHAPE_NORMAL 1
#define FILTER_SHAPE_SHARP 2

struct AgcPreset {
    bool on;
    std::optional<int> decay;
    std::optional<int> slope;
};

static const AgcPreset AgcPresets[] = {
    AgcPreset{true, 100, 0},                      // FAST
    AgcPreset{true, 500, 0},                      // MEDIUM
    AgcPreset{true, 2000, 0},                     // SLOW
    AgcPreset{true, std::nullopt, std::nullopt},  // USER
    AgcPreset{false, std::nullopt, std::nullopt}, // OFF
};

enum EAgcPreset {
    FAST = 0,
    MEDIUM = 1,
    SLOW = 2,
    USER = 3,
    OFF = 4,
    COUNT = 5
};

VfoOpt::VfoOpt(VFOChannelModel* vfo_, QWidget* parent) :
    QWidget(parent), ui(new Ui::VfoOpt), vfo(vfo_)
{
    ui->setupUi(this);
    ui->verticalLayout->setAlignment(Qt::AlignTop);

    ui->modeSelector->addItems(QStringList{
        "Off",          // OFF              0
        "Raw I/Q",      // RAW              1
        "AM",           // AM               2
        "AM-Sync",      // AMSYNC           3
        "LSB",          // LSB              4
        "USB",          // USB              5
        "CW-L",         // CWL              6
        "CW-U",         // CWU              7
        "Narrow FM",    // MODW_NFM         8
        "WFM (mono)",   // WFM_MONO         9
        "WFM (stereo)", // WFM_STEREO       10
        "WFM (oirt)"    // WFM_STEREO_OIRT  11
    });

    ui->agcPresetCombo->addItems(QStringList{
        "Fast",   // FAST          0
        "Medium", // MEDIUM        1
        "Slow",   // SLOW          2
        "User",   // USER          3
        "Off",    // OFF           4
    });

    audioOptions = new CAudioOptions(this);

    connect(audioOptions, &CAudioOptions::newRecDirSelected, this,
            &VfoOpt::onNewRecDirSelected);

    // use same slot for filteCombo and filterShapeCombo
    connect(ui->filterShapeCombo, SIGNAL(activated(int)), this,
            SLOT(on_filterCombo_currentIndexChanged(int)));

    // demodulator options dialog
    demodOpt = new CDemodOptions(vfo, this);
    demodOpt->setCurrentPage(CDemodOptions::PAGE_NO_OPT);

    // AGC options dialog
    agcOpt = new CAgcOptions(vfo, this);

    // Noise blanker options
    nbOpt = new CNbOptions(vfo, this);

    ui->audioPlayButton->setEnabled(false);

    // audio player
    audioPlayer = new AudioPlayer(this);
    audioPosStr.reserve(16);
    durationStr.reserve(16);

    connect(audioPlayer, &AudioPlayer::positionChanged, this,
            &VfoOpt::onAudioPlayerPositionChanged);
    connect(audioPlayer, &AudioPlayer::durationChanged, this,
            &VfoOpt::onAudioPlayerDurationChanged);
    connect(audioPlayer, &AudioPlayer::playbackStateChanged, this,
            &VfoOpt::onAudioPlayerPlaybackStateChanged);
    connect(audioPlayer, &AudioPlayer::playbackFinished, this,
            &VfoOpt::onAudioPlayerPlaybackFinished);

    connect(vfo, &VFOChannelModel::demodChanged, this, &VfoOpt::onDemodChanged);
    connect(vfo, &VFOChannelModel::filterChanged, this,
            &VfoOpt::onFilterChanged);
    connect(vfo, &VFOChannelModel::offsetChanged, this,
            &VfoOpt::onFilterOffsetChanged);
    connect(vfo->parentModel(), &ReceiverModel::centerFreqChanged, this,
            &VfoOpt::onCenterFreqChanged);
    connect(vfo, &VFOChannelModel::agcOnChanged, this,
            &VfoOpt::onAgcParamChanged);
    connect(vfo, &VFOChannelModel::agcHangChanged, this,
            &VfoOpt::onAgcParamChanged);
    connect(vfo, &VFOChannelModel::agcThresholdChanged, this,
            &VfoOpt::onAgcParamChanged);
    connect(vfo, &VFOChannelModel::agcSlopeChanged, this,
            &VfoOpt::onAgcParamChanged);
    connect(vfo, &VFOChannelModel::agcManualGainChanged, this,
            &VfoOpt::onAgcParamChanged);
    connect(vfo, &VFOChannelModel::agcDecayChanged, this,
            &VfoOpt::onAgcParamChanged);
    connect(vfo, &VFOChannelModel::noiseBlankerOnChanged, this,
            &VfoOpt::onNoiseBlankerOnChanged);
    connect(vfo, &VFOChannelModel::sqlLevelChanged, this,
            &VfoOpt::onSqlChanged);
    connect(vfo, &VFOChannelModel::audioGainChanged, this,
            &VfoOpt::onAudioGainChanged);
    connect(vfo, &VFOChannelModel::audioRecordingStarted, this,
            &VfoOpt::onAudioRecordingStarted);
    connect(vfo, &VFOChannelModel::audioRecordingStopped, this,
            &VfoOpt::onAudioRecordingStopped);
    connect(vfo, &VFOChannelModel::udpStreamingStarted, this,
            &VfoOpt::onUdpStreamingStarted);
    connect(vfo, &VFOChannelModel::udpStreamingStopped, this,
            &VfoOpt::onUdpStreamingStopped);

    signalPowerTimer = new QTimer(this);
    connect(signalPowerTimer, &QTimer::timeout, this,
            &VfoOpt::onSignalPowerTimerTimeout);
    signalPowerTimer->start(100);

    setupRds();

    refreshFilterCombo();
    refreshFreqSpinBox();
    refreshFilterFreq();
    refreshAgcPresetCombo();
    refreshNoiseBlankerButtons();
    refreshSqlSpinBox();
    refreshAudioGainSlider();
    refreshAudioGainLabel();
    refreshMuteButton();
    refreshRecordButton();
    refreshAudioStreamButton();
    refreshRds();

    // Location of audio recordings
    // FIXME
    recDir = QDir::homePath();

    ExpandableWidget::setExpandFunctionality(ui->expandAudioButton,
                                             ui->audioSection, this);

    ExpandableWidget::setExpandFunctionality(ui->expandRdsButton,
                                             ui->rdsSection, this);
}

VfoOpt::~VfoOpt() { delete ui; }

void VfoOpt::on_freqSpinBox_valueChanged(double freq)
{
    // reset spinbox
    refreshFreqSpinBox();

    vfo->setFilterOffset((qint64)(freq * 1000));
}

void VfoOpt::on_filterFreq_requestNewFrequency(qint64 freq)
{
    if (!vfo->parentModel())
        return;

    qint64 filterOffset = freq - vfo->parentModel()->centerFreq();
    vfo->setFilterOffset(filterOffset);
}

void VfoOpt::on_audioGainSlider_valueChanged(int value)
{
    refreshAudioGainSlider();

    vfo->setAudioGain(value / 10.0);
}

void VfoOpt::on_audioMuteButton_toggled(bool checked)
{
    refreshMuteButton();

    if (checked) {
        vfo->setAudioGain(-INFINITY);
    } else {
        vfo->setAudioGain(ui->audioGainSlider->value() / 10.0);
    }
}

void VfoOpt::on_audioRecButton_toggled(bool checked)
{
    refreshRecordButton();

    if (checked) {
        ReceiverModel* rxModel = vfo->parentModel();
        if (!rxModel)
            return;

        qint64 freq = rxModel->centerFreq() + vfo->filterOffset();

        QString fileName = QDateTime::currentDateTime().toUTC().toString(
            "gqrx_yyyyMMdd_hhmmss");

        vfo->startAudioRecording(
            QString("%1/%2_%3.wav").arg(recDir).arg(fileName).arg(freq));
    } else {
        vfo->stopAudioRecording();
    }
}

void VfoOpt::on_audioConfButton_clicked() { audioOptions->show(); }

void VfoOpt::refreshFreqSpinBox()
{
    ui->freqSpinBox->blockSignals(true);
    ui->freqSpinBox->setValue(1.e-3 * (double)vfo->filterOffset());
    ui->freqSpinBox->blockSignals(false);
}

void VfoOpt::on_audioStreamButton_toggled(bool checked)
{
    refreshAudioStreamButton();

    if (checked) {
        vfo->startUdpStreaming(audioOptions->udpHost(), audioOptions->udpPort(),
                               audioOptions->udpStereo());
    } else {
        vfo->stopUdpStreaming();
    }
}

void VfoOpt::refreshFilterCombo()
{
    FilterPreset preset = vfo->filterPreset();
    Filter filter = vfo->filter();

    ui->filterCombo->blockSignals(true);
    if (filter.low == preset.wide.low && filter.high == preset.wide.high) {
        ui->filterCombo->setCurrentIndex(FILTER_PRESET_WIDE);
    } else if (filter.low == preset.normal.low &&
               filter.high == preset.normal.high) {
        ui->filterCombo->setCurrentIndex(FILTER_PRESET_NORMAL);
    } else if (filter.low == preset.narrow.low &&
               filter.high == preset.narrow.high) {
        ui->filterCombo->setCurrentIndex(FILTER_PRESET_NARROW);
    } else {
        ui->filterCombo->setCurrentIndex(FILTER_PRESET_USER);
        ui->filterCombo->setItemText(
            FILTER_PRESET_USER,
            QString("User (%1 k)").arg((filter.high - filter.low) / 1000.0));
    }
    ui->filterCombo->blockSignals(false);

    // restore filterShapeCombo
    switch (filter.shape) {
    case FilterShape::NORMAL:
        ui->filterShapeCombo->setCurrentIndex(FILTER_SHAPE_NORMAL);
        break;
    case FilterShape::SOFT:
        ui->filterShapeCombo->setCurrentIndex(FILTER_SHAPE_SOFT);
        break;
    case FilterShape::SHARP:
        ui->filterShapeCombo->setCurrentIndex(FILTER_SHAPE_SHARP);
        break;
    }
}

Filter VfoOpt::getFilter(int filterIndex, int filterShapeIndex)
{
    FilterPreset preset = vfo->filterPreset();
    Filter filter = vfo->filter();

    switch (filterIndex) {
    case FILTER_PRESET_WIDE:
        filter = preset.wide;
        break;
    case FILTER_PRESET_NORMAL:
        filter = preset.normal;
        break;
    case FILTER_PRESET_NARROW:
        filter = preset.narrow;
        break;
    }

    switch (filterShapeIndex) {
    case FILTER_SHAPE_SOFT:
        filter.shape = FilterShape::SOFT;
        break;
    case FILTER_SHAPE_NORMAL:
        filter.shape = FilterShape::NORMAL;
        break;
    case FILTER_SHAPE_SHARP:
        filter.shape = FilterShape::SHARP;
        break;
    }

    return filter;
}

void VfoOpt::on_filterCombo_currentIndexChanged(int)
{
    int filterComboIndex = ui->filterCombo->currentIndex();
    int filterShapeComboIndex = ui->filterShapeCombo->currentIndex();

    // restore filterCombo because the request might not be successful
    refreshFilterCombo();

    Filter filter = getFilter(filterComboIndex, filterShapeComboIndex);
    vfo->setFilter(filter.low, filter.high, filter.shape);
}

void VfoOpt::onDemodChanged()
{
    int index = (int)vfo->demod();

    ui->modeSelector->blockSignals(true);
    ui->modeSelector->setCurrentIndex(index);
    ui->modeSelector->blockSignals(false);

    refreshRds();
}

void VfoOpt::onFilterChanged() { refreshFilterCombo(); }

void VfoOpt::onFilterOffsetChanged()
{
    refreshFreqSpinBox();
    refreshFilterFreq();
}

void VfoOpt::onCenterFreqChanged() { refreshFilterFreq(); }

void VfoOpt::refreshFilterFreq()
{
    if (vfo->parentModel())
        ui->filterFreq->setFrequency(vfo->filterOffset() +
                                     vfo->parentModel()->centerFreq());
}

void VfoOpt::on_modeSelector_currentIndexChanged(int index)
{
    // set old index
    int oldIndex = (int)vfo->demod();
    Demod demod = (Demod)index;

    ui->modeSelector->blockSignals(true);
    ui->modeSelector->setCurrentIndex(oldIndex);
    ui->modeSelector->blockSignals(false);

    vfo->setDemod(demod);
}

void VfoOpt::refreshAgcPresetCombo()
{
    for (int i = 0; i < EAgcPreset::COUNT; i++) {
        AgcPreset preset = AgcPresets[i];

        if (preset.on == vfo->agcOn() &&
            (!preset.slope.has_value() ||
             preset.slope.value() == vfo->agcSlope()) &&
            (!preset.decay.has_value() ||
             preset.decay.value() == vfo->agcDecay())) {

            ui->agcPresetCombo->blockSignals(true);
            ui->agcPresetCombo->setCurrentIndex(i);
            ui->agcPresetCombo->blockSignals(false);
            return;
        }
    }

    ui->agcPresetCombo->blockSignals(true);
    ui->agcPresetCombo->setCurrentIndex(EAgcPreset::USER);
    ui->agcPresetCombo->blockSignals(false);
}

void VfoOpt::on_modeButton_clicked() { demodOpt->show(); }

void VfoOpt::on_agcButton_clicked() { agcOpt->show(); }

void VfoOpt::on_autoSquelchButton_clicked()
{
    vfo->getSignalPower().then(this, [this](float pwr) {
        pwr += 3.0;
        vfo->setSqlLevel(pwr);
    });
}

void VfoOpt::on_sqlSpinBox_valueChanged(double value)
{
    refreshSqlSpinBox();

    vfo->setSqlLevel(value);
}

void VfoOpt::on_resetSquelchButton_clicked() { vfo->setSqlLevel(-150.0); }

void VfoOpt::on_agcPresetCombo_currentIndexChanged(int index)
{
    // if new perset is not "USER" revert back until the request is accepted
    // and the corresponding signal is emitted
    if (index != EAgcPreset::USER)
        refreshAgcPresetCombo();

    setAgcPreset(index);
}

/** Noise blanker 1 button has been toggled. */
void VfoOpt::on_nb1Button_toggled(bool checked)
{
    refreshNoiseBlankerButtons();
    vfo->setNoiseBlanker(1, checked);
}

/** Noise blanker 2 button has been toggled. */
void VfoOpt::on_nb2Button_toggled(bool checked)
{
    refreshNoiseBlankerButtons();
    vfo->setNoiseBlanker(2, checked);
}

void VfoOpt::on_nbOptButton_clicked() { nbOpt->show(); }

void VfoOpt::on_audioPlayButton_toggled(bool checked)
{
    if (checked) {
        try {
            audioPlayer->play();
        } catch (const std::exception& e) {
            QMessageBox::critical(nullptr, "Audio Error", e.what(),
                                  QMessageBox::Ok, QMessageBox::NoButton);
        }
    } else {
        try {
            audioPlayer->stop();
        } catch (const std::exception& e) {
            QMessageBox::critical(nullptr, "Audio Error", e.what(),
                                  QMessageBox::Ok, QMessageBox::NoButton);
        }
    }
}

void VfoOpt::on_audioPosSlider_sliderMoved(int value)
{
    quint32 pos =
        (value * audioPlayer->duration()) / ui->audioPosSlider->maximum();

    audioPlayer->setPosition(pos);
}

void VfoOpt::setAgcPreset(int presetIdx)
{
    AgcPreset preset = AgcPresets[presetIdx];

    vfo->setAgcOn(preset.on);
    if (preset.decay)
        vfo->setAgcDecay(preset.decay.value());
    if (preset.slope)
        vfo->setAgcSlope(preset.slope.value());
}

void VfoOpt::onAgcParamChanged() { refreshAgcPresetCombo(); }

void VfoOpt::onSignalPowerTimerTimeout()
{
    if (!signalPower.isValid()) {
        signalPower = vfo->getSignalPower();
    }

    if (!signalPower.isFinished())
        return;

    float pwr = signalPower.takeResult();
    ui->meter->setLevel(pwr);
}

void VfoOpt::onNoiseBlankerOnChanged() { refreshNoiseBlankerButtons(); }

void VfoOpt::onSqlChanged()
{
    refreshSqlSpinBox();
    refreshMeter();
}

void VfoOpt::onAudioGainChanged()
{
    refreshAudioGainSlider();
    refreshAudioGainLabel();
    refreshMuteButton();
}

void VfoOpt::onAudioRecordingStarted(const QString& path)
{
    lastAudio = path;
    refreshRecordButton();
}

void VfoOpt::onAudioRecordingStopped()
{
    try {
        audioPlayer->setSource(QUrl::fromLocalFile(lastAudio));
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Audio Error", e.what(), QMessageBox::Ok,
                              QMessageBox::NoButton);
    }
    refreshRecordButton();
}

void VfoOpt::onUdpStreamingStarted(const QString& host, int port, bool stereo)
{
    audioOptions->setUdpHost(host);
    audioOptions->setUdpPort(port);
    audioOptions->setUdpStereo(stereo);
    audioOptions->setUdpOptionsEnabled(false);

    refreshAudioStreamButton();
}

void VfoOpt::onUdpStreamingStopped()
{
    audioOptions->setUdpOptionsEnabled(true);
    refreshAudioStreamButton();
}

void VfoOpt::onNewRecDirSelected(const QString& dir) { recDir = dir; }

void VfoOpt::refreshMuteButton()
{
    float gain = vfo->audioGain();

    ui->audioMuteButton->blockSignals(true);
    if (gain < -1000) {
        ui->audioMuteButton->setChecked(true);
    } else {
        ui->audioMuteButton->setChecked(false);
    }
    ui->audioMuteButton->blockSignals(false);
}

void VfoOpt::refreshAudioGainSlider()
{
    float gain = vfo->audioGain();

    if (gain < -1000)
        return;

    ui->audioGainSlider->blockSignals(true);
    ui->audioGainSlider->setValue(std::round(gain * 10.0));
    ui->audioGainSlider->blockSignals(false);
}

void VfoOpt::refreshAudioGainLabel()
{
    float gain = vfo->audioGain();

    if (gain < -1000)
        return;

    ui->audioGainDbLabel->setText(QString("%1 dB").arg(gain, 5, 'f', 1));
}

void VfoOpt::refreshNoiseBlankerButtons()
{
    ui->nb1Button->blockSignals(true);
    ui->nb1Button->setChecked(vfo->noiseBlanker1On());
    ui->nb1Button->blockSignals(false);

    ui->nb2Button->blockSignals(true);
    ui->nb2Button->setChecked(vfo->noiseBlanker2On());
    ui->nb2Button->blockSignals(false);
}

void VfoOpt::refreshSqlSpinBox()
{
    ui->sqlSpinBox->blockSignals(true);
    ui->sqlSpinBox->setValue(vfo->sqlLevel());
    ui->sqlSpinBox->blockSignals(false);
}

void VfoOpt::refreshRecordButton()
{
    if (vfo->isRecordingAudio()) {
        QFileInfo info(lastAudio);

        ui->audioRecButton->blockSignals(true);
        ui->audioRecButton->setChecked(true);
        ui->audioRecButton->blockSignals(false);

        ui->audioRecButton->setToolTip(tr("Stop audio recorder"));

        /* prevent playback while recording */
        ui->audioPlayButton->setEnabled(false);
    } else {
        ui->audioRecButton->blockSignals(true);
        ui->audioRecButton->setChecked(false);
        ui->audioRecButton->blockSignals(false);

        ui->audioRecButton->setToolTip(tr("Start audio recorder"));

        if (!lastAudio.isEmpty())
            ui->audioPlayButton->setEnabled(true);
    }
}

void VfoOpt::refreshMeter() { ui->meter->setSqlLevel(vfo->sqlLevel()); }

void VfoOpt::refreshAudioStreamButton()
{
    ui->audioStreamButton->blockSignals(true);
    ui->audioStreamButton->setChecked(vfo->isUdpStreaming());
    ui->audioStreamButton->blockSignals(false);
}

void VfoOpt::onAudioPlayerPositionChanged(quint32 pos)
{
    quint32 mins = (pos / (1000 * 60));
    quint32 secs = (pos / 1000) - mins * 60;

    // FIXME: I should probably use QString utilities, but meh!
    size_t size =
        fmt::format_to_n((int16_t*)audioPosStr.data(), audioPosStr.capacity(),
                         "{:02}:{:02}", mins, secs)
            .size;
    audioPosStr.resize(size);

    ui->audioElapsedLabel->setText(audioPosStr);

    if (!ui->audioPosSlider->isSliderDown()) {
        ui->audioPosSlider->blockSignals(true);
        ui->audioPosSlider->setSliderPosition(
            (pos * ui->audioPosSlider->maximum()) / audioPlayer->duration());
        ui->audioPosSlider->blockSignals(false);
    }
}

void VfoOpt::onAudioPlayerDurationChanged(quint32 duration)
{
    quint32 mins = (duration / (1000 * 60));
    quint32 secs = (duration / 1000) - mins * 60;

    // FIXME: I should probably use QString utilities, but meh!
    size_t size =
        fmt::format_to_n((int16_t*)durationStr.data(), durationStr.capacity(),
                         "{:02}:{:02}", mins, secs)
            .size;
    durationStr.resize(size);

    ui->audioDurationLabel->setText(durationStr);
}

void VfoOpt::onAudioPlayerPlaybackStateChanged(int state)
{
    ui->audioPlayButton->blockSignals(true);
    switch (state) {
    case AudioPlayer::PlaybackState::PlayingState:
        ui->audioPlayButton->setChecked(true);
        break;
    case AudioPlayer::PlaybackState::StoppedState:
        ui->audioPlayButton->setChecked(false);
        break;
    default:
        break;
    }
    ui->audioPlayButton->blockSignals(false);
}

void VfoOpt::onAudioPlayerPlaybackFinished()
{
    ui->audioPlayButton->blockSignals(true);
    ui->audioPlayButton->setChecked(false);
    ui->audioPlayButton->blockSignals(false);

    audioPlayer->setPosition(0);
}

void VfoOpt::setupRds()
{
    rdsTimer = new QTimer(this);
    connect(rdsTimer, &QTimer::timeout, this, &VfoOpt::rdsTimeout);

    connect(vfo, &VFOChannelModel::rdsDecoderStarted, this,
            &VfoOpt::onRdsDecoderStarted);
    connect(vfo, &VFOChannelModel::rdsDecoderStopped, this,
            &VfoOpt::onRdsDecoderStopped);
}

void VfoOpt::rdsTimeout()
{
    if (!waitingForRds) {
        waitingForRds = true;
        vfo->getRdsData()
            .then([this](const RdsData& rdsData) {
                waitingForRds = false;
                updateRds(rdsData);
            })
            .onFailed([this]() {
                waitingForRds = false;
                // TODO
            });
    }
}

void VfoOpt::updateRds(const RdsData& rdsData)
{
    QString out;

    /* type 0 = PI
     * type 1 = PS
     * type 2 = PTY
     * type 3 = flagstring: TP, TA, MuSp, MoSt, AH, CMP, stPTY
     * type 4 = RadioText
     * type 5 = ClockTime
     * type 6 = Alternative Frequencies
     */
    switch (rdsData.type) {
    case 0:
        ui->program_information->setText(rdsData.message);
        break;
    case 1:
        ui->station_name->setText(rdsData.message);
        break;
    case 2:
        ui->program_type->setText(rdsData.message);
        break;
    case 3:
        out = "";
        if (rdsData.message.at(0) == '1')
            out.append("TP ");
        if (rdsData.message.at(1) == '1')
            out.append("TA ");
        if (rdsData.message.at(2) == '0')
            out.append("Speech ");
        if (rdsData.message.at(2) == '1')
            out.append("Music ");
        if (rdsData.message.at(3) == '0')
            out.append("Stereo ");
        if (rdsData.message.at(3) == '1')
            out.append("Mono ");
        if (rdsData.message.at(4) == '1')
            out.append("AH ");
        if (rdsData.message.at(5) == '1')
            out.append("CMP ");
        if (rdsData.message.at(6) == '1')
            out.append("stPTY ");
        ui->flags->setText(out);
        break;
    case 4:
        ui->radiotext->setText(rdsData.message);
        break;
    case 5:
        ui->clocktime->setText(rdsData.message);
        break;
    case 6:
        ui->alt_freq->setText(rdsData.message);
        break;
    default:
        // nothing to do
        break;
    }
}

void VfoOpt::refreshRds()
{
    refreshRdsCheckbox();

    ui->rdsSection->setEnabled(vfo->supportsRds());

    ui->program_information->setText("");
    ui->station_name->setText("");
    ui->program_type->setText("");
    ui->radiotext->setText("");
    ui->alt_freq->setText("");
    ui->clocktime->setText("");
    ui->flags->setText("");
}

void VfoOpt::refreshRdsCheckbox()
{
    ui->rdsCheckbox->blockSignals(true);
    ui->rdsCheckbox->setChecked(vfo->isRdsDecoderActive());
    ui->rdsCheckbox->blockSignals(false);
}

void VfoOpt::on_rdsCheckbox_toggled(bool checked)
{
    refreshRdsCheckbox();

    vfo->resetRdsParser();

    if (checked)
        vfo->startRdsDecoder();
    else
        vfo->stopRdsDecoder();
}

void VfoOpt::onRdsDecoderStarted()
{
    refreshRds();
    rdsTimer->start(250);
}

void VfoOpt::onRdsDecoderStopped()
{
    refreshRdsCheckbox();
    rdsTimer->stop();
}
