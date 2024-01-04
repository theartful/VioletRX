#ifndef QTCLIENT_AUDIO_PLAYER_H
#define QTCLIENT_AUDIO_PLAYER_H

#include <QObject>
#include <QUrl>

#include <array>

#include <portaudio.h>
#include <qtypes.h>
#include <sndfile.h>

PaStreamCallback audioPlayerPortaudioCallback;

class AudioPlayer : public QObject
{
    Q_OBJECT

    friend PaStreamCallback audioPlayerPortaudioCallback;

public:
    enum PlaybackState { PlayingState, StoppedState };
    enum MediaStatus { HasMedia, NoMedia, InvalidMedia };

public:
    AudioPlayer(QObject* parent);
    ~AudioPlayer();

    void play();
    void stop();

    void setPosition(quint32 position);
    void setSource(const QUrl& source);

    MediaStatus mediaStatus() const;
    PlaybackState playbackState() const;

    qint32 duration();
    qint32 position();
    bool isSeekable();

Q_SIGNALS:
    void sourceChanged(const QUrl& media);
    void mediaStatusChanged(MediaStatus newStatus);
    void playbackStateChanged(PlaybackState newState);
    void durationChanged(quint32 duration);
    void positionChanged(quint32 position);
    void playbackFinished();

private:
    void resume();
    void start();
    void closeFile();
    void stopStream();
    void startStream();
    void closeStream();
    void incrementPosition(quint32);
    void setPositionInFrames(quint32);
    void audioFinished();
    void assertSourceIsAttached();
    void reset();

    void setPlaybackState(PlaybackState);
    void setMediaStatus(MediaStatus);
    void seekPositionInFrames(quint32);

    size_t read(float* buffer, size_t size);

    int paStreamCallback(const void* inputBuffer, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags);

private:
    SNDFILE* file;
    SF_INFO fileInfo;

    PaStream* stream;
    PaStreamParameters streamParameters;

    PlaybackState playbackState_;
    MediaStatus mediaStatus_;

    quint32 positionInFrames;

    // caching stuff
    struct SeekRequest {
        bool shouldSeek;
        quint32 pos;
    };

    std::atomic<SeekRequest> seekRequest;
};

#endif // QTCLIENT_AUDIO_PLAYER_H
