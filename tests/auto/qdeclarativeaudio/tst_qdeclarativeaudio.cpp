/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

//TESTED_COMPONENT=plugins/declarative/multimedia

#include <QtTest/QtTest>

#include "qdeclarativeaudio_p.h"
#include "qdeclarativemediametadata_p.h"

#include <qmediaplayercontrol.h>
#include <qmediaservice.h>
#include <qmediaserviceprovider.h>
#include <qmetadatareadercontrol.h>

#include <QtGui/qapplication.h>

class tst_QDeclarativeAudio : public QObject
{
    Q_OBJECT
public slots:
    void initTestCase();

private slots:
    void nullPlayerControl();
    void nullMetaDataControl();
    void nullService();

    void source();
    void autoLoad();
    void playing();
    void paused();
    void duration();
    void position();
    void volume();
    void muted();
    void bufferProgress();
    void seekable();
    void playbackRate();
    void status();
    void metaData_data();
    void metaData();
    void error();
    void loops();
};

Q_DECLARE_METATYPE(QtMultimediaKit::MetaData);
Q_DECLARE_METATYPE(QDeclarativeAudio::Error);

class QtTestMediaPlayerControl : public QMediaPlayerControl
{
    Q_OBJECT
public:
    QtTestMediaPlayerControl(QObject *parent = 0)
        : QMediaPlayerControl(parent)
        , m_state(QMediaPlayer::StoppedState)
        , m_mediaStatus(QMediaPlayer::NoMedia)
        , m_duration(0)
        , m_position(0)
        , m_playbackRate(1.0)
        , m_volume(50)
        , m_bufferStatus(0)
        , m_muted(false)
        , m_audioAvailable(false)
        , m_videoAvailable(false)
        , m_seekable(false)
    {
    }

    QMediaPlayer::State state() const { return m_state; }
    void updateState(QMediaPlayer::State state) { emit stateChanged(m_state = state); }

    QMediaPlayer::MediaStatus mediaStatus() const { return m_mediaStatus; }
    void updateMediaStatus(QMediaPlayer::MediaStatus status) {
        emit mediaStatusChanged(m_mediaStatus = status); }
    void updateMediaStatus(QMediaPlayer::MediaStatus status, QMediaPlayer::State state)
    {
        m_mediaStatus = status;
        m_state = state;

        emit mediaStatusChanged(m_mediaStatus);
        emit stateChanged(m_state);
    }

    qint64 duration() const { return m_duration; }
    void setDuration(qint64 duration) { emit durationChanged(m_duration = duration); }

    qint64 position() const { return m_position; }
    void setPosition(qint64 position) { emit positionChanged(m_position = position); }

    int volume() const { return m_volume; }
    void setVolume(int volume) { emit volumeChanged(m_volume = volume); }

    bool isMuted() const { return m_muted; }
    void setMuted(bool muted) { emit mutedChanged(m_muted = muted); }

    int bufferStatus() const { return m_bufferStatus; }
    void setBufferStatus(int status) { emit bufferStatusChanged(m_bufferStatus = status); }

    bool isAudioAvailable() const { return m_audioAvailable; }
    void setAudioAvailable(bool available) {
        emit audioAvailableChanged(m_audioAvailable = available); }
    bool isVideoAvailable() const { return m_videoAvailable; }
    void setVideoAvailable(bool available) {
        emit videoAvailableChanged(m_videoAvailable = available); }

    bool isSeekable() const { return m_seekable; }
    void setSeekable(bool seekable) { emit seekableChanged(m_seekable = seekable); }

    QMediaTimeRange availablePlaybackRanges() const { return QMediaTimeRange(); }

    qreal playbackRate() const { return m_playbackRate; }
    void setPlaybackRate(qreal rate) { emit playbackRateChanged(m_playbackRate = rate); }

    QMediaContent media() const { return m_media; }
    const QIODevice *mediaStream() const { return 0; }
    void setMedia(const QMediaContent &media, QIODevice *)
    {
        m_media = media;

        m_mediaStatus = m_media.isNull()
                ? QMediaPlayer::NoMedia
                : QMediaPlayer::LoadingMedia;

        emit mediaChanged(m_media);
        emit mediaStatusChanged(m_mediaStatus);
    }

