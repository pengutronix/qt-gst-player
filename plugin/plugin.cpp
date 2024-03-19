// SPDX-License-Identifier: LGPL-2.1-only
/*
 * Copyright (C) 2021 Pengutronix, Marian Cichy <m.cichy@pengutronix.de>
 */

#include "plugin.hpp"

#include <gst/gst.h>

void QGstPlayerQmlPlugin::registerTypes(const char *uri)
{
	Q_UNUSED(uri);
	gst_init(0,0);
	// registers plugin for "import org.freedesktop.gstreamer.GLVideoItem 1.0"
	gst_plugin_load_by_name("qml6");
}
