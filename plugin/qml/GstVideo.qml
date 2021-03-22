// SPDX-License-Identifier: LGPL-2.1-only
/*
 * Copyright (C) 2021 Pengutronix, Marian Cichy <m.cichy@pengutronix.de>
 */

import QtQuick 2.15
import GstPlayer 1.0
import org.freedesktop.gstreamer.GLVideoItem 1.0


Item {
	property alias verbose : gst.verbose
	property alias state : gst.state
	property alias source : gst.source
	property alias position : gst.position
	property alias duration : gst.duration
	property alias audio : gst.audio
	property alias volume : gst.volume
	property alias mute : gst.mute
	property alias uploadVideoFormat : gst.uploadVideoFormat

	function play() { gst.play() }
	function pause() { gst.pause() }
	function stop() { gst.stop() }
	function seek(offset) { gst.seek(offset) }

	GstPlayer {
		id : gst
		sink : gstsink
	}

	GstGLVideoItem {
		id : gstsink
		anchors.fill : parent
	}
}