    void play()
    {
        m_state = QMediaPlayer::PlayingState;
        if (m_mediaStatus == QMediaPlayer::EndOfMedia)
            updateMediaStatus(QMediaPlayer::LoadedMedia);
        emit stateChanged(m_state);
    }
    void pause() { emit stateChanged(m_state = QMediaPlayer::PausedState); }
    void stop() { emit stateChanged(m_state = QMediaPlayer::StoppedState); }

    void emitError(QMediaPlayer::Error err, const QString &errorString) {
        emit error(err, errorString); }

private:
    QMediaPlayer::State m_state;
    QMediaPlayer::MediaStatus m_mediaStatus;
    qint64 m_duration;
    qint64 m_position;
    qreal m_playbackRate;
    int m_volume;
    int m_bufferStatus;
    bool m_muted;
    bool m_audioAvailable;
    bool m_videoAvailable;
    bool m_seekable;
    QMediaContent m_media;
};

class QtTestMetaDataControl : public QMetaDataReaderControl
{
    Q_OBJECT
public:
    QtTestMetaDataControl(QObject *parent = 0)
        : QMetaDataReaderControl(parent)
    {
    }

    bool isMetaDataAvailable() const { return true; }

    QVariant metaData(QtMultimediaKit::MetaData key) const { return m_metaData.value(key); }
    void setMetaData(QtMultimediaKit::MetaData key, const QVariant &value) {
        m_metaData.insert(key, value); emit metaDataChanged(); }

    QList<QtMultimediaKit::MetaData> availableMetaData() const { return m_metaData.keys(); }

    QVariant extendedMetaData(const QString &) const { return QVariant(); }
    QStringList availableExtendedMetaData() const { return QStringList(); }

private:
    QMap<QtMultimediaKit::MetaData, QVariant> m_metaData;
};

class QtTestMediaService : public QMediaService
{
    Q_OBJECT
public:
    QtTestMediaService(
            QtTestMediaPlayerControl *playerControl,
            QtTestMetaDataControl *metaDataControl,
            QObject *parent)
        : QMediaService(parent)
        , playerControl(playerControl)
        , metaDataControl(metaDataControl)
    {
    }

    QMediaControl *requestControl(const char *name)
    {
        if (qstrcmp(name, QMediaPlayerControl_iid) == 0)
            return playerControl;
        else if (qstrcmp(name, QMetaDataReaderControl_iid) == 0)
            return metaDataControl;
        else
            return 0;
    }

    void releaseControl(QMediaControl *) {}

    QtTestMediaPlayerControl *playerControl;
    QtTestMetaDataControl *metaDataControl;
};

class QtTestMediaServiceProvider : public QMediaServiceProvider
{
    Q_OBJECT
public:
    QtTestMediaServiceProvider()
        : service(new QtTestMediaService(
                new QtTestMediaPlayerControl(this), new QtTestMetaDataControl(this), this))
    {
        setDefaultServiceProvider(this);
    }

    QtTestMediaServiceProvider(QtTestMediaService *service)
        : service(service)
    {
        setDefaultServiceProvider(this);
    }

    QtTestMediaServiceProvider(
            QtTestMediaPlayerControl *playerControl, QtTestMetaDataControl *metaDataControl)
        : service(new QtTestMediaService(playerControl, metaDataControl, this))
    {
        setDefaultServiceProvider(this);
    }

    ~QtTestMediaServiceProvider()
    {
        setDefaultServiceProvider(0);
    }

    QMediaService *requestService(
            const QByteArray &type,
            const QMediaServiceProviderHint & = QMediaServiceProviderHint())
    {
        requestedService = type;

        return service;
    }

    void releaseService(QMediaService *) {}

    inline QtTestMediaPlayerControl *playerControl() { return service->playerControl; }
    inline QtTestMetaDataControl *metaDataControl() { return service->metaDataControl; }

    QtTestMediaService *service;
    QByteArray requestedService;
};

