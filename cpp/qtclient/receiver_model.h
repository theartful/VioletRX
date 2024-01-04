#ifndef RECEIVER_MODEL_H
#define RECEIVER_MODEL_H

#include <QColor>
#include <QFuture>
#include <QObject>
#include <QString>

#include <boost/signals2/connection.hpp>

#include "async_core/types.h"

// Not really necessary, but I would like to give this "style" a chance instead
// of qt's signals and slots
#include "bindable_properties.h"

class ReceiverModel;

using core::Demod;
using core::FilterRange;
using core::FilterShape;
using core::WindowType;
using Filter = core::Filter;
using Timestamp = core::Timestamp;

struct FftFrame {
    Timestamp timestamp;
    qint64 center_freq;
    qint64 sample_rate;
    std::vector<float> fft_points;
};

struct SnifferFrame {
    std::vector<float> samples;
};

struct RdsData {
    QString message;
    int type;
};

// YUCK: same as core::GainStage, but using QString instead of std::string
struct GainStage {
    QString name;
    double start;
    double stop;
    double step;
    double value;
};

struct FilterPreset {
    Filter wide;
    Filter normal;
    Filter narrow;
};

template <typename T>
using Property = bindable_properties::property<T>;

class VFOChannelModel : public QObject
{
    Q_OBJECT

    friend class ReceiverModel;

public:
    VFOChannelModel(ReceiverModel* parent, core::AsyncVfoIfaceSptr);
    ~VFOChannelModel();

    ReceiverModel* parentModel() const;

    /* filter */
    QFuture<void> setFilterOffset(qint64);
    QFuture<void> setFilter(qint64, qint64, FilterShape);
    QFuture<void> setFilter(Filter);
    QFuture<void> setCwOffset(qint64);
    QFuture<void> setDemod(Demod);

    QFuture<float> getSignalPower();

    /* Noise blanker */
    QFuture<void> setNoiseBlanker(int nbid, bool on);
    QFuture<void> setNoiseBlankerThreshold(int nbid, float threshold);

    /* Sql parameter */
    QFuture<void> setSqlLevel(double level_db);
    QFuture<void> setSqlAlpha(double alpha);

    /* AGC */
    QFuture<void> setAgcOn(bool agc_on);
    QFuture<void> setAgcHang(bool use_hang);
    QFuture<void> setAgcThreshold(int threshold);
    QFuture<void> setAgcSlope(int slope);
    QFuture<void> setAgcDecay(int decay_ms);
    QFuture<void> setAgcManualGain(int gain);

    /* FM parameters */
    QFuture<void> setFmMaxDev(float maxdev_hz);
    QFuture<void> setFmDeemph(double tau);

    /* AM parameters */
    QFuture<void> setAmDcr(bool enabled);

    /* AM-Sync parameters */
    QFuture<void> setAmSyncDcr(bool enabled);
    QFuture<void> setAmSyncPllBw(float pll_bw);

    /* Audio recording */
    // should these audio options be in the client side or the server side?
    // for the sake of simplicity i will assume it's on the server side for now
    QFuture<void> setAudioGain(float gain_db);
    QFuture<void> startAudioRecording(const QString& filename);
    QFuture<void> stopAudioRecording();

    /* UDP streaming */
    QFuture<void> startUdpStreaming(const QString host, int port, bool stereo);
    QFuture<void> stopUdpStreaming();

    /* sample sniffer */
    QFuture<void> startSniffer(int samplrate, int buffsize);
    QFuture<void> stopSniffer();
    QFuture<void> getSnifferData(SnifferFrame*);

    /* rds functions */
    QFuture<RdsData> getRdsData();
    QFuture<void> startRdsDecoder();
    QFuture<void> stopRdsDecoder();
    QFuture<void> resetRdsParser();
    bool supportsRds();

    // there is an argument to be made that filter presets should not be in the
    // model but i think its simpler, and it might be a step anyways if demods
    // are made to be plugins
    FilterPreset getFilterPreset(Demod) const;

    uint64_t getId() const;
    core::AsyncVfoIfaceSptr inner() { return vfo; }

    // user data
    void setActive(bool);
    void setColor(const QColor&);
    void setName(const QString&);

public:
    /* properties */
    Property<qint64> filterOffsetProperty() const;
    qint64 filterOffset() const;

    Property<qint64> cwOffsetProperty() const;
    qint64 cwOffset() const;

