// SPDX-License-Identifier: LGPL-2.1-only
/*
 * Copyright (C) 2021 Pengutronix, Marian Cichy <m.cichy@pengutronix.de>
 */

#pragma once

#include <QObject>
#include <QtPlugin>
#include <QtQml/qqml.h>
#include <QString>
#include <QTimer>

#include <gst/gst.h>

class QtGstPipeline : public QObject, public QQmlParserStatus
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)
	Q_PROPERTY(bool verbose READ verbose WRITE setVerbose NOTIFY verboseChanged)
	Q_PROPERTY(PipelineState state READ state WRITE setState NOTIFY stateChanged)
	Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
	Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
	Q_PROPERTY(gboolean seekable READ seekable NOTIFY seekableChanged)
	Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
	Q_PROPERTY(QObject* sink READ sink WRITE setSink)
	Q_PROPERTY(bool audio READ audio WRITE setAudio)
	Q_PROPERTY(double volume READ volume WRITE setVolume)
	Q_PROPERTY(bool mute READ mute WRITE setMute)
	Q_PROPERTY(QString uploadVideoFormat READ uploadVideoFormat WRITE setUploadVideoFormat)
	QML_NAMED_ELEMENT(GstPlayer)

public:
	enum class PipelineState {
		Play,
		Pause,
		Stop
	};
	Q_ENUM(PipelineState)

	enum PlaybinFlags {
		AUDIO = 0x02,
		NATIVE_VIDEO = 0x40
	};

	QtGstPipeline();
	~QtGstPipeline();

	PipelineState state() const;
	void setState(PipelineState state);

	QString source() const;
	void setSource(QString source);

	QObject *sink() const;
	void setSink(QObject *sink);

	qint64 position() const;
	void setPosition(qint64 position);

	qint64 duration() const;
	void queryDuration();

	gboolean seekable() const;

	bool verbose() const;
	void setVerbose(bool verbose);

	bool audio() const;
	void setAudio(bool audio);

	bool mute() const;
	void setMute(bool mute);

	double volume() const;
	void setVolume(double volume);

	QString uploadVideoFormat() const;
	void setUploadVideoFormat(QString format);

	virtual void classBegin() override;
	virtual void componentComplete() override;

private:
	void updatePipeline();

	static void setup_element_callback(GstElement *bin, GstElement *element, gpointer data);
	void setupElementCallback(GstElement *element);

	static gboolean bus_callback(GstBus *bus, GstMessage *msg, gpointer data);
	bool busCallback(GstBus *bus, GstMessage *msg);

	GstElement *m_pipeline;
	QString m_source;
	QObject *m_sink;
	PipelineState m_state;

	qint64 m_position;
	qint64 m_duration;
	bool m_durationChanged;
	gboolean m_seekable;
	bool m_audio;
	double m_volume;
	bool m_mute;
	guint64 m_connectionSpeed;
	bool m_loop;
	bool m_ready;
	bool m_verbose;
	QString m_uploadVideoFormat;

	QTimer m_positionUpdater;

public slots:
	void onItemInitializeChanged();
	void queryPosition();

	void play();
	void pause();
	void stop();
	void seek(qint64 offset);

signals:
	void verboseChanged();
	void stateChanged(PipelineState newState);
	void positionChanged(gint64 newPosition);
	void durationChanged(gint64 newDuration);
	void seekableChanged();
	void sourceChanged();

};