void tst_QDeclarativeAudio::initTestCase()
{
    qRegisterMetaType<QDeclarativeAudio::Error>();
}

void tst_QDeclarativeAudio::nullPlayerControl()
{
    QtTestMetaDataControl metaDataControl;
    QtTestMediaServiceProvider provider(0, &metaDataControl);

    QDeclarativeAudio audio;
    audio.classBegin();

    QCOMPARE(audio.source(), QUrl());
    audio.setSource(QUrl("http://example.com"));
    QCOMPARE(audio.source(), QUrl("http://example.com"));

    QCOMPARE(audio.isPlaying(), false);
    audio.setPlaying(true);
    QCOMPARE(audio.isPlaying(), true);
    audio.setPlaying(false);
    audio.play();
    QCOMPARE(audio.isPlaying(), false);

    QCOMPARE(audio.isPaused(), false);
    audio.pause();
    QCOMPARE(audio.isPaused(), false);
    audio.setPaused(true);
    QCOMPARE(audio.isPaused(), true);

    QCOMPARE(audio.duration(), 0);

    QCOMPARE(audio.position(), 0);
    audio.setPosition(10000);
    QCOMPARE(audio.position(), 10000);

    QCOMPARE(audio.volume(), qreal(1.0));
    audio.setVolume(0.5);
    QCOMPARE(audio.volume(), qreal(0.5));

    QCOMPARE(audio.isMuted(), false);
    audio.setMuted(true);
    QCOMPARE(audio.isMuted(), true);

    QCOMPARE(audio.bufferProgress(), qreal(0));

    QCOMPARE(audio.isSeekable(), false);

    QCOMPARE(audio.playbackRate(), qreal(1.0));

    QCOMPARE(audio.status(), QDeclarativeAudio::NoMedia);

    QCOMPARE(audio.error(), QDeclarativeAudio::ServiceMissing);
}

void tst_QDeclarativeAudio::nullMetaDataControl()
{
    QtTestMediaPlayerControl playerControl;
    QtTestMediaServiceProvider provider(&playerControl, 0);

    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QVERIFY(audio.metaData());
}

void tst_QDeclarativeAudio::nullService()
{
    QtTestMediaServiceProvider provider(0);

    QDeclarativeAudio audio;
    audio.classBegin();

    QCOMPARE(audio.source(), QUrl());
    audio.setSource(QUrl("http://example.com"));
    QCOMPARE(audio.source(), QUrl("http://example.com"));

    QCOMPARE(audio.isPlaying(), false);
    audio.setPlaying(true);
    QCOMPARE(audio.isPlaying(), true);
    audio.setPlaying(false);
    audio.play();
    QCOMPARE(audio.isPlaying(), false);

    QCOMPARE(audio.isPaused(), false);
    audio.pause();
    QCOMPARE(audio.isPaused(), false);
    audio.setPaused(true);
    QCOMPARE(audio.isPaused(), true);

    QCOMPARE(audio.duration(), 0);

    QCOMPARE(audio.position(), 0);
    audio.setPosition(10000);
    QCOMPARE(audio.position(), 10000);

    QCOMPARE(audio.volume(), qreal(1.0));
    audio.setVolume(0.5);
    QCOMPARE(audio.volume(), qreal(0.5));

    QCOMPARE(audio.isMuted(), false);
    audio.setMuted(true);
    QCOMPARE(audio.isMuted(), true);

    QCOMPARE(audio.bufferProgress(), qreal(0));

    QCOMPARE(audio.isSeekable(), false);

    QCOMPARE(audio.playbackRate(), qreal(1.0));

    QCOMPARE(audio.status(), QDeclarativeAudio::NoMedia);

    QCOMPARE(audio.error(), QDeclarativeAudio::ServiceMissing);

    QVERIFY(audio.metaData());
}

