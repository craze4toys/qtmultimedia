requires(qtHaveModule(gui))

load(configure)
qtCompileTest(openal)
win32 {
    qtCompileTest(directshow) {
        qtCompileTest(wshellitem)
    }
    qtCompileTest(wmsdk)
    qtCompileTest(wmp)
    contains(QT_CONFIG, wmf-backend): qtCompileTest(wmf)
    qtCompileTest(evr)
} else:mac {
    qtCompileTest(avfoundation)
} else:qnx {
    qtCompileTest(mmrenderer)
} else {
    qtCompileTest(alsa)
    qtCompileTest(pulseaudio)

    isEmpty(GST_VERSION) {
        contains(QT_CONFIG, gstreamer-0.10) {
            GST_VERSION = 0.10
        } else: contains(QT_CONFIG, gstreamer-1.0) {
            GST_VERSION = 1.0
        }
    }
    cache(GST_VERSION, set)
    !isEmpty(GST_VERSION):qtCompileTest(gstreamer) {
        qtCompileTest(gstreamer_photography)
        qtCompileTest(gstreamer_encodingprofiles)
        qtCompileTest(gstreamer_appsrc)
        qtCompileTest(linux_v4l)
    }

    qtCompileTest(resourcepolicy)
    qtCompileTest(gpu_vivante)
}

load(qt_parts)

