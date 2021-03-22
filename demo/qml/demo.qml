// SPDX-License-Identifier: 0BSD
/*
 * Copyright (C) 2021 Pengutronix, Marian Cichy <m.cichy@pengutronix.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15

import GstPlayer 1.0

import "demo.js" as Script

Rectangle {
	id : root
	color : "red"
	Row {
		id: toolbar
		width: parent.width
		z : 10

		Button {
			id: playpause
			text: qsTr("Play/Pause")
			onClicked: { player.state == GstPlayer.Play ? player.pause() : player.play() }
		}

		Button {
			text: qsTr("Stop")
			onClicked: { player.stop() }
		}

		Button {
			text: qsTr("Seek -10")
			onClicked: { player.seek(-10000) }
		}

		Button {
			text: qsTr("Seek +10")
			onClicked: { player.seek(10000) }
		}

		Button {
			text: "quit"
			onClicked: { Qt.quit() }
		}

		Rectangle {
			id: time
			height: toolbar.height
			width: text.width

			Text {
				id: text
				anchors.verticalCenter: time.verticalCenter
				text: Script.toHHMMSS(player.position) + '/' + Script.toHHMMSS(player.duration)
			}
		}
	}

	GstVideo {
		id: player
		y: toolbar.height
		width: parent.width
		height: parent.height - toolbar.height
		uploadVideoFormat: "RGBA"
		source: "gst-pipeline: videotestsrc"
	}
}