void tst_QDeclarativeAudio::source()
{
    const QUrl url1("http://example.com");
    const QUrl url2("file:///local/path");
    const QUrl url3;

    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(&audio, SIGNAL(sourceChanged()));

    audio.setSource(url1);
    QCOMPARE(audio.source(), url1);
    QCOMPARE(provider.playerControl()->media().canonicalUrl(), url1);
    QCOMPARE(spy.count(), 1);

    audio.setSource(url2);
    QCOMPARE(audio.source(), url2);
    QCOMPARE(provider.playerControl()->media().canonicalUrl(), url2);
    QCOMPARE(spy.count(), 2);

    audio.setSource(url3);
    QCOMPARE(audio.source(), url3);
    QCOMPARE(provider.playerControl()->media().canonicalUrl(), url3);
    QCOMPARE(spy.count(), 3);
}

void tst_QDeclarativeAudio::autoLoad()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(&audio, SIGNAL(autoLoadChanged()));

    QCOMPARE(audio.isAutoLoad(), true);

    audio.setAutoLoad(false);
    QCOMPARE(audio.isAutoLoad(), false);
    QCOMPARE(spy.count(), 1);

    audio.setSource(QUrl("http://example.com"));
    QCOMPARE(audio.source(), QUrl("http://example.com"));
    audio.play();
    QCOMPARE(audio.isPlaying(), true);
    audio.stop();

    audio.setAutoLoad(true);
    audio.setSource(QUrl("http://example.com"));
    audio.setPaused(true);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(audio.isPaused(), true);
}

