=============
qt-gst-player
=============

This is a QML plugin (extension) that provides the GstGLVideo QML
video playback item.

Project Structure
=================

The project consists of two parts:

- The plugin itself, which is built as a library file. This is the primary
  target of this project and is the part that is meant to be used in other
  projects. All relevant files for the plugin reside in the plugin/ folder.

- A demo application which uses the plugin. All relevant files for the demo
  application can be found in the demo/ folder. The demo application is meant
  to test the plugin by setting up a minimal QML application with a GstGLVideo
  item inside it and some buttons to test its features and properties.