    Property<bool> isRdsDecoderActiveProperty() const;
    bool isRdsDecoderActive() const;

    Property<bool> isRecordingAudioProperty() const;
    bool isRecordingAudio() const;

    Property<float> audioGainProperty() const;
    float audioGain() const;

    Property<Demod> demodProperty() const;
    Demod demod() const;

    Property<FilterRange> filterRangeProperty() const;
    FilterRange filterRange() const;

    Property<FilterPreset> filterPresetProperty() const;
    FilterPreset filterPreset() const;

    Property<Filter> filterProperty() const;
    Filter filter() const;

    qint64 filterBw() const;

    Property<bool> agcOnProperty() const;
    bool agcOn() const;

    Property<bool> agcHangProperty() const;
    bool agcHang() const;

    Property<int> agcThresholdProperty() const;
    int agcThreshold() const;

    Property<int> agcSlopeProperty() const;
    int agcSlope() const;

    Property<int> agcDecayProperty() const;
    int agcDecay() const;

    Property<int> agcManualGainProperty() const;
    int agcManualGain() const;

    Property<float> fmMaxDevProperty() const;
    float fmMaxDev() const;

    Property<double> fmDeemphProperty() const;
    double fmDeemph() const;

    Property<bool> amDcrProperty() const;
    bool amDcr() const;

    Property<bool> amSyncDcrProperty() const;
    bool amSyncDcr() const;

    Property<float> amSyncPllBwProperty() const;
    float amSyncPllBw() const;

    Property<bool> noiseBlanker1OnProperty() const;
    bool noiseBlanker1On() const;

    Property<bool> noiseBlanker2OnProperty() const;
    bool noiseBlanker2On() const;

    Property<float> noiseBlanker1ThresholdProperty() const;
    float noiseBlanker1Threshold() const;

    Property<float> noiseBlanker2ThresholdProperty() const;
    float noiseBlanker2Threshold() const;

    Property<double> sqlLevelProperty() const;
    double sqlLevel() const;

    Property<double> sqlAlphaProperty() const;
    double sqlAlpha() const;

    Property<bool> isUdpStreamingProperty() const;
    bool isUdpStreaming() const;

    Property<bool> isActiveProperty() const;
    bool isActive() const;

    Property<QColor> colorProperty() const;
    QColor color() const;

    QString name() const;

    static QString demodAsString(Demod);
    static Demod demodFromString(const QString&);

Q_SIGNALS:
    void demodChanged(Demod);
    void filterPresetChanged(FilterPreset);
    void offsetChanged(qint64);
    void cwOffsetChanged(qint64);
    void filterChanged(qint64, qint64, FilterShape);
    void filterRangeChanged(FilterRange);

    /* Noise blanker */
    void noiseBlankerOnChanged(int, bool);
    void noiseBlankerThresholdChanged(int, float);

    /* Squelch parameter */
    void sqlLevelChanged(double);
    void sqlAlphaChanged(double);

    /* AGC */
    void agcOnChanged(bool agc_on);
    void agcHangChanged(bool use_hang);
    void agcThresholdChanged(int threshold);
    void agcSlopeChanged(int slope);
    void agcDecayChanged(int decay_ms);
    void agcManualGainChanged(int gain);

    /* FM parameters */
    void fmMaxDevChanged(float maxdev_hz);
    void fmDeemphChanged(double tau);

    /* AM parameters */
    void amDcrChanged(bool enabled);

    /* AM-Sync parameters */
    void amSyncDcrChanged(bool enabled);
    void amSyncPllBwChanged(float pll_bw);

    /* Audio recording */
    void audioGainChanged(float gain_db);
    void audioRecordingStarted(const QString& filename);
    void audioRecordingStopped();

    /* UDP streaming */
    void udpStreamingStarted(const QString& host, int port, bool stereo);
    void udpStreamingStopped();

    /* sample sniffer */
    void snifferStarted(int samplrate, int buffsize);
    void snifferStopped();

    /* rds functions */
    void rdsDecoderStarted(void);
    void rdsDecoderStopped();
    void rdsParserReset(void);

    /* user data */
    void activeStatusChanged(bool);
    void colorChanged(const QColor&);
    void nameChanged(const QString&);

    void removed();

private:
    void prepareToDie();
    void setActive_impl(bool);