void tst_QDeclarativeAudio::playing()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();

    QSignalSpy playingChangedSpy(&audio, SIGNAL(playingChanged()));
    QSignalSpy startedSpy(&audio, SIGNAL(started()));
    QSignalSpy stoppedSpy(&audio, SIGNAL(stopped()));

    int playingChanged = 0;
    int started = 0;
    int stopped = 0;

    audio.componentComplete();
    audio.setSource(QUrl("http://example.com"));

    QCOMPARE(audio.isPlaying(), false);

    // setPlaying(true) when stopped.
    audio.setPlaying(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(startedSpy.count(),        ++started);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPlaying(false) when playing.
    audio.setPlaying(false);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(stoppedSpy.count(),        ++stopped);

    // play() when stopped.
    audio.play();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(startedSpy.count(),        ++started);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // stop() when playing.
    audio.stop();
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(stoppedSpy.count(),        ++stopped);

    // stop() when stopped.
    audio.stop();
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPlaying(false) when stopped.
    audio.setPlaying(false);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(stoppedSpy.count(),          stopped);

    audio.setPlaying(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(startedSpy.count(),        ++started);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPlaying(true) when playing.
    audio.setPlaying(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // play() when playing.
    audio.play();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(stoppedSpy.count(),          stopped);
}

void tst_QDeclarativeAudio::paused()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();

    QSignalSpy playingChangedSpy(&audio, SIGNAL(playingChanged()));
    QSignalSpy pausedChangedSpy(&audio, SIGNAL(pausedChanged()));
    QSignalSpy startedSpy(&audio, SIGNAL(started()));
    QSignalSpy pausedSpy(&audio, SIGNAL(paused()));
    QSignalSpy resumedSpy(&audio, SIGNAL(resumed()));
    QSignalSpy stoppedSpy(&audio, SIGNAL(stopped()));

    int playingChanged = 0;
    int pausedChanged = 0;
    int started = 0;
    int paused = 0;
    int resumed = 0;
    int stopped = 0;

    audio.componentComplete();
    audio.setSource(QUrl("http://example.com"));

    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), false);

    // setPlaying(true) when stopped.
    audio.setPlaying(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),        ++started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPaused(true) when playing.
    audio.setPaused(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PausedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),         ++paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPaused(true) when paused.
    audio.setPaused(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PausedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // pause() when paused.
    audio.pause();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PausedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPaused(false) when paused.
    audio.setPaused(false);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),        ++resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPaused(false) when playing.
    audio.setPaused(false);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // pause() when playing.
    audio.pause();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PausedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),         ++paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPlaying(false) when paused.
    audio.setPlaying(false);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),        ++stopped);

    // setPaused(true) when stopped and paused.
    audio.setPaused(true);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPaused(false) when stopped and paused.
    audio.setPaused(false);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPaused(true) when stopped.
    audio.setPaused(true);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPlaying(true) when stopped and paused.
    audio.setPlaying(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PausedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),        ++started);
    QCOMPARE(pausedSpy.count(),         ++paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // play() when paused.
    audio.play();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),        ++resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPaused(true) when playing.
    audio.setPaused(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PausedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),         ++paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // stop() when paused.
    audio.stop();
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),        ++stopped);

    // setPaused(true) when stopped.
    audio.setPaused(true);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // stop() when stopped and paused.
    audio.stop();
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // pause() when stopped.
    audio.pause();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PausedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),        ++started);
    QCOMPARE(pausedSpy.count(),         ++paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPlaying(false) when paused.
    audio.setPlaying(false);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),        ++stopped);

    // pause() when stopped and paused.
    audio.pause();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PausedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),        ++started);
    QCOMPARE(pausedSpy.count(),         ++paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPlaying(false) when paused.
    audio.setPlaying(false);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(audio.isPaused(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(pausedChangedSpy.count(),    pausedChanged);
    QCOMPARE(startedSpy.count(),          started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),        ++stopped);

    // play() when stopped and paused.
    audio.play();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(audio.isPaused(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(pausedChangedSpy.count(),  ++pausedChanged);
    QCOMPARE(startedSpy.count(),        ++started);
    QCOMPARE(pausedSpy.count(),           paused);
    QCOMPARE(resumedSpy.count(),          resumed);
    QCOMPARE(stoppedSpy.count(),          stopped);
}

void tst_QDeclarativeAudio::duration()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(&audio, SIGNAL(durationChanged()));

    QCOMPARE(audio.duration(), 0);

    provider.playerControl()->setDuration(4040);
    QCOMPARE(audio.duration(), 4040);
    QCOMPARE(spy.count(), 1);

    provider.playerControl()->setDuration(-129);
    QCOMPARE(audio.duration(), -129);
    QCOMPARE(spy.count(), 2);

    provider.playerControl()->setDuration(0);
    QCOMPARE(audio.duration(), 0);
    QCOMPARE(spy.count(), 3);

    // Unnecessary duration changed signals aren't filtered.
    provider.playerControl()->setDuration(0);
    QCOMPARE(audio.duration(), 0);
    QCOMPARE(spy.count(), 4);
}

void tst_QDeclarativeAudio::position()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(&audio, SIGNAL(positionChanged()));

    QCOMPARE(audio.position(), 0);

    // QDeclarativeAudio won't bound set positions to the duration.  A media service may though.
    QCOMPARE(audio.duration(), 0);

    audio.setPosition(450);
    QCOMPARE(audio.position(), 450);
    QCOMPARE(provider.playerControl()->position(), qint64(450));
    QCOMPARE(spy.count(), 1);

    audio.setPosition(-5403);
    QCOMPARE(audio.position(), -5403);
    QCOMPARE(provider.playerControl()->position(), qint64(-5403));
    QCOMPARE(spy.count(), 2);

    audio.setPosition(-5403);
    QCOMPARE(audio.position(), -5403);
    QCOMPARE(provider.playerControl()->position(), qint64(-5403));
    QCOMPARE(spy.count(), 2);

    // Check the signal change signal is emitted if the change originates from the media service.
    provider.playerControl()->setPosition(0);
    QCOMPARE(audio.position(), 0);
    QCOMPARE(spy.count(), 3);

    connect(&audio, SIGNAL(positionChanged()), &QTestEventLoop::instance(), SLOT(exitLoop()));

    provider.playerControl()->updateState(QMediaPlayer::PlayingState);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(spy.count() > 3 && spy.count() < 6); // 4 or 5

    provider.playerControl()->updateState(QMediaPlayer::PausedState);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(spy.count() < 6);
}

void tst_QDeclarativeAudio::volume()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(&audio, SIGNAL(volumeChanged()));

    QCOMPARE(audio.volume(), qreal(1.0));

    audio.setVolume(0.7);
    QCOMPARE(audio.volume(), qreal(0.7));
    QCOMPARE(provider.playerControl()->volume(), 70);
    QCOMPARE(spy.count(), 1);

    audio.setVolume(0.7);
    QCOMPARE(audio.volume(), qreal(0.7));
    QCOMPARE(provider.playerControl()->volume(), 70);
    QCOMPARE(spy.count(), 1);

    provider.playerControl()->setVolume(30);
    QCOMPARE(audio.volume(), qreal(0.3));
    QCOMPARE(spy.count(), 2);
}

