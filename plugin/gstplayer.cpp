// SPDX-License-Identifier: LGPL-2.1-only
/*
 * Copyright (C) 2021 Pengutronix, Marian Cichy <m.cichy@pengutronix.de>
 */

#include <QLoggingCategory>
#include <QtConcurrent>

#include "gstplayer.hpp"

Q_LOGGING_CATEGORY(lcGstPlayer, "gst.player", QtWarningMsg)

QtGstPlayer::QtGstPlayer() :
		m_pipeline(NULL),
		m_sink(NULL),
		m_state(PipelineState::Play),
		m_position(0),
		m_duration(0),
		m_durationChanged(false),
		m_seekable(false),
		m_audio(true),
		m_volume(1.0),
		m_mute(false),
		m_connectionSpeed(1),
		m_loop(false),
		m_ready(false),
		m_verbose(false),
		m_uploadVideoFormat("")
{
	connect(&m_positionUpdater, SIGNAL(timeout()), this, SLOT(queryPosition()));
}

QtGstPlayer::~QtGstPlayer()
{
	gst_element_set_state(m_pipeline, GST_STATE_NULL);
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
	gst_bus_remove_watch(bus);
	gst_object_unref(bus);
	g_object_unref(m_pipeline);
}

void QtGstPlayer::play()
{
	setState(PipelineState::Play);
}

void QtGstPlayer::pause()
{
	setState(PipelineState::Pause);
}

void QtGstPlayer::stop()
{
	setState(PipelineState::Stop);
}

void QtGstPlayer::seek(qint64 offset)
{
	if (m_seekable) {
		queryPosition();
		setPosition(m_position + offset);
	}
	else {
		qCInfo(lcGstPlayer, "seeking is not available in this stream");
	}
}

bool QtGstPlayer::verbose() const
{
	return m_verbose;
}

void QtGstPlayer::setVerbose(bool verbose)
{
	if (verbose != m_verbose) {
		m_verbose = verbose;
		emit verboseChanged();
	}
}

bool
QtGstPlayer::audio() const
{
	return m_audio;
}

void
QtGstPlayer::setAudio(bool audio)
{
	m_audio = audio;
}

bool
QtGstPlayer::mute() const
{
	return m_mute;
}

void
QtGstPlayer::setMute(bool mute)
{
	qCDebug(lcGstPlayer, "audio %s", mute ? "muted" : "unmuted");
	m_mute = mute;
	if (m_pipeline)
		g_object_set(G_OBJECT(m_pipeline), "mute", mute, NULL);
}

bool
QtGstPlayer::loop() const
{
	return m_loop;
}

void
QtGstPlayer::setLoop(bool loop)
{
	m_loop = loop;
}

double
QtGstPlayer::volume() const
{
	return m_volume;
}

void
QtGstPlayer::setVolume(double volume)
{
	qCDebug(lcGstPlayer, "volume set to %.1f %%", 100.0 * volume);
	m_volume = volume;
	if (m_pipeline)
		g_object_set(G_OBJECT(m_pipeline), "volume", volume, NULL);
}

void
QtGstPlayer::queryPosition()
{
	gint64 position;

	if (m_pipeline) {
		if (gst_element_query_position (m_pipeline, GST_FORMAT_TIME, &position)) {
			position = position / 1'000'000;
		}
		else {
			return;
		}
	}
	else {
		position = 0;
	}

	if (position != m_position) {
		m_position = position;
		if (QThread::currentThread() == thread()) {
			emit positionChanged(m_position);
		}
		else {
			qCDebug(lcGstPlayer, "emitting queued positionChanged signal");
			QMetaObject::invokeMethod(this, "positionChanged",
				Qt::QueuedConnection, Q_ARG(qint64, m_position));
		}
	}
}

qint64 QtGstPlayer::position() const
{
	return m_position;
}

