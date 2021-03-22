// SPDX-License-Identifier: 0BSD
/*
 * Copyright (C) 2021 Pengutronix, Philipp Zabel <p.zabel@pengutronix.de>
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

function toHHMMSS(msecs) {
	var hours   = Math.floor(msecs / 3600000)
	var minutes = Math.floor(msecs / 60000) % 60
	var seconds = Math.floor(msecs / 1000) % 60
	var text = minutes + ":" + (seconds < 10 ? "0" + seconds : seconds)

	if (hours > 0) {
		return hours + (minutes < 10 ? ":0" : ":") + text
	} else {
		return text
	}
}