void tst_QDeclarativeAudio::muted()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(&audio, SIGNAL(mutedChanged()));

    QCOMPARE(audio.isMuted(), false);

    audio.setMuted(true);
    QCOMPARE(audio.isMuted(), true);
    QCOMPARE(provider.playerControl()->isMuted(), true);
    QCOMPARE(spy.count(), 1);

    provider.playerControl()->setMuted(false);
    QCOMPARE(audio.isMuted(), false);
    QCOMPARE(spy.count(), 2);

    audio.setMuted(false);
    QCOMPARE(audio.isMuted(), false);
    QCOMPARE(provider.playerControl()->isMuted(), false);
    QCOMPARE(spy.count(), 3);
}

void tst_QDeclarativeAudio::bufferProgress()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(&audio, SIGNAL(bufferProgressChanged()));

    QCOMPARE(audio.bufferProgress(), qreal(0.0));

    provider.playerControl()->setBufferStatus(20);
    QCOMPARE(audio.bufferProgress(), qreal(0.2));
    QCOMPARE(spy.count(), 1);

    provider.playerControl()->setBufferStatus(20);
    QCOMPARE(audio.bufferProgress(), qreal(0.2));
    QCOMPARE(spy.count(), 2);

    provider.playerControl()->setBufferStatus(40);
    QCOMPARE(audio.bufferProgress(), qreal(0.4));
    QCOMPARE(spy.count(), 3);

    connect(&audio, SIGNAL(positionChanged()), &QTestEventLoop::instance(), SLOT(exitLoop()));

    provider.playerControl()->updateMediaStatus(
            QMediaPlayer::BufferingMedia, QMediaPlayer::PlayingState);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(spy.count() > 3 && spy.count() < 6); // 4 or 5

    provider.playerControl()->updateMediaStatus(QMediaPlayer::BufferedMedia);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(spy.count() < 6);
}

void tst_QDeclarativeAudio::seekable()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(&audio, SIGNAL(seekableChanged()));

    QCOMPARE(audio.isSeekable(), false);

    provider.playerControl()->setSeekable(true);
    QCOMPARE(audio.isSeekable(), true);
    QCOMPARE(spy.count(), 1);

    provider.playerControl()->setSeekable(true);
    QCOMPARE(audio.isSeekable(), true);
    QCOMPARE(spy.count(), 2);

    provider.playerControl()->setSeekable(false);
    QCOMPARE(audio.isSeekable(), false);
    QCOMPARE(spy.count(), 3);
}

void tst_QDeclarativeAudio::playbackRate()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(&audio, SIGNAL(playbackRateChanged()));

    QCOMPARE(audio.playbackRate(), qreal(1.0));

    audio.setPlaybackRate(0.5);
    QCOMPARE(audio.playbackRate(), qreal(0.5));
    QCOMPARE(provider.playerControl()->playbackRate(), qreal(0.5));
    QCOMPARE(spy.count(), 1);

    provider.playerControl()->setPlaybackRate(2.0);
    QCOMPARE(provider.playerControl()->playbackRate(), qreal(2.0));
    QCOMPARE(spy.count(), 2);

    audio.setPlaybackRate(2.0);
    QCOMPARE(audio.playbackRate(), qreal(2.0));
    QCOMPARE(provider.playerControl()->playbackRate(), qreal(2.0));
    QCOMPARE(spy.count(), 3);
}