void QtGstPlayer::setPosition(qint64 position)
{
	if (position < 0)
		position = 0;

	if (m_seekable) {
		gint64 start;
		qCDebug(lcGstPlayer, "set position to %lld", position);

		if (m_pipeline) {
			start =  position * 1'000'000;
			gst_element_seek (m_pipeline, 1.0, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, start,
				GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
		}
		else {
			position = 0;
		}

		if (position != m_position) {
			m_position = position;
			if (QThread::currentThread() == thread()) {
				emit positionChanged(position);
			}
			else {
				qCDebug(lcGstPlayer, "emitting queued positionChanged signal");
				QMetaObject::invokeMethod(this, "positionChanged",
					Qt::QueuedConnection, Q_ARG(qint64, position));
			}
		}
	}
	else {
		qCInfo(lcGstPlayer, "setting position is not available in this stream");
	}
}

qint64 QtGstPlayer::duration() const
{
	return m_duration;
}

void
QtGstPlayer::queryDuration()
{
	GstState state;
	gint64 duration = 0;

	if (m_pipeline) {
		gst_element_get_state (m_pipeline, &state, NULL, 0);
		if (state >= GST_STATE_PAUSED &&
		    gst_element_query_duration (m_pipeline, GST_FORMAT_TIME, &duration)) {
			duration = duration / 1'000'000;
			if (duration > 0) {
				GstQuery *seek_query = gst_query_new_seeking(GST_FORMAT_TIME);
				if (gst_element_query(m_pipeline, seek_query))
					gst_query_parse_seeking(seek_query, NULL, &m_seekable, NULL, NULL);
				gst_query_unref(seek_query);
			}
		}
		else {
			return;
		}
	}

	if (duration != m_duration) {
		m_duration = duration;
		qCDebug(lcGstPlayer, "duration changed to %lld", m_duration);
		m_durationChanged = false;
		if (QThread::currentThread() == thread()) {
			emit durationChanged(m_duration);
		}
		else {
			qCDebug(lcGstPlayer, "emitting queued durationChanged signal");
			QMetaObject::invokeMethod(this, "durationChanged",
				Qt::QueuedConnection, Q_ARG(qint64, m_duration));
		}
	}
}

gboolean QtGstPlayer::seekable() const
{
	return m_seekable;
}

QString QtGstPlayer::source() const
{
	return m_source;
}

void QtGstPlayer::setSource(QString source)
{
	m_source = source;
	updatePipeline();
}

QString QtGstPlayer::uploadVideoFormat() const
{
	return m_uploadVideoFormat;
}

void QtGstPlayer::setUploadVideoFormat(QString format)
{
	m_uploadVideoFormat = format;
}

QObject*
QtGstPlayer::sink() const
{
	return m_sink;
}

void
QtGstPlayer::setSink(QObject *sink)
{
	m_sink = sink;
	connect(m_sink, SIGNAL(itemInitializedChanged()),
		this, SLOT(onItemInitializeChanged()));
	updatePipeline();
}

void
QtGstPlayer::setupElementCallback(GstElement *element)
{
	if (g_strcmp0("v4l2h264dec", G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(element))) == 0)
		g_object_set(G_OBJECT(element), "capture-io-mode", 4, NULL);
}

void
QtGstPlayer::setup_element_callback(GstElement *bin, GstElement *element, gpointer data)
{
	Q_UNUSED(bin);
	QtGstPlayer *pipeline = static_cast<QtGstPlayer*>(data);
	pipeline->setupElementCallback(element);
}

QtGstPlayer::PipelineState QtGstPlayer::state() const
{
	return m_state;
}