    // void* instead of VfoEvent because I don't want to include the events
    // header
    void onStateChanged(const void*);
    void onDemodChanged(Demod);
    void onOffsetChanged(int64_t);
    void onCwOffsetChanged(int64_t);
    void onFilterChanged(Filter);
    void onStartedRecording();
    void onStoppedRecording();
    void onNoiseBlankerOnChanged(int, bool);
    void onNoiseBlankerThresholdChanged(int, float);
    void onSqlLevelChanged(double);
    void onSqlAlphaChanged(double);
    void onAgcOnChanged(bool);
    void onAgcHangChanged(bool);
    void onAgcThresholdChanged(int);
    void onAgcSlopeChanged(int);
    void onAgcDecayChanged(int);
    void onAgcManualGainChanged(int);
    void onFmMaxdevChanged(float);
    void onFmDeemphChanged(double);
    void onAmDcrChanged(bool);
    void onAmSyncDcrChanged(bool);
    void onAmSyncPllBwChanged(float);
    void onAudioGainChanged(float);
    void onRecordingStarted(std::string);
    void onRecordingStopped();
    void onUdpStreamingStarted(std::string, int, bool);
    void onUdpStreamingStopped();
    void onSnifferStarted(int, int);
    void onSnifferStopped();
    void onRdsDecoderStarted();
    void onRdsDecoderStopped();
    void onRdsParserReset();
    void onRemoved();

private:
    ReceiverModel* parent;
    core::AsyncVfoIfaceSptr vfo;

    /* properties */

    // filter and demod
    Property<Demod> m_demod;
    Property<FilterRange> m_filterRange;
    Property<FilterPreset> m_filterPreset;
    Property<Filter> m_filter;
    Property<qint64> m_filterOffset;
    Property<qint64> m_cwOffset;

    // FM parameters
    Property<float> m_fmMaxDev;
    Property<double> m_fmDeemph;

    // AM parameters
    Property<bool> m_amDcr;

    // AM-Sync parameters
    Property<bool> m_amSyncDcr;
    Property<float> m_amSyncPllBw;

    // audio
    Property<bool> m_recordingAudio;
    Property<bool> m_udpStreamingAudio;
    Property<float> m_audioGain;

    // decoders
    Property<bool> m_rdsDecoderActive;

    // agc
    Property<bool> m_agcOn;
    Property<bool> m_agcHang;
    Property<int> m_agcThreshold;
    Property<int> m_agcSlope;
    Property<int> m_agcDecay;
    Property<int> m_agcManualGain;

    // squelch
    Property<double> m_sqlLevel;
    Property<double> m_sqlAlpha;

    // noise blanker
    Property<bool> m_nb1On;
    Property<float> m_nb1Threshold;

    Property<bool> m_nb2On;
    Property<float> m_nb2Threshold;

    // user data
    Property<QColor> m_color;
    Property<bool> m_active;

    QString m_name;

    /* connections */
    boost::signals2::scoped_connection conStateChanged;
};

class ReceiverModel : public QObject
{
    Q_OBJECT

    friend class VFOChannelModel;

public:
    ReceiverModel(QObject* parent = nullptr);
    ReceiverModel(core::AsyncReceiverIfaceSptr, QObject* parent = nullptr);

    ~ReceiverModel();

    void subscribe();
    void unsubscribe();

    QFuture<void> start();
    QFuture<void> stop();

    QFuture<void> setInputDevice(const QString& device);
    QFuture<void> setOutputDevice(const QString& device);
    QList<QString> getAntennas();
    QFuture<void> setAntenna(const QString& antenna);
    QFuture<qint64> setInputRate(qint64 rate);
    QFuture<int> setInputDecim(int decim);
    QFuture<qint64> setAnalogBandwidth(qint64 bw);
    QFuture<void> setIqSwap(bool reversed);
    QFuture<void> setDcCancel(bool enable);
    QFuture<void> setIqBalance(bool enable);
    QFuture<qint64> setCenterFreq(qint64);
    QFuture<qint64> setHwFreq(qint64);
    QFuture<qint64> setLnbLo(qint64);
    QFuture<void> setAutoGain(bool automatic);
    QFuture<double> setGain(const QString& name, double value);
    QFuture<double> setFreqCorr(double ppm);
    QFuture<void> setIqFftSize(int newsize);
    QFuture<void> setIqFftWindow(WindowType window_type, bool normalize_energy);
    QFuture<void> getIqFftData(FftFrame* frame);

