
CONFIG += qt
CONFIG += debug
#CONFIG += model_test
#CONFIG += debug_and_release build_all
QT += gui widgets network

build_pass:CONFIG(debug, debug|release) {
        TARGET = envgui_debug
        MOC_DIR = .gen/debug/moc
        OBJECTS_DIR = .gen/debug/obj
        RCC_DIR = .gen/debug/rcc
        UI_DIR = .gen/debug/ui
} else {
        TARGET = envgui
        MOC_DIR = .gen/release/moc
        OBJECTS_DIR = .gen/release/obj
        RCC_DIR = .gen/release/rcc
        UI_DIR = .gen/release/ui
}

DEPEND_PATH += $$INCLUDE_PATH

SOURCES = main.cpp \
          xmlparser.cpp \
          xml.cpp \
          bookmarkitem.cpp \
          bookmarkrootitem.cpp \
          bookmarktreeview.cpp \
          bookmarksyncer.cpp \
          faviconstore.cpp \
          faviconfetch.cpp \
          bookmarkwindow.cpp \
          mainobject.cpp \
          menuview.cpp \
          lockfile.cpp

HEADERS = xmlparser.h \
          xml.h \
          bookmarkitem.h \
          bookmarkrootitem.h \
          bookmarktreeview.h \
          bookmarksyncer.h \
          faviconstore.h \
          faviconfetch.h \
          bookmarkwindow.h \
          mainobject.h \
          menuview.h \
          lockfile.h

RESOURCES = res.qrc

model_test {
        QT += testlib
        SOURCES += modeltest.cpp
        HEADERS += modeltest.h
        DEFINES += MODEL_TEST
}