void
QtGstPlayer::setState(PipelineState state)
{
	GstState gst_state, pending;
	bool isPlaying;

	if (!m_pipeline) {
		if (state != m_state) {
			m_state = state;
			emit stateChanged(state);
		}
		return;
	}

	gst_element_get_state(m_pipeline, &gst_state, &pending, 0);
	if (pending == GST_STATE_VOID_PENDING)
		isPlaying = gst_state == GST_STATE_PLAYING;
	else
		isPlaying = pending == GST_STATE_PLAYING;

	if (isPlaying && state == PipelineState::Play)
		return;

	if (m_sink->property("itemInitialized").toBool()) {
		switch(state) {
		case PipelineState::Play:
			QtConcurrent::run(gst_element_set_state, m_pipeline, GST_STATE_PLAYING);
			m_positionUpdater.start(500);
			break;

		case PipelineState::Pause:
			GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(m_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline.dot");
			m_positionUpdater.stop();
			QtConcurrent::run(gst_element_set_state, m_pipeline, GST_STATE_PAUSED);
			break;

		case PipelineState::Stop:
			m_positionUpdater.stop();
			QtConcurrent::run(gst_element_set_state, m_pipeline, GST_STATE_NULL);
			break;
		}
	}

	m_state = state;
	emit stateChanged(state);
}

void QtGstPlayer::updatePipeline()
{

	if (!m_ready)
		return;

	if (m_pipeline) {
		gst_element_set_state(m_pipeline, GST_STATE_NULL);
		GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
		gst_bus_remove_watch(bus);
		gst_object_unref(bus);
		gst_object_unref(m_pipeline);
		m_pipeline = NULL;
	}

	if (!m_sink || m_source.isEmpty())
		return;

	GstElement *sinkBin = gst_element_factory_make("bin", NULL);
	GstElement *upload = gst_element_factory_make("glupload", NULL);
	GstElement *sink = gst_element_factory_make("qml6glsink", NULL);

	g_object_set(G_OBJECT(sink), "widget", m_sink,
				     "processing-deadline", 35 * GST_MSECOND,
				     "max-lateness", 15 * GST_MSECOND, NULL);

	gst_bin_add_many(GST_BIN(sinkBin), upload, sink, NULL);
	gst_element_link_many(upload, sink, NULL);

	GstPad *pad = gst_element_get_static_pad(upload, "sink");

	/* construct a caps-filter only if the format property is set */
	if (!uploadVideoFormat().isEmpty()) {
		GstElement *capsfilter = gst_element_factory_make("capsfilter", NULL);
		GstCaps *caps = gst_caps_new_simple("video/x-raw",
				"format", G_TYPE_STRING, uploadVideoFormat().toStdString().c_str(),
				NULL);

		g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
		gst_caps_unref(caps);

		gst_bin_add(GST_BIN(sinkBin), capsfilter);
		gst_element_link(capsfilter, upload);

		gst_object_unref(pad);
		pad = gst_element_get_static_pad(capsfilter, "sink");
	}

	gst_element_add_pad(sinkBin, gst_ghost_pad_new("sink", pad));
	gst_object_unref(pad);

	QString prefix = "gst-pipeline:";
	QString source = m_source;

	/* default pipeline (playbin3) */
	if (source.indexOf(prefix) != 0) {
		m_pipeline = gst_element_factory_make("playbin3", NULL);

		g_signal_connect(m_pipeline, "element-setup", G_CALLBACK(setup_element_callback), this);

		if (!m_pipeline) {
			qCWarning(lcGstPlayer, "Failed to create playbin3 element, \
				  is the GStreamer playback plugin installed?");
			return;
		}

		uint32_t flags = NATIVE_VIDEO;
		flags |= m_audio ? AUDIO : 0;

		g_object_set(G_OBJECT(m_pipeline),
			"flags", flags,
			"video-sink", sinkBin,
			"mute", m_mute,
			"volume", m_volume,
			"connection-speed", m_connectionSpeed,
			"uri", source.toLatin1().data(),
			NULL);
	}
	/* custom pipeline */
	else {
		source.remove(0, prefix.length());
		m_pipeline = gst_pipeline_new("QtGstPlayer");
		GstElement *sourceBin = gst_parse_bin_from_description(source.toLatin1().data(), true, NULL);
		gst_bin_add_many(GST_BIN(m_pipeline), sourceBin, sinkBin, NULL);
		gst_element_link(sourceBin, sinkBin);
	}

	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
	gst_bus_add_watch(bus, QtGstPlayer::bus_callback, this);
	gst_object_unref(bus);

	if (m_verbose)
		g_signal_connect(m_pipeline, "deep-notify",
				G_CALLBACK(gst_object_default_deep_notify), NULL);

	setState(m_state);
}

gboolean
QtGstPlayer::bus_callback(GstBus *bus, GstMessage *msg, gpointer data)
{
	QtGstPlayer *pipeline = static_cast<QtGstPlayer*>(data);
	return pipeline->busCallback(bus, msg);
}

bool
QtGstPlayer::busCallback(GstBus *bus, GstMessage *msg)
{
	Q_UNUSED(bus);
	switch(GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_EOS:
		qCDebug(lcGstPlayer, "got EOS for %s", source().toLatin1().data());
		if (!m_loop) {
			m_state = PipelineState::Stop;
			emit stateChanged(m_state);
		}
		updatePipeline();
		break;
	case GST_MESSAGE_DURATION_CHANGED:
		m_durationChanged = true;
		queryDuration();
		break;
	case GST_MESSAGE_ASYNC_DONE:
		if (m_durationChanged)
			queryDuration();
		break;
	case GST_MESSAGE_LATENCY:
		gst_bin_recalculate_latency(GST_BIN(m_pipeline));
		break;
	case GST_MESSAGE_ERROR:
	{
		GError *err;
		gchar *debug;
		gst_message_parse_error(msg, &err, &debug);
		qCCritical(lcGstPlayer, "GStreamer Error Message: %s - %s",err->message, debug);
		g_free(debug);
		g_error_free(err);
		break;
	}
	case GST_MESSAGE_ELEMENT:
		if (gst_message_has_name(msg, "adaptive-streaming-statistics")) {
			const GstStructure *s = gst_message_get_structure(msg);
			if (gst_structure_has_field(s, "uri") &&
			    gst_structure_has_field(s, "bitrate")) {
				qCDebug(lcGstPlayer, "switch source uri: %s", gst_structure_get_string(s, "uri"));
			}
		}
		break;
	default:
		break;
	}
	return true;
}

void
QtGstPlayer::onItemInitializeChanged()
{
	if (m_sink->property("itemInitialized").toBool())
		setState(m_state);
}

void
QtGstPlayer::classBegin()
{
}

void
QtGstPlayer::componentComplete()
{
	m_ready = true;
	updatePipeline();
}
