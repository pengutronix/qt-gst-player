TEMPLATE = lib
CONFIG += plugin qmltypes link_pkgconfig
QT += qml quick concurrent

QT_CONFIG += pkg-config

PKGCONFIG += gstreamer-1.0

QML_IMPORT_NAME = GstPlayer
QML_IMPORT_MAJOR_VERSION = 1

DESTDIR = ../$$QML_IMPORT_NAME
TARGET = qt-gst-player

HEADERS += \
	gstplayer.hpp \
	plugin.hpp

SOURCES += \
	plugin.cpp \
	gstplayer.cpp

target.path = $$[QT_INSTALL_QML]/$$QML_IMPORT_NAME

plugin_install.files = qmldir qml/GstGLVideo.qml
plugin_install.path = $$[QT_INSTALL_QML]/$$QML_IMPORT_NAME

# Copy the qmldir file to the same folder as the plugin binary
plugin_copy.files = qmldir qml/GstGLVideo.qml
plugin_copy.path = $$DESTDIR
COPIES += plugin_copy

INSTALLS += target plugin_install