void tst_QDeclarativeAudio::status()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy statusChangedSpy(&audio, SIGNAL(statusChanged()));

    QCOMPARE(audio.status(), QDeclarativeAudio::NoMedia);

    // Set media, start loading.
    provider.playerControl()->updateMediaStatus(QMediaPlayer::LoadingMedia);
    QCOMPARE(audio.status(), QDeclarativeAudio::Loading);
    QCOMPARE(statusChangedSpy.count(), 1);

    // Finish loading.
    provider.playerControl()->updateMediaStatus(QMediaPlayer::LoadedMedia);
    QCOMPARE(audio.status(), QDeclarativeAudio::Loaded);
    QCOMPARE(statusChangedSpy.count(), 2);

    // Play, start buffering.
    provider.playerControl()->updateMediaStatus(
            QMediaPlayer::StalledMedia, QMediaPlayer::PlayingState);
    QCOMPARE(audio.status(), QDeclarativeAudio::Stalled);
    QCOMPARE(statusChangedSpy.count(), 3);

    // Enough data buffered to proceed.
    provider.playerControl()->updateMediaStatus(QMediaPlayer::BufferingMedia);
    QCOMPARE(audio.status(), QDeclarativeAudio::Buffering);
    QCOMPARE(statusChangedSpy.count(), 4);

    // Errant second buffering status changed.
    provider.playerControl()->updateMediaStatus(QMediaPlayer::BufferingMedia);
    QCOMPARE(audio.status(), QDeclarativeAudio::Buffering);
    QCOMPARE(statusChangedSpy.count(), 4);

    // Buffer full.
    provider.playerControl()->updateMediaStatus(QMediaPlayer::BufferedMedia);
    QCOMPARE(audio.status(), QDeclarativeAudio::Buffered);
    QCOMPARE(statusChangedSpy.count(), 5);

    // Buffer getting low.
    provider.playerControl()->updateMediaStatus(QMediaPlayer::BufferingMedia);
    QCOMPARE(audio.status(), QDeclarativeAudio::Buffering);
    QCOMPARE(statusChangedSpy.count(), 6);

    // Buffer full.
    provider.playerControl()->updateMediaStatus(QMediaPlayer::BufferedMedia);
    QCOMPARE(audio.status(), QDeclarativeAudio::Buffered);
    QCOMPARE(statusChangedSpy.count(), 7);

    // Finished.
    provider.playerControl()->updateMediaStatus(
            QMediaPlayer::EndOfMedia, QMediaPlayer::StoppedState);
    QCOMPARE(audio.status(), QDeclarativeAudio::EndOfMedia);
    QCOMPARE(statusChangedSpy.count(), 8);
}

void tst_QDeclarativeAudio::metaData_data()
{
    QTest::addColumn<QByteArray>("propertyName");
    QTest::addColumn<QtMultimediaKit::MetaData>("propertyKey");
    QTest::addColumn<QVariant>("value");

    QTest::newRow("title")
            << QByteArray("title")
            << QtMultimediaKit::Title
            << QVariant(QString::fromLatin1("This is a title"));

    QTest::newRow("genre")
            << QByteArray("genre")
            << QtMultimediaKit::Genre
            << QVariant(QString::fromLatin1("rock"));

    QTest::newRow("trackNumber")
            << QByteArray("trackNumber")
            << QtMultimediaKit::TrackNumber
            << QVariant(8);
}

void tst_QDeclarativeAudio::metaData()
{
    QFETCH(QByteArray, propertyName);
    QFETCH(QtMultimediaKit::MetaData, propertyKey);
    QFETCH(QVariant, value);

    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy spy(audio.metaData(), SIGNAL(metaDataChanged()));

    const int index = audio.metaData()->metaObject()->indexOfProperty(propertyName.constData());
    QVERIFY(index != -1);

    QMetaProperty property = audio.metaData()->metaObject()->property(index);
    QCOMPARE(property.read(&audio), QVariant());

    property.write(audio.metaData(), value);
    QCOMPARE(property.read(audio.metaData()), QVariant());
    QCOMPARE(provider.metaDataControl()->metaData(propertyKey), QVariant());
    QCOMPARE(spy.count(), 0);

    provider.metaDataControl()->setMetaData(propertyKey, value);
    QCOMPARE(property.read(audio.metaData()), value);
    QCOMPARE(spy.count(), 1);
}

