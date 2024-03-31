#include <algorithm>
#include <chrono>
#include <exception>
#include <functional>
#include <stdexcept>
#include <variant>

#include <QRandomGenerator>

#include <spdlog/spdlog.h>

#include "async_core/async_receiver.h"
#include "async_core/async_receiver_iface.h"
#include "async_core/async_vfo_iface.h"
#include "async_core/error_codes.h"
#include "async_core/events.h"
#include "async_core/events_format.h"
#include "async_core/types.h"
#include "receiver_model.h"

#define DEFAULT_VOID_CALLBACK                                                  \
    [promise = std::move(promise)](violetrx::ErrorCode code) mutable {         \
        if (code == violetrx::ErrorCode::OK) {                                 \
            promise.start();                                                   \
            promise.finish();                                                  \
        } else {                                                               \
            promise.start();                                                   \
            promise.setException(makeException(code));                         \
            promise.finish();                                                  \
        }                                                                      \
    }

#define DEFAULT_ARG_CALLBACK                                                   \
    [promise = std::move(promise)](violetrx::ErrorCode code,                   \
                                   auto arg) mutable {                         \
        if (code == violetrx::ErrorCode::OK) {                                 \
            promise.start();                                                   \
            promise.addResult(std::move(arg));                                 \
            promise.finish();                                                  \
        } else {                                                               \
            promise.start();                                                   \
            promise.setException(makeException(code));                         \
            promise.finish();                                                  \
        }                                                                      \
    }

ReceiverModel::ReceiverModel(QObject* parent) :
    ReceiverModel(violetrx::AsyncReceiver::make(), parent)
{
}

ReceiverModel::ReceiverModel(violetrx::AsyncReceiverIface::sptr rx_,
                             QObject* parent) :
    QObject(parent),
    rx(rx_),
    m_activeVfo(nullptr),
    m_lnbLo(0),
    m_hwFreq(0),
    m_centerFreq(m_hwFreq),
    m_inputRate(0),
    m_inputDecim(1),
    m_fftSize(receiver::DEFAULT_FFT_SIZE)
{
}

ReceiverModel::~ReceiverModel() {}

static inline std::exception_ptr makeException(violetrx::ErrorCode error)
{
    return std::make_exception_ptr(
        std::runtime_error(violetrx::errorMsg(error)));
}

void ReceiverModel::subscribe()
{
    rx->subscribe(
        [this](const violetrx::ReceiverEvent& e) {
            spdlog::info("ReceiverModel: {}", e);
            onStateChanged(static_cast<const void*>(&e));
        },
        [this](violetrx::ErrorCode err, violetrx::Connection connection) {
            if (err == violetrx::ErrorCode::OK) {
                conStateChanged = std::move(connection);
                onSubscribed();
            } else {
                // TODO
            }
        });
}

void ReceiverModel::unsubscribe()
{
    rx->unsubscribe(conStateChanged);
    onUnsubscribed();
}

void ReceiverModel::onStateChanged(const void* event_erased)
{
    // this function runs in the async receiver thread
    // so we have to dispatch using invokeMethod

#define INVOKE_METHOD(func)                                                    \
    QMetaObject::invokeMethod(this, [=, this]() { func; })

    using namespace violetrx;

    const ReceiverEvent& e = *static_cast<const ReceiverEvent*>(event_erased);
    std::visit(
        Visitor{
            [&](const SyncStart&) { INVOKE_METHOD(onSyncStart()); },
            [&](const SyncEnd&) { INVOKE_METHOD(onSyncEnd()); },
            [&](const Unsubscribed&) { INVOKE_METHOD(onUnsubscribed()); },
            [&](const Started&) { INVOKE_METHOD(onStarted()); },
            [&](const Stopped&) { INVOKE_METHOD(onStopped()); },
            [&](const InputDeviceChanged& ev) {
                INVOKE_METHOD(onInputDeviceChanged(ev.device));
            },
            [&](const AntennaChanged& ev) {
                INVOKE_METHOD(onAntennaChanged(ev.antenna));
            },
            [&](const AntennasChanged& ev) {
                INVOKE_METHOD(onAntennasChanged(ev.antennas));
            },
            [&](const InputRateChanged& ev) {
                INVOKE_METHOD(onInputRateChanged(ev.input_rate));
            },
            [&](const RfFreqChanged& ev) {
                INVOKE_METHOD(onInputRateChanged(ev.freq));
            },
            [&](const InputDecimChanged& ev) {
                INVOKE_METHOD(onInputDecimChanged(ev.decim));
            },
            [&](const IqSwapChanged& ev) {
                INVOKE_METHOD(onIqSwapChanged(ev.enabled));
            },
            [&](const IqBalanceChanged& ev) {
                INVOKE_METHOD(onIqBalanceChanged(ev.enabled));
            },
            [&](const DcCancelChanged& ev) {
                INVOKE_METHOD(onDcCancelChanged(ev.enabled));
            },
            [&](const GainStagesChanged& ev) {
                INVOKE_METHOD(onGainStagesChanged(ev.stages));
            },
            [&](const AutoGainChanged& ev) {
                INVOKE_METHOD(onAutoGainChanged(ev.enabled));
            },
            [&](const GainChanged& ev) {
                INVOKE_METHOD(onGainChanged(ev.name, ev.value));
            },
            [&](const FftWindowChanged& ev) {
                INVOKE_METHOD(onIqFftWindowChanged(ev.window, false));
            },
            [&](const FftSizeChanged& ev) {
                INVOKE_METHOD(onIqFftSizeChanged(ev.size));
            },
            [&](const VfoAdded& ev) {
                auto vfo = rx->getVfo(ev.handle);
                INVOKE_METHOD(onVfoAdded(vfo));
            },
            [&](const VfoRemoved& ev) {
                auto vfo = rx->getVfo(ev.handle);
                INVOKE_METHOD(onVfoRemoved(vfo));
            },
            [&](const IqRecordingStarted& ev) {
                INVOKE_METHOD(onIqRecordingStarted(ev.path));
            },
            [&](const IqRecordingStopped&) {
                INVOKE_METHOD(onIqRecordingStopped());
            },
            [&](const FreqCorrChanged& ev) {
                INVOKE_METHOD(onFreqCorrChanged(ev.ppm));
            },
        },
        e);

#undef INVOKE_METHOD
}

