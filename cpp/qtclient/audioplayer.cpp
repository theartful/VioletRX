#include "audioplayer.h"

#include <algorithm>
#include <atomic>
#include <exception>
#include <portaudio.h>
#include <sndfile.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

int audioPlayerPortaudioCallback(const void* inputBuffer, void* outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* timeInfo,
                                 PaStreamCallbackFlags statusFlags, void* arg)
{
    return static_cast<AudioPlayer*>(arg)->paStreamCallback(
        inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

AudioPlayer::AudioPlayer(QObject* parent) :
    QObject{parent},
    file{nullptr},
    stream{nullptr},
    streamParameters{},
    playbackState_{StoppedState},
    mediaStatus_{NoMedia},
    positionInFrames{0}
{
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error(fmt::format("Initialize failed: {}", err));
    }

    // TODO: customize output device
    PaDeviceIndex device = 0;
    device = Pa_GetDefaultOutputDevice();
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(device);
    spdlog::info(
        "AudioPlayer: {:s} is the chosen device using {:s} as the host",
        deviceInfo->name, Pa_GetHostApiInfo(deviceInfo->hostApi)->name);

    streamParameters.device = device;

    // for simplicity's sake we always use float32 samples
    streamParameters.sampleFormat = paFloat32;
    streamParameters.hostApiSpecificStreamInfo = NULL;

    seekRequest.store(SeekRequest{false, 0}, std::memory_order_relaxed);
}

AudioPlayer::~AudioPlayer()
{
    spdlog::debug("~AudioPlayer");

    try {
        stopStream();
        closeStream();
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
    }

    try {
        closeFile();
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
    }

    int err = Pa_Terminate();
    if (err) {
        spdlog::error("{}", err);
    }
}

void AudioPlayer::startStream()
{

    PaError err = Pa_StartStream(stream);
    if (err != paNoError) {
        throw std::runtime_error(
            fmt::format("StartStream failed: {}", Pa_GetErrorText(err)));
    }
}

void AudioPlayer::stopStream()
{
    if (!stream)
        return;

    PaError err;
    err = Pa_StopStream(stream);
    if (err) {
        throw std::runtime_error(fmt::format("{}", Pa_GetErrorText(err)));
    }
}

void AudioPlayer::closeStream()
{
    if (!stream)
        return;

    PaError err = Pa_CloseStream(stream);
    if (err) {
        throw std::runtime_error(fmt::format("{}", Pa_GetErrorText(err)));
    }

    stream = nullptr;
}

void AudioPlayer::play()
{
    if (mediaStatus() == NoMedia) {
        throw std::runtime_error(fmt::format("No source is attached"));
    } else if (mediaStatus() == InvalidMedia) {
        throw std::runtime_error(fmt::format("Invalid source"));
    }

    if (playbackState() == StoppedState) {
        start();
    }
}

void AudioPlayer::closeFile()
{
    if (!file) {
        return;
    }

    int err = sf_close(file);

    if (err) {
        throw std::runtime_error(fmt::format("{}", sf_error_number(err)));
    }

    file = nullptr;
}

void AudioPlayer::start()
{
    assertSourceIsAttached();

    // TODO: assert assumptions
    // the assumption here is that we have a stopped opened stream

    startStream();
    setPlaybackState(PlayingState);
}

void AudioPlayer::stop()
{
    if (playbackState() == PlayingState) {
        stopStream();
        setPlaybackState(StoppedState);
    }
}

void AudioPlayer::assertSourceIsAttached()
{
    if (mediaStatus() == NoMedia) {
        throw std::runtime_error(fmt::format("No source is attached"));
    } else if (mediaStatus() == InvalidMedia) {
        throw std::runtime_error(fmt::format("Invalid source"));
    }
}

void AudioPlayer::setPosition(quint32 position)
{
    assertSourceIsAttached();

    if (duration() == 0) {
        seekPositionInFrames(0);
    } else {
        seekPositionInFrames(
            std::min((quint32)(fileInfo.frames * position / duration()),
                     (quint32)fileInfo.frames));
    }
}

void AudioPlayer::resume() { start(); }

void AudioPlayer::reset()
{
    PlaybackState oldPlaybackState = playbackState();

    setMediaStatus(NoMedia);
    setPlaybackState(StoppedState);
    seekPositionInFrames(0);
    durationChanged(0);
    seekRequest.store(SeekRequest{false, 0}, std::memory_order_relaxed);

    if (oldPlaybackState == PlayingState) {
        stopStream();
    }

    closeStream();
    closeFile();
}

void AudioPlayer::setSource(const QUrl& source)
{
    reset();

    if (!source.isLocalFile()) {
        throw std::runtime_error("AudioPlayer can only play local files!");
    }

    std::string filePath = source.toLocalFile().toStdString();
    file = sf_open(filePath.c_str(), SFM_READ, &fileInfo);

    if (!file) {
        setMediaStatus(InvalidMedia);
        return;
    }

    streamParameters.channelCount = fileInfo.channels;

    PaError err =
        Pa_OpenStream(&stream,
                      NULL, // No input
                      &streamParameters, fileInfo.samplerate, 1024, paNoFlag,
                      &audioPlayerPortaudioCallback, (void*)this);

    if (err != paNoError) {
        throw std::runtime_error(fmt::format("OpenStream failed: {}", err));
    }

    setMediaStatus(HasMedia);
    setPlaybackState(StoppedState);
    seekPositionInFrames(0);
    Q_EMIT durationChanged(duration());
}

qint32 AudioPlayer::duration()
{
    if (file) {
        if (fileInfo.samplerate == 0)
            return 0;
        else
            return fileInfo.frames * 1000 / fileInfo.samplerate;
    } else {
        return 0;
    }
}

qint32 AudioPlayer::position()
{
    if (file) {
        qint32 numerator = positionInFrames * 1000;
        qint32 denominator = fileInfo.samplerate;

        if (denominator == 0)
            return 0;
        else
            return numerator / denominator;
    } else {
        return 0;
    }
}

bool AudioPlayer::isSeekable()
{
    if (file)
        return fileInfo.seekable;
    else
        return 0;
}

AudioPlayer::MediaStatus AudioPlayer::mediaStatus() const
{
    return mediaStatus_;
}
AudioPlayer::PlaybackState AudioPlayer::playbackState() const
{
    return playbackState_;
}

void AudioPlayer::setPlaybackState(PlaybackState s)
{
    if (playbackState_ != s) {
        playbackState_ = s;
        Q_EMIT playbackStateChanged(s);
    }
}

void AudioPlayer::setMediaStatus(MediaStatus s)
{
    if (mediaStatus_ != s) {
        mediaStatus_ = s;
        Q_EMIT mediaStatusChanged(s);
    }
}

void AudioPlayer::seekPositionInFrames(quint32 p)
{
    if (playbackState() == PlaybackState::PlayingState) {
        seekRequest.store(SeekRequest{true, p}, std::memory_order_relaxed);
    } else {
        if (file) {
            sf_count_t pos = sf_seek(file, p, SEEK_SET);
            setPositionInFrames(pos);
        }
    }
}

void AudioPlayer::setPositionInFrames(quint32 p)
{
    positionInFrames = p;
    Q_EMIT positionChanged(position());
}

void AudioPlayer::incrementPosition(quint32 inc)
{
    positionInFrames += inc;

    Q_EMIT positionChanged(position());
}

void AudioPlayer::audioFinished()
{
    stopStream();
    setPlaybackState(StoppedState);
    Q_EMIT playbackFinished();
}

int AudioPlayer::paStreamCallback(
    const void* /* inputBuffer */, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* /* timeInfo */,
    PaStreamCallbackFlags /* statusFlags */)
{
    // WARNING: this function is assumed to run in a different thread
    // don't do anything that is not thread safe!!

    SeekRequest empty = {false, 0};
    SeekRequest request = seekRequest.load(std::memory_order_relaxed);

    while (!std::atomic_compare_exchange_weak(&seekRequest, &request, empty))
        ;

    if (request.shouldSeek) {
        sf_count_t pos = sf_seek(file, request.pos, SEEK_SET);
        QMetaObject::invokeMethod(
            this, std::bind(&AudioPlayer::setPositionInFrames, this, pos));
    }

    sf_count_t readFrames =
        sf_readf_float(file, (float*)outputBuffer, framesPerBuffer);

    QMetaObject::invokeMethod(
        this, std::bind(&AudioPlayer::incrementPosition, this, readFrames));

    if (static_cast<unsigned long>(readFrames) < framesPerBuffer) {
        int readBytes =
            readFrames * sizeof(float) * streamParameters.channelCount;
        int remainingBytes =
            framesPerBuffer * sizeof(float) * streamParameters.channelCount -
            readBytes;

        memset(static_cast<char*>(outputBuffer) + readBytes, 0, remainingBytes);

        QMetaObject::invokeMethod(this, &AudioPlayer::audioFinished);

        // FIXME: here we punish underruns by stopping the stream!
        return paComplete;
    } else {
        return paContinue;
    }
}