void tst_QDeclarativeAudio::error()
{
    const QString errorString = QLatin1String("Failed to open device.");

    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;
    audio.classBegin();
    audio.componentComplete();

    QSignalSpy errorSpy(&audio, SIGNAL(error(QDeclarativeAudio::Error,QString)));
    QSignalSpy errorChangedSpy(&audio, SIGNAL(errorChanged()));

    QCOMPARE(audio.error(), QDeclarativeAudio::NoError);
    QCOMPARE(audio.errorString(), QString());

    provider.playerControl()->emitError(QMediaPlayer::ResourceError, errorString);

    QCOMPARE(audio.error(), QDeclarativeAudio::ResourceError);
    QCOMPARE(audio.errorString(), errorString);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorChangedSpy.count(), 1);

    // Changing the source resets the error properties.
    audio.setSource(QUrl("http://example.com"));
    QCOMPARE(audio.error(), QDeclarativeAudio::NoError);
    QCOMPARE(audio.errorString(), QString());
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorChangedSpy.count(), 2);

    // But isn't noisy.
    audio.setSource(QUrl("file:///file/path"));
    QCOMPARE(audio.error(), QDeclarativeAudio::NoError);
    QCOMPARE(audio.errorString(), QString());
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorChangedSpy.count(), 2);
}

void tst_QDeclarativeAudio::loops()
{
    QtTestMediaServiceProvider provider;
    QDeclarativeAudio audio;

    QSignalSpy loopsChangedSpy(&audio, SIGNAL(loopCountChanged()));
    QSignalSpy playingChangedSpy(&audio, SIGNAL(playingChanged()));
    QSignalSpy stoppedSpy(&audio, SIGNAL(stopped()));

    int playingChanged = 0;
    int stopped = 0;
    int loopsChanged = 0;

    audio.classBegin();
    audio.componentComplete();

    QCOMPARE(audio.isPlaying(), false);

    //setLoopCount(3) when stopped.
    audio.setLoopCount(3);
    QCOMPARE(audio.loopCount(), 3);
    QCOMPARE(loopsChangedSpy.count(), ++loopsChanged);

    //play till end
    audio.play();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);

    // setPlaying(true) when playing.
    audio.setPlaying(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(stoppedSpy.count(),          stopped);

    provider.playerControl()->updateMediaStatus(QMediaPlayer::EndOfMedia, QMediaPlayer::StoppedState);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);

    //play to end
    provider.playerControl()->updateMediaStatus(QMediaPlayer::EndOfMedia, QMediaPlayer::StoppedState);
    //play to end
    provider.playerControl()->updateMediaStatus(QMediaPlayer::EndOfMedia, QMediaPlayer::StoppedState);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(stoppedSpy.count(),        ++stopped);

    // stop when playing
    audio.play();
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    provider.playerControl()->updateMediaStatus(QMediaPlayer::EndOfMedia, QMediaPlayer::StoppedState);
    audio.stop();
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(stoppedSpy.count(),        ++stopped);

    //setPlaying(true) with infinite loop
    audio.setLoopCount(-1);
    QCOMPARE(audio.loopCount(), -1);
    QCOMPARE(loopsChangedSpy.count(), ++loopsChanged);
    audio.setPlaying(true);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    provider.playerControl()->updateMediaStatus(QMediaPlayer::EndOfMedia, QMediaPlayer::StoppedState);
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(), playingChanged);

    // play() when playing.
    audio.play();
    QCOMPARE(audio.isPlaying(), true);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::PlayingState);
    QCOMPARE(playingChangedSpy.count(),   playingChanged);
    QCOMPARE(stoppedSpy.count(),          stopped);

    // setPlaying(false) when playing in infinite loop.
    audio.setPlaying(false);
    QCOMPARE(audio.isPlaying(), false);
    QCOMPARE(provider.playerControl()->state(), QMediaPlayer::StoppedState);
    QCOMPARE(playingChangedSpy.count(), ++playingChanged);
    QCOMPARE(stoppedSpy.count(),        ++stopped);
}

QTEST_MAIN(tst_QDeclarativeAudio)

#include "tst_qdeclarativeaudio.moc"