QFuture<void> ReceiverModel::start()
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->start(DEFAULT_VOID_CALLBACK);

    return future;
}
QFuture<void> ReceiverModel::stop()
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->stop(DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> ReceiverModel::setInputDevice(const QString& device)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->setInputDevice(device.toStdString(), DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> ReceiverModel::setOutputDevice(const QString&)
{
    // TODO?
    return QtFuture::makeReadyFuture();
}

QList<QString> ReceiverModel::getAntennas() { return m_antennas; }

QFuture<void> ReceiverModel::setAntenna(const QString& antenna)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->setAntenna(antenna.toStdString(), DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<qint64> ReceiverModel::setInputRate(qint64 rate)
{
    QPromise<qint64> promise;
    QFuture<qint64> future = promise.future();

    rx->setInputRate(rate, DEFAULT_ARG_CALLBACK);

    return future;
}

QFuture<int> ReceiverModel::setInputDecim(int decim)
{
    QPromise<int> promise;
    QFuture<int> future = promise.future();

    rx->setInputDecim(decim, DEFAULT_ARG_CALLBACK);

    return future;
}

QFuture<qint64> ReceiverModel::setAnalogBandwidth(qint64 bw)
{
    // TODO
    return QtFuture::makeReadyFuture(bw);
}

QFuture<void> ReceiverModel::setIqSwap(bool reversed)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->setIqSwap(reversed, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> ReceiverModel::setDcCancel(bool enable)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->setDcCancel(enable, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> ReceiverModel::setIqBalance(bool enable)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->setIqBalance(enable, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<qint64> ReceiverModel::setCenterFreq(qint64 freq)
{
    return setHwFreq(freq - m_lnbLo).then(this, [this](qint64 hwfreq) {
        return hwfreq + m_lnbLo;
    });
}

QFuture<qint64> ReceiverModel::setHwFreq(qint64 freq)
{
    QPromise<qint64> promise;
    QFuture<qint64> future = promise.future();

    rx->setRfFreq(freq, DEFAULT_ARG_CALLBACK);

    return future;
}

QFuture<qint64> ReceiverModel::setLnbLo(qint64 freq)
{
    m_lnbLo = freq;
    m_centerFreq = m_hwFreq + m_lnbLo;

    Q_EMIT lnbLoChanged(m_lnbLo);
    Q_EMIT centerFreqChanged(m_centerFreq);

    return QtFuture::makeReadyFuture(freq);
}

QList<GainStage> ReceiverModel::getGainStages() const { return m_gainStages; }

QFuture<void> ReceiverModel::setAutoGain(bool enable)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->setAutoGain(enable, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<double> ReceiverModel::setGain(const QString& name, double value)
{
    QPromise<double> promise;
    QFuture<double> future = promise.future();

    rx->setGain(name.toStdString(), value, DEFAULT_ARG_CALLBACK);

    return future;
}

QFuture<double> ReceiverModel::setFreqCorr(double ppm)
{
    QPromise<double> promise;
    QFuture<double> future = promise.future();

    rx->setFreqCorr(ppm, DEFAULT_ARG_CALLBACK);

    return future;
}

QFuture<void> ReceiverModel::setIqFftSize(int newsize)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->setIqFftSize(newsize, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> ReceiverModel::setIqFftWindow(WindowType window, bool normalize)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->setIqFftWindow(window, normalize, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> ReceiverModel::getIqFftData(FftFrame* frame)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    frame->fft_points.resize(m_fftSize);

    auto callback = [promise = std::move(promise),
                     frame](violetrx::ErrorCode code,
                            violetrx::Timestamp timestamp, int64_t centerFreq,
                            int sample_rate, float*, int size) mutable {
        if (code == violetrx::ErrorCode::OK) {
            frame->timestamp = timestamp;
            frame->center_freq = centerFreq;
            frame->sample_rate = sample_rate;
            frame->fft_points.resize(size);

            promise.start();
            promise.finish();

        } else {
            promise.start();
            promise.setException(makeException(code));
            promise.finish();
        }
    };

    rx->getIqFftData(frame->fft_points.data(), m_fftSize, std::move(callback));

    return future;
}

QFuture<void> ReceiverModel::startIqRecording(const QString& filename)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->startIqRecording(filename.toStdString(), DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> ReceiverModel::stopIqRecording()
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->stopIqRecording(DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> ReceiverModel::seekIqFile(long /* pos */)
{
    // TODO
    return QtFuture::makeReadyFuture();
}

QFuture<VFOChannelModel*> ReceiverModel::addVFOChannel()
{
    QPromise<VFOChannelModel*> promise;
    QFuture<VFOChannelModel*> future = promise.future();

    auto callback = [this, promise = std::move(promise)](
                        violetrx::ErrorCode code,
                        violetrx::AsyncVfoIfaceSptr vfo) mutable {
        if (code == violetrx::ErrorCode::OK) {
            // we have to call addVfoIfDoesntExist in the qt gui thread
            // to avoid the need for synchronization
            QMetaObject::invokeMethod(
                this, [this, vfo, promise = std::move(promise)]() mutable {
                    VFOChannelModel* vfoModel = addVfoIfDoesntExist(vfo);
                    promise.start();
                    promise.addResult(vfoModel);
                    promise.finish();
                });
        } else {
            promise.start();
            promise.setException(makeException(code));
            promise.finish();
        }
    };
    rx->addVfoChannel(std::move(callback));

    return future;
}

VFOChannelModel*
ReceiverModel::addVfoIfDoesntExist(violetrx::AsyncVfoIfaceSptr vfo)
{
    auto it = std::find_if(vfos.begin(), vfos.end(), [&](auto& vfoModel) {
        return vfoModel->getId() == vfo->getId();
    });

    if (it != vfos.end())
        return *it;

    VFOChannelModel* vfoModel = new VFOChannelModel(this, vfo);

    vfos.push_back(vfoModel);
    Q_EMIT vfoAdded(vfoModel);

    setActiveVfo(vfoModel);

    return vfoModel;
}

QFuture<void> ReceiverModel::removeVFOChannel(VFOChannelModel* vfo)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    rx->removeVfoChannel(vfo->inner());

    return future;
}

void ReceiverModel::onSubscribed() { Q_EMIT subscribed(); }
void ReceiverModel::onUnsubscribed() { Q_EMIT unsubscribed(); }
void ReceiverModel::onSyncStart()
{
    //
}
void ReceiverModel::onSyncEnd()
{
    //
}
void ReceiverModel::onStarted()
{
    m_running = true;
    Q_EMIT started();
}
void ReceiverModel::onStopped()
{
    m_running = false;
    Q_EMIT stopped();
}
void ReceiverModel::onInputDeviceChanged(std::string dev)
{
    Q_EMIT inputDeviceChanged(QString::fromStdString(dev));
}
void ReceiverModel::onAntennaChanged(std::string antenna)
{
    Q_EMIT antennaChanged(QString::fromStdString(antenna));
}
void ReceiverModel::onInputRateChanged(int64_t rate)
{
    m_inputRate = rate;
    Q_EMIT inputRateChanged(rate);
}
void ReceiverModel::onInputDecimChanged(int decim)
{
    m_inputDecim = decim;
    Q_EMIT inputDecimChanged(decim);
}
void ReceiverModel::onIqSwapChanged(bool enable)
{
    Q_EMIT iqSwapChanged(enable);
}
void ReceiverModel::onDcCancelChanged(bool enable)
{
    Q_EMIT dcCancelChanged(enable);
}
void ReceiverModel::onIqBalanceChanged(bool enable)
{
    Q_EMIT iqBalanceChanged(enable);
}
void ReceiverModel::onCenterFreqChanged(int64_t)
{
    // TODO: add lnb to async receiver
    // currently this will never be invoked
}
void ReceiverModel::onRfFreqChanged(int64_t freq)
{
    m_hwFreq = freq;
    m_centerFreq = m_hwFreq + m_lnbLo;

    Q_EMIT hwFreqChanged(m_hwFreq);
    Q_EMIT centerFreqChanged(m_centerFreq);
}
void ReceiverModel::onGainStagesChanged(
    std::vector<violetrx::GainStage> gainStages_std)
{
    m_gainStages.clear();
    m_gainStages.resize(gainStages_std.size());
    for (auto& stage : gainStages_std) {
        m_gainStages.push_back(GainStage{QString::fromStdString(stage.name),
                                         stage.start, stage.stop, stage.step,
                                         stage.value

        });
    }
}
void ReceiverModel::onAntennasChanged(std::vector<std::string> antennas_std)
{
    m_antennas.clear();
    m_antennas.resize(antennas_std.size());

    m_antennas.resize(antennas_std.size());
    for (auto& antenna : antennas_std) {
        m_antennas.push_back(QString::fromStdString(antenna));
    }
}
void ReceiverModel::onAutoGainChanged(bool enable)
{
    Q_EMIT autoGainChanged(enable);
}
void ReceiverModel::onGainChanged(std::string gain_std, double value)
{
    QString gain = QString::fromStdString(gain_std);

    auto it = std::find_if(
        m_gainStages.begin(), m_gainStages.end(),
        [&](const GainStage& stage) { return stage.name == gain; });

    if (it != m_gainStages.end()) {
        it->value = value;
    } else {
        spdlog::error("Gain stage ({}) change to ({}), but I can't find it any "
                      "my registered stages!",
                      gain_std, value);
    }

    Q_EMIT gainChanged(gain, value);
}
void ReceiverModel::onFreqCorrChanged(double ppm)
{
    Q_EMIT freqCorrChanged(ppm);
}
void ReceiverModel::onIqFftSizeChanged(int size)
{
    m_fftSize = size;
    Q_EMIT iqFftSizeChanged(size);
}
void ReceiverModel::onIqFftWindowChanged(WindowType window, bool normalize)
{
    Q_EMIT iqFftWindowChanged(window, normalize);
}
void ReceiverModel::onIqRecordingStarted(std::string filename)
{
    Q_EMIT iqRecordingStarted(QString::fromStdString(filename));
}
void ReceiverModel::onIqRecordingStopped() { Q_EMIT iqRecordingStopped(); }

void ReceiverModel::onVfoAdded(violetrx::AsyncVfoIfaceSptr vfo)
{
    addVfoIfDoesntExist(vfo);
}
void ReceiverModel::onVfoRemoved(violetrx::AsyncVfoIfaceSptr vfo)
{
    auto it = std::find_if(vfos.begin(), vfos.end(), [&](auto& vfoModel) {
        return vfoModel->getId() == vfo->getId();
    });

    if (it == vfos.end()) {
        spdlog::error("Attempted to remove vfo with id {} but did not find it!",
                      vfo->getId());
        return;
    }

    VFOChannelModel* vfoModel = *it;

    vfos.erase(it);

    vfoModel->prepareToDie();
    Q_EMIT vfoRemoved(vfoModel);

    vfoModel->deleteLater();

    if (vfoModel == m_activeVfo) {
        if (vfos.empty()) {
            setActiveVfo(nullptr);
        } else {
            setActiveVfo(vfos.first());
        }
    }
}

Property<bool> ReceiverModel::isRunningProperty() const { return m_running; }
bool ReceiverModel::isRunning() const { return m_running; }

Property<qint64> ReceiverModel::lnbLoProperty() const { return m_lnbLo; }
qint64 ReceiverModel::lnbLo() const { return m_lnbLo; }

Property<qint64> ReceiverModel::hwFreqProperty() const { return m_hwFreq; }
qint64 ReceiverModel::hwFreq() const { return m_hwFreq; }

Property<qint64> ReceiverModel::centerFreqProperty() const
{
    return m_centerFreq;
}
qint64 ReceiverModel::centerFreq() const { return m_centerFreq; }

Property<qint64> ReceiverModel::inputRateProperty() const
{
    return m_inputRate;
}
qint64 ReceiverModel::inputRate() const { return m_inputRate; }

Property<int> ReceiverModel::inputDecimProperty() const { return m_inputDecim; }
int ReceiverModel::inputDecim() const { return m_inputDecim; }

Property<int> ReceiverModel::iqFftSizeProperty() const { return m_fftSize; }
int ReceiverModel::iqFftSize() const { return m_fftSize; }

void ReceiverModel::setActiveVfo(VFOChannelModel* vfo)
{
    if (vfo == m_activeVfo)
        return;

    if (vfo) {
        auto it = std::find(vfos.begin(), vfos.end(), vfo);
        if (it == vfos.end()) {
            spdlog::error("ReceiverModel::setActiveVfo vfo is not part of "
                          "the receiver!");
            return;
        }
    }

    if (m_activeVfo) {
        m_activeVfo->setActive_impl(false);
    }

    m_activeVfo = vfo;

    if (m_activeVfo) {
        m_activeVfo->setActive_impl(true);
    }

    Q_EMIT activeVfoChanged(vfo);
}

VFOChannelModel* ReceiverModel::activeVfo() { return m_activeVfo; }

VFOChannelModel::VFOChannelModel(ReceiverModel* parent_,
                                 violetrx::AsyncVfoIfaceSptr vfo_) :
    QObject(parent_),
    parent(parent_),
    vfo(vfo_),
    m_filterOffset(0),
    m_fmMaxDev(5000),
    m_fmDeemph(75.0e-6),
    m_amDcr(true),
    m_amSyncDcr(true),
    m_amSyncPllBw(0.001),
    m_recordingAudio(false),
    m_udpStreamingAudio(false),
    m_rdsDecoderActive(false),
    m_sqlLevel(-150.0),
    m_sqlAlpha(0.001),
    m_nb1On(false),
    m_nb1Threshold(3.3),
    m_nb2On(false),
    m_nb2Threshold(2.5),
    m_active(false)
{
    // properties
    // TODO: set property setters

    // A horrible way to choose a random color
    QColor color;
    color.setRed(QRandomGenerator::global()->bounded(0, 255));
    color.setGreen(QRandomGenerator::global()->bounded(0, 255));
    color.setBlue(QRandomGenerator::global()->bounded(0, 255));
    m_color = color;

    // connections
    vfo->subscribe(
        [this](const violetrx::VfoEvent& e) {
            onStateChanged(static_cast<const void*>(&e));
        },
        [this](violetrx::ErrorCode err, violetrx::Connection connection) {
            if (err == violetrx::ErrorCode::OK) {
                conStateChanged = std::move(connection);
            } else {
                // TODO
            }
        });
}

ReceiverModel* VFOChannelModel::parentModel() const { return parent; }

QFuture<void> VFOChannelModel::setFilterOffset(qint64 offset)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setFilterOffset(offset, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setFilter(Filter filter)
{
    return setFilter(filter.low, filter.high, filter.shape);
}

QFuture<void> VFOChannelModel::setFilter(qint64 low, qint64 high,
                                         FilterShape shape)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setFilter(low, high, shape, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setCwOffset(qint64 offset)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setCwOffset(offset, DEFAULT_VOID_CALLBACK);

    return future;
}

FilterPreset VFOChannelModel::getFilterPreset(Demod d) const
{
    switch (d) {
    case Demod::OFF:
        return FilterPreset{Filter{FilterShape::NORMAL, 0, 0},
                            Filter{FilterShape::NORMAL, 0, 0},
                            Filter{FilterShape::NORMAL, 0, 0}};
    case Demod::RAW:
        return FilterPreset{Filter{FilterShape::NORMAL, -15000, 15000},
                            Filter{FilterShape::NORMAL, -5000, 5000},
                            Filter{FilterShape::NORMAL, -1000, 1000}};
    case Demod::AM:
        return FilterPreset{Filter{FilterShape::NORMAL, -10000, 10000},
                            Filter{FilterShape::NORMAL, -5000, 5000},
                            Filter{FilterShape::NORMAL, -2500, 2500}};
    case Demod::AM_SYNC:
        return FilterPreset{Filter{FilterShape::NORMAL, -10000, 10000},
                            Filter{FilterShape::NORMAL, -5000, 5000},
                            Filter{FilterShape::NORMAL, -2500, 2500}};
    case Demod::LSB:
        return FilterPreset{Filter{FilterShape::NORMAL, -4000, -100},
                            Filter{FilterShape::NORMAL, -2800, -100},
                            Filter{FilterShape::NORMAL, -2400, -300}};
    case Demod::USB:
        return FilterPreset{Filter{FilterShape::NORMAL, 100, 4000},
                            Filter{FilterShape::NORMAL, 100, 2800},
                            Filter{FilterShape::NORMAL, 300, 2400}};
    case Demod::CWL:
        return FilterPreset{Filter{FilterShape::NORMAL, -1000, 1000},
                            Filter{FilterShape::NORMAL, -250, 250},
                            Filter{FilterShape::NORMAL, -100, 100}};
    case Demod::CWU:
        return FilterPreset{Filter{FilterShape::NORMAL, -1000, 1000},
                            Filter{FilterShape::NORMAL, -250, 250},
                            Filter{FilterShape::NORMAL, -100, 100}};
    case Demod::NFM:
        return FilterPreset{Filter{FilterShape::NORMAL, -10000, 10000},
                            Filter{FilterShape::NORMAL, -5000, 5000},
                            Filter{FilterShape::NORMAL, -2500, 2500}};
    case Demod::WFM_MONO:
        return FilterPreset{Filter{FilterShape::NORMAL, -100000, 100000},
                            Filter{FilterShape::NORMAL, -80000, 80000},
                            Filter{FilterShape::NORMAL, -60000, 60000}};
    case Demod::WFM_STEREO:
        return FilterPreset{Filter{FilterShape::NORMAL, -100000, 100000},
                            Filter{FilterShape::NORMAL, -80000, 80000},
                            Filter{FilterShape::NORMAL, -60000, 60000}};
    case Demod::WFM_STEREO_OIRT:
        return FilterPreset{Filter{FilterShape::NORMAL, -100000, 100000},
                            Filter{FilterShape::NORMAL, -80000, 80000},
                            Filter{FilterShape::NORMAL, -60000, 60000}};
    default:
        return FilterPreset{Filter{FilterShape::NORMAL, 0, 0},
                            Filter{FilterShape::NORMAL, 0, 0},
                            Filter{FilterShape::NORMAL, 0, 0}};
    }
}

QFuture<void> VFOChannelModel::setDemod(Demod demod)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setDemod(demod, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<float> VFOChannelModel::getSignalPower()
{
    QPromise<float> promise;
    QFuture<float> future = promise.future();

    vfo->getSignalPwr(DEFAULT_ARG_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setNoiseBlanker(int nbid, bool on)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setNoiseBlanker(nbid, on, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setNoiseBlankerThreshold(int nbid,
                                                        float threshold)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setNoiseBlankerThreshold(nbid, threshold, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setSqlLevel(double level_db)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setSqlLevel(level_db, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setSqlAlpha(double alpha)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setSqlAlpha(alpha, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAgcOn(bool agc_on)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAgcOn(agc_on, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAgcHang(bool use_hang)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAgcHang(use_hang, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAgcThreshold(int threshold)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAgcThreshold(threshold, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAgcSlope(int slope)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAgcSlope(slope, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAgcDecay(int decay_ms)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAgcDecay(decay_ms, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAgcManualGain(int gain)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAgcManualGain(gain, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setFmMaxDev(float maxdev_hz)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setFmMaxDev(maxdev_hz, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setFmDeemph(double tau)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setFmDeemph(tau, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAmDcr(bool enabled)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAmDcr(enabled, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAmSyncDcr(bool enabled)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAmSyncDcr(enabled, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAmSyncPllBw(float pll_bw)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAmSyncPllBw(pll_bw, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::setAudioGain(float gain_db)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->setAudioGain(gain_db, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::startAudioRecording(const QString& filename)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->startAudioRecording(filename.toStdString(), DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::stopAudioRecording()
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->stopAudioRecording(DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::startUdpStreaming(const QString host, int port,
                                                 bool stereo)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->startUdpStreaming(host.toStdString(), port, stereo,
                           DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::stopUdpStreaming()
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->stopUdpStreaming(DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::startSniffer(int samplrate, int buffsize)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->startSniffer(samplrate, buffsize, DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::stopSniffer()
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->stopSniffer(DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::getSnifferData(SnifferFrame* /* frame */)
{
    // TODO
    return QtFuture::makeExceptionalFuture(
        std::make_exception_ptr(std::runtime_error("unimplemented!")));
}

QFuture<RdsData> VFOChannelModel::getRdsData()
{
    QPromise<RdsData> promise;
    QFuture<RdsData> future = promise.future();

    auto callback = [promise = std::move(promise)](violetrx::ErrorCode code,
                                                   std::string str,
                                                   int type) mutable {
        if (code == violetrx::ErrorCode::OK) {
            RdsData data;
            data.message = QString::fromStdString(str).trimmed();
            data.type = type;
            promise.start();
            promise.addResult(std::move(data));
            promise.finish();
        } else {
            promise.start();
            promise.setException(makeException(code));
            promise.finish();
        }
    };
    vfo->getRdsData(std::move(callback));

    return future;
}

QFuture<void> VFOChannelModel::startRdsDecoder()
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->startRdsDecoder(DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::stopRdsDecoder()
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->stopRdsDecoder(DEFAULT_VOID_CALLBACK);

    return future;
}

QFuture<void> VFOChannelModel::resetRdsParser(void)
{
    QPromise<void> promise;
    QFuture<void> future = promise.future();

    vfo->resetRdsParser(DEFAULT_VOID_CALLBACK);

    return future;
}

bool VFOChannelModel::supportsRds()
{
    return m_demod == Demod::WFM_MONO || m_demod == Demod::WFM_STEREO ||
           m_demod == Demod::WFM_STEREO_OIRT;
}

void VFOChannelModel::prepareToDie() { Q_EMIT removed(); }

Property<bool> VFOChannelModel::isRdsDecoderActiveProperty() const
{
    return m_rdsDecoderActive;
}
bool VFOChannelModel::isRdsDecoderActive() const { return m_rdsDecoderActive; }

Property<bool> VFOChannelModel::isRecordingAudioProperty() const
{
    return m_recordingAudio;
}
bool VFOChannelModel::isRecordingAudio() const { return m_recordingAudio; }

Property<qint64> VFOChannelModel::filterOffsetProperty() const
{
    return m_filterOffset;
}

qint64 VFOChannelModel::filterOffset() const { return m_filterOffset; }

Property<Demod> VFOChannelModel::demodProperty() const { return m_demod; }

Demod VFOChannelModel::demod() const { return m_demod; }

Property<FilterRange> VFOChannelModel::filterRangeProperty() const
{
    return m_filterRange;
}

FilterRange VFOChannelModel::filterRange() const { return m_filterRange; }

Property<FilterPreset> VFOChannelModel::filterPresetProperty() const
{
    return m_filterPreset;
}

FilterPreset VFOChannelModel::filterPreset() const { return m_filterPreset; }

Property<Filter> VFOChannelModel::filterProperty() const { return m_filter; }
Filter VFOChannelModel::filter() const { return m_filter; }

qint64 VFOChannelModel::filterBw() const
{
    return m_filter.value().high - m_filter.value().low;
}

Property<bool> VFOChannelModel::agcOnProperty() const { return m_agcOn; }
bool VFOChannelModel::agcOn() const { return m_agcOn; }

Property<bool> VFOChannelModel::agcHangProperty() const { return m_agcHang; }
bool VFOChannelModel::agcHang() const { return m_agcHang; }

Property<int> VFOChannelModel::agcThresholdProperty() const
{
    return m_agcThreshold;
}
int VFOChannelModel::agcThreshold() const { return m_agcThreshold; }

Property<int> VFOChannelModel::agcSlopeProperty() const { return m_agcSlope; }
int VFOChannelModel::agcSlope() const { return m_agcSlope; }

Property<int> VFOChannelModel::agcDecayProperty() const { return m_agcDecay; }
int VFOChannelModel::agcDecay() const { return m_agcDecay; }

Property<int> VFOChannelModel::agcManualGainProperty() const
{
    return m_agcManualGain;
}
int VFOChannelModel::agcManualGain() const { return m_agcManualGain; }

Property<float> VFOChannelModel::audioGainProperty() const
{
    return m_audioGain;
}
float VFOChannelModel::audioGain() const { return m_audioGain; }

Property<float> VFOChannelModel::fmMaxDevProperty() const { return m_fmMaxDev; }
float VFOChannelModel::fmMaxDev() const { return m_fmMaxDev; }

Property<double> VFOChannelModel::fmDeemphProperty() const
{
    return m_fmDeemph;
}
double VFOChannelModel::fmDeemph() const { return m_fmDeemph; }

Property<bool> VFOChannelModel::amDcrProperty() const { return m_amDcr; }
bool VFOChannelModel::amDcr() const { return m_amDcr; }

Property<bool> VFOChannelModel::amSyncDcrProperty() const
{
    return m_amSyncDcr;
}
bool VFOChannelModel::amSyncDcr() const { return m_amSyncDcr; }

Property<float> VFOChannelModel::amSyncPllBwProperty() const
{
    return m_amSyncPllBw;
}
float VFOChannelModel::amSyncPllBw() const { return m_amSyncPllBw; }

Property<qint64> VFOChannelModel::cwOffsetProperty() const
{
    return m_cwOffset;
}
qint64 VFOChannelModel::cwOffset() const { return m_cwOffset; }

Property<bool> VFOChannelModel::noiseBlanker1OnProperty() const
{
    return m_nb1On;
}
bool VFOChannelModel::noiseBlanker1On() const { return m_nb1On; }

Property<bool> VFOChannelModel::noiseBlanker2OnProperty() const
{
    return m_nb2On;
}
bool VFOChannelModel::noiseBlanker2On() const { return m_nb2On; }

Property<float> VFOChannelModel::noiseBlanker1ThresholdProperty() const
{
    return m_nb1Threshold;
}

float VFOChannelModel::noiseBlanker1Threshold() const { return m_nb1Threshold; }

Property<float> VFOChannelModel::noiseBlanker2ThresholdProperty() const
{
    return m_nb2Threshold;
}

float VFOChannelModel::noiseBlanker2Threshold() const { return m_nb2Threshold; }

Property<double> VFOChannelModel::sqlLevelProperty() const
{
    return m_sqlLevel;
}
double VFOChannelModel::sqlLevel() const { return m_sqlLevel; }

Property<double> VFOChannelModel::sqlAlphaProperty() const
{
    return m_sqlAlpha;
}
double VFOChannelModel::sqlAlpha() const { return m_sqlAlpha; }

Property<bool> VFOChannelModel::isUdpStreamingProperty() const
{
    return m_udpStreamingAudio;
}
bool VFOChannelModel::isUdpStreaming() const { return m_udpStreamingAudio; }

QString VFOChannelModel::demodAsString(Demod demod)
{
    switch (demod) {
    case Demod::OFF:
        return "Demod Off";
    case Demod::RAW:
        return "Raw I/Q";
    case Demod::AM:
        return "AM";
    case Demod::AM_SYNC:
        return "AM-Sync";
    case Demod::LSB:
        return "LSB";
    case Demod::USB:
        return "USB";
    case Demod::CWL:
        return "CWL";
    case Demod::CWU:
        return "CWU";
    case Demod::NFM:
        return "Narrow FM";
    case Demod::WFM_MONO:
        return "WFM (mono)";
    case Demod::WFM_STEREO:
        return "WFM (stereo)";
    case Demod::WFM_STEREO_OIRT:
        return "WFM (oirt)";
    default:
        return "Unknown";
    }
}

Demod VFOChannelModel::demodFromString(const QString& str)
{
    if (str == "Demod Off") {
        return Demod::OFF;
    } else if (str == "Raw I/Q") {
        return Demod::RAW;
    } else if (str == "AM") {
        return Demod::AM;
    } else if (str == "AM-Sync") {
        return Demod::AM_SYNC;
    } else if (str == "LSB") {
        return Demod::LSB;
    } else if (str == "USB") {
        return Demod::USB;
    } else if (str == "CW-L") {
        return Demod::CWL;
    } else if (str == "CW-U") {
        return Demod::CWU;
    } else if (str == "Narrow FM") {
        return Demod::NFM;
    } else if (str == "WFM (mono)") {
        return Demod::WFM_MONO;
    } else if (str == "WFM (stereo)") {
        return Demod::WFM_STEREO;
    } else if (str == "WFM (oirt)") {
        return Demod::WFM_STEREO_OIRT;
    } else {
        return Demod::OFF;
    }
}

void VFOChannelModel::setActive(bool active)
{
    if (active) {
        parent->setActiveVfo(this);
    } else {
        parent->setActiveVfo(nullptr);
    }
}

void VFOChannelModel::setActive_impl(bool active)
{
    m_active = active;
    Q_EMIT activeStatusChanged(active);
}

bool VFOChannelModel::isActive() const { return m_active; }
Property<bool> VFOChannelModel::isActiveProperty() const { return m_active; }

Property<QColor> VFOChannelModel::colorProperty() const { return m_color; }
QColor VFOChannelModel::color() const { return m_color; }

void VFOChannelModel::setColor(const QColor& color)
{
    m_color = color;
    Q_EMIT colorChanged(color);
}

void VFOChannelModel::setName(const QString& name)
{
    m_name = name;
    Q_EMIT nameChanged(name);
}

QString VFOChannelModel::name() const { return m_name; }

uint64_t VFOChannelModel::getId() const { return vfo->getId(); }

void VFOChannelModel::onStateChanged(const void* event_erased)
{
#define INVOKE_METHOD(func)                                                    \
    QMetaObject::invokeMethod(this, [=, this]() { func; })

    using namespace violetrx;
    const VfoEvent& e = *static_cast<const VfoEvent*>(event_erased);

    std::visit(
        Visitor{
            [&](const VfoSyncStart&) {},
            [&](const VfoSyncEnd&) {},
            [&](const DemodChanged& ev) {
                INVOKE_METHOD(onDemodChanged(ev.demod));
            },
            [&](const OffsetChanged& ev) {
                INVOKE_METHOD(onOffsetChanged(ev.offset));
            },
            [&](const CwOffsetChanged& ev) {
                INVOKE_METHOD(onCwOffsetChanged(ev.offset));
            },
            [&](const FilterChanged& ev) {
                INVOKE_METHOD(onFilterChanged(ev.filter()));
            },
            [&](const NoiseBlankerOnChanged& ev) {
                INVOKE_METHOD(onNoiseBlankerOnChanged(ev.nb_id, ev.enabled));
            },
            [&](const NoiseBlankerThresholdChanged& ev) {
                INVOKE_METHOD(
                    onNoiseBlankerThresholdChanged(ev.nb_id, ev.threshold));
            },
            [&](const SqlLevelChanged& ev) {
                INVOKE_METHOD(onSqlLevelChanged(ev.level));
            },
            [&](const SqlAlphaChanged& ev) {
                INVOKE_METHOD(onSqlLevelChanged(ev.alpha));
            },
            [&](const AgcOnChanged& ev) {
                INVOKE_METHOD(onAgcOnChanged(ev.enabled));
            },
            [&](const AgcHangChanged& ev) {
                INVOKE_METHOD(onAgcHangChanged(ev.enabled));
            },
            [&](const AgcThresholdChanged& ev) {
                INVOKE_METHOD(onAgcThresholdChanged(ev.threshold));
            },
            [&](const AgcSlopeChanged& ev) {
                INVOKE_METHOD(onAgcSlopeChanged(ev.slope));
            },
            [&](const AgcDecayChanged& ev) {
                INVOKE_METHOD(onAgcDecayChanged(ev.decay));
            },
            [&](const AgcManualGainChanged& ev) {
                INVOKE_METHOD(onAgcManualGainChanged(ev.gain));
            },
            [&](const FmMaxDevChanged& ev) {
                INVOKE_METHOD(onFmMaxdevChanged(ev.maxdev));
            },
            [&](const FmDeemphChanged& ev) {
                INVOKE_METHOD(onFmDeemphChanged(ev.tau));
            },
            [&](const AmDcrChanged& ev) {
                INVOKE_METHOD(onAmDcrChanged(ev.enabled));
            },
            [&](const AmSyncDcrChanged& ev) {
                INVOKE_METHOD(onAmSyncDcrChanged(ev.enabled));
            },
            [&](const AmSyncPllBwChanged& ev) {
                INVOKE_METHOD(onAmSyncPllBwChanged(ev.bw));
            },
            [&](const RecordingStarted& ev) {
                INVOKE_METHOD(onRecordingStarted(ev.path));
            },
            [&](const RecordingStopped&) {
                INVOKE_METHOD(onRecordingStopped());
            },
            [&](const SnifferStarted& ev) {
                INVOKE_METHOD(onSnifferStarted(ev.sample_rate, ev.size));
            },
            [&](const SnifferStopped&) { INVOKE_METHOD(onSnifferStopped()); },
            [&](const UdpStreamingStarted& ev) {
                INVOKE_METHOD(
                    onUdpStreamingStarted(ev.host, ev.port, ev.stereo));
            },
            [&](const UdpStreamingStopped&) {
                INVOKE_METHOD(onUdpStreamingStopped());
            },
            [&](const RdsDecoderStarted&) {
                INVOKE_METHOD(onRdsDecoderStarted());
            },
            [&](const RdsDecoderStopped&) {
                INVOKE_METHOD(onRdsDecoderStopped());
            },
            [&](const RdsParserReset&) { INVOKE_METHOD(onRdsParserReset()); },
            [&](const AudioGainChanged& ev) {
                INVOKE_METHOD(onAudioGainChanged(ev.gain));
            },
            [&](const VfoRemoved&) { INVOKE_METHOD(onRemoved()); },
        },
        e);

#undef INVOKE_METHOD
}

void VFOChannelModel::onDemodChanged(Demod demod)
{
    m_demod = demod;
    m_filterRange = vfo->getFilterRange(demod);
    m_filterPreset = getFilterPreset(demod);

    Q_EMIT demodChanged(m_demod);
    Q_EMIT filterRangeChanged(m_filterRange);
    Q_EMIT filterPresetChanged(m_filterPreset);
}

void VFOChannelModel::onOffsetChanged(int64_t offset)
{
    m_filterOffset = offset;
    Q_EMIT offsetChanged(offset);
}
void VFOChannelModel::onCwOffsetChanged(int64_t offset)
{
    m_cwOffset = offset;
    Q_EMIT cwOffsetChanged(offset);
}
void VFOChannelModel::onFilterChanged(Filter filter)
{
    m_filter = filter;
    Q_EMIT filterChanged(filter.low, filter.high, filter.shape);
}
void VFOChannelModel::onNoiseBlankerOnChanged(int id, bool on)
{
    if (id == 1) {
        m_nb1On = on;
    } else if (id == 2) {
        m_nb2On = on;
    }

    Q_EMIT noiseBlankerOnChanged(id, on);
}
void VFOChannelModel::onNoiseBlankerThresholdChanged(int id, float threshold)
{
    if (id == 1) {
        m_nb1Threshold = threshold;
    } else if (id == 2) {
        m_nb2Threshold = threshold;
    }

    Q_EMIT noiseBlankerThresholdChanged(id, threshold);
}
void VFOChannelModel::onSqlLevelChanged(double level)
{
    m_sqlLevel = level;
    Q_EMIT sqlLevelChanged(level);
}
void VFOChannelModel::onSqlAlphaChanged(double alpha)
{
    m_sqlAlpha = alpha;
    Q_EMIT sqlAlphaChanged(alpha);
}
void VFOChannelModel::onAgcOnChanged(bool enable)
{
    m_agcOn = enable;
    Q_EMIT agcOnChanged(enable);
}
void VFOChannelModel::onAgcHangChanged(bool enable)
{
    m_agcHang = enable;
    Q_EMIT agcHangChanged(enable);
}
void VFOChannelModel::onAgcThresholdChanged(int threshold)
{
    m_agcThreshold = threshold;
    Q_EMIT agcThresholdChanged(threshold);
}
void VFOChannelModel::onAgcSlopeChanged(int slope)
{
    m_agcSlope = slope;
    Q_EMIT agcSlopeChanged(slope);
}
void VFOChannelModel::onAgcDecayChanged(int decay)
{
    m_agcDecay = decay;
    Q_EMIT agcDecayChanged(decay);
}
void VFOChannelModel::onAgcManualGainChanged(int gain)
{
    m_agcManualGain = gain;
    Q_EMIT agcManualGainChanged(gain);
}
void VFOChannelModel::onFmMaxdevChanged(float maxdev)
{
    m_fmMaxDev = maxdev;
    Q_EMIT fmMaxDevChanged(maxdev);
}
void VFOChannelModel::onFmDeemphChanged(double tau)
{
    m_fmDeemph = tau;
    Q_EMIT fmDeemphChanged(tau);
}
void VFOChannelModel::onAmDcrChanged(bool enable)
{
    m_amDcr = enable;
    Q_EMIT amDcrChanged(enable);
}
void VFOChannelModel::onAmSyncDcrChanged(bool enable)
{
    m_amSyncDcr = enable;
    Q_EMIT amSyncDcrChanged(enable);
}
void VFOChannelModel::onAmSyncPllBwChanged(float bw)
{
    m_amSyncPllBw = bw;
    Q_EMIT amSyncPllBwChanged(bw);
}
void VFOChannelModel::onAudioGainChanged(float gain)
{
    m_audioGain = gain;
    Q_EMIT audioGainChanged(gain);
}
void VFOChannelModel::onRecordingStarted(std::string filename)
{
    m_recordingAudio = true;
    Q_EMIT audioRecordingStarted(QString::fromStdString(filename));
}
void VFOChannelModel::onRecordingStopped()
{
    m_recordingAudio = false;
    Q_EMIT audioRecordingStopped();
}
void VFOChannelModel::onUdpStreamingStarted(std::string host, int port,
                                            bool stereo)
{
    m_udpStreamingAudio = true;
    Q_EMIT udpStreamingStarted(QString::fromStdString(host), port, stereo);
}
void VFOChannelModel::onUdpStreamingStopped()
{
    m_udpStreamingAudio = false;
    Q_EMIT udpStreamingStopped();
}
void VFOChannelModel::onSnifferStarted(int samplerate, int buffsize)
{
    Q_EMIT snifferStarted(samplerate, buffsize);
}
void VFOChannelModel::onSnifferStopped() { Q_EMIT snifferStopped(); }
void VFOChannelModel::onRdsDecoderStarted()
{
    m_rdsDecoderActive = true;
    Q_EMIT rdsDecoderStarted();
}
void VFOChannelModel::onRdsDecoderStopped()
{
    m_rdsDecoderActive = false;
    Q_EMIT rdsDecoderStopped();
}
void VFOChannelModel::onRdsParserReset() { Q_EMIT rdsParserReset(); }
void VFOChannelModel::onRemoved()
{
    // do nothing, ReceiverModel will handle it by calling "prepareToDie!"
}

VFOChannelModel::~VFOChannelModel() {}