    /* I/Q recording and playback */
    QFuture<void> startIqRecording(const QString& filename);
    QFuture<void> stopIqRecording();
    QFuture<void> seekIqFile(long pos);

    QFuture<VFOChannelModel*> addVFOChannel();
    QFuture<void> removeVFOChannel(VFOChannelModel*);

    const QList<VFOChannelModel*>& vfoChannels() { return vfos; }

    // user data
    void setActiveVfo(VFOChannelModel*);
    VFOChannelModel* activeVfo();

Q_SIGNALS:
    void started();
    void stopped();
    void inputDeviceChanged(const QString& device);
    void outputDeviceChanged(const QString& device);
    void antennaChanged(const QString& antenna);
    void inputRateChanged(qint64 rate);
    void inputDecimChanged(int);
    void analogBandwidthChanged(qint64);
    void iqSwapChanged(bool);
    void dcCancelChanged(bool);
    void iqBalanceChanged(bool);
    void centerFreqChanged(qint64);
    void lnbLoChanged(qint64);
    void hwFreqChanged(qint64);
    void autoGainChanged(bool);
    void gainChanged(const QString& name, double value);
    void freqCorrChanged(double ppm);
    void iqFftSizeChanged(int newsize);
    void iqFftWindowChanged(WindowType window_type, bool normalize_energy);
    void iqRecordingStarted(const QString& filename);
    void iqRecordingStopped();
    void iqFileSeeked(long pos);

    void vfoAdded(VFOChannelModel*);
    void vfoRemoved(VFOChannelModel*);
    void subscribed();
    void unsubscribed();

    void activeVfoChanged(VFOChannelModel*);

public:
    QList<GainStage> getGainStages() const;

    Property<bool> isRunningProperty() const;
    bool isRunning() const;

    /* properties */
    Property<qint64> lnbLoProperty() const;
    qint64 lnbLo() const;

    Property<qint64> hwFreqProperty() const;
    qint64 hwFreq() const;

    Property<qint64> centerFreqProperty() const;
    qint64 centerFreq() const;

    Property<qint64> inputRateProperty() const;
    qint64 inputRate() const;

    Property<int> inputDecimProperty() const;
    int inputDecim() const;

    Property<int> iqFftSizeProperty() const;
    int iqFftSize() const;

private:
    // void* instead of ReceiverEvent because I don't want to include the events
    // header
    void onStateChanged(const void*);
    void onSubscribed();
    void onUnsubscribed();
    void onSyncStart();
    void onSyncEnd();
    void onStarted();
    void onStopped();
    void onInputDeviceChanged(std::string);
    void onAntennaChanged(std::string);
    void onInputRateChanged(int64_t);
    void onInputDecimChanged(int);
    void onIqSwapChanged(bool);
    void onDcCancelChanged(bool);
    void onIqBalanceChanged(bool);
    void onCenterFreqChanged(int64_t);
    void onRfFreqChanged(int64_t);
    void onGainStagesChanged(std::vector<core::GainStage>);
    void onAntennasChanged(std::vector<std::string>);
    void onAutoGainChanged(bool);
    void onGainChanged(std::string, double);
    void onFreqCorrChanged(double);
    void onIqFftSizeChanged(int);
    void onIqFftWindowChanged(WindowType, bool);
    void onIqRecordingStarted(std::string);
    void onIqRecordingStopped();
    void onVfoAdded(core::AsyncVfoIfaceSptr);
    void onVfoRemoved(core::AsyncVfoIfaceSptr);

    VFOChannelModel* addVfoIfDoesntExist(core::AsyncVfoIfaceSptr);

private:
    core::AsyncReceiverIfaceSptr rx;
    QList<VFOChannelModel*> vfos;

    VFOChannelModel* m_activeVfo;

    /* properties */
    Property<bool> m_running;
    Property<qint64> m_lnbLo;
    Property<qint64> m_hwFreq;
    Property<qint64> m_centerFreq;
    Property<qint64> m_inputRate;
    Property<int> m_inputDecim;
    Property<int> m_fftSize;

    QList<GainStage> m_gainStages;
    QList<QString> m_antennas;

    /* connections */
    boost::signals2::scoped_connection conStateChanged;
};

#endif // RECEIVER_MODEL_H
