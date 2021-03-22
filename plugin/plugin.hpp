// SPDX-License-Identifier: LGPL-2.1-only
/*
 * Copyright (C) 2021 Pengutronix, Marian Cichy <m.cichy@pengutronix.de>
 */

#include <QQmlEngineExtensionPlugin>

class QGstPipelineQmlPlugin : public QQmlExtensionPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)
public:
	void registerTypes(const char *uri) override;
};
