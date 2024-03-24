#ifndef VFOOPT_H
#define VFOOPT_H

#include "qtclient/agc_options.h"
#include "qtclient/audio_options.h"
#include "qtclient/demod_options.h"
#include "qtclient/nb_options.h"

#include <QFuture>
#include <QWidget>

namespace Ui
{
class VfoOpt;
}

// not really in love with this
namespace violetrx
{
struct Filter;
}

class VFOChannelModel;
class QTimer;
class AudioPlayer;
struct RdsData;

class VfoOpt : public QWidget
{
    Q_OBJECT

public:
    explicit VfoOpt(VFOChannelModel* vfo, QWidget* parent = 0);
    ~VfoOpt();

private:
    unsigned int filterIdxFromLoHi(int lo, int hi) const;

    violetrx::Filter getFilter(int filterIndex, int filterShapeIndex);
    void setAgcPreset(int);

private Q_SLOTS:
    void on_freqSpinBox_valueChanged(double freq);
    void on_filterFreq_requestNewFrequency(qint64 freq);
    void on_filterCombo_currentIndexChanged(int index);
    void on_modeSelector_currentIndexChanged(int index);
    void on_modeButton_clicked();
    void on_agcButton_clicked();
    void on_autoSquelchButton_clicked();
    void on_resetSquelchButton_clicked();
    void on_agcPresetCombo_currentIndexChanged(int index);
    void on_sqlSpinBox_valueChanged(double value);
    void on_nb1Button_toggled(bool checked);
    void on_nb2Button_toggled(bool checked);
    void on_nbOptButton_clicked();
    void on_audioGainSlider_valueChanged(int value);
    void on_audioMuteButton_toggled(bool checked);
    void on_audioRecButton_toggled(bool checked);
    void on_audioStreamButton_toggled(bool checked);
    void on_audioConfButton_clicked();
    void on_audioPlayButton_toggled(bool checked);
    void on_audioPosSlider_sliderMoved(int value);
    void on_rdsCheckbox_toggled(bool checked);

    void onDemodChanged();
    void onFilterChanged();
    void onFilterOffsetChanged();
    void onCenterFreqChanged();
    void onAgcParamChanged();
    void onNoiseBlankerOnChanged();
    void onSqlChanged();
    void onAudioGainChanged();
    void onAudioRecordingStarted(const QString&);
    void onAudioRecordingStopped();
    void onUdpStreamingStarted(const QString& host, int port, bool stereo);
    void onUdpStreamingStopped();
    void onNewRecDirSelected(const QString& dir);
    void onRdsDecoderStarted();
    void onRdsDecoderStopped();

    void onAudioPlayerPositionChanged(quint32 pos);
    void onAudioPlayerDurationChanged(quint32 duration);
    void onAudioPlayerPlaybackStateChanged(int state);
    void onAudioPlayerPlaybackFinished();

private:
    void refreshFilterCombo();
    void refreshFreqSpinBox();
    void refreshFilterFreq();
    void refreshAgcPresetCombo();
    void refreshNoiseBlankerButtons();
    void refreshSqlSpinBox();
    void refreshMeter();
    void refreshAudioGainSlider();
    void refreshAudioGainLabel();
    void refreshMuteButton();
    void refreshRecordButton();
    void refreshAudioStreamButton();
    void refreshRds();
    void refreshRdsCheckbox();

    void onSignalPowerTimerTimeout();

    void setupRds();
    void rdsTimeout();
    void updateRds(const RdsData&);

private:
    Ui::VfoOpt* ui;              /** The Qt designer UI file. */
    CDemodOptions* demodOpt;     /** Demodulator options. */
    CAgcOptions* agcOpt;         /** AGC options. */
    CNbOptions* nbOpt;           /** Noise blanker options. */
    CAudioOptions* audioOptions; /*! Audio options dialog. */
    QTimer* signalPowerTimer;
    AudioPlayer* audioPlayer;
    QFuture<float> signalPower;

    VFOChannelModel* vfo;

    QString lastAudio;
    QString recDir;

    QString audioPosStr;
    QString durationStr;

    // rds
    QTimer* rdsTimer;
    bool waitingForRds;
};

#endif // VFOOPT_H
