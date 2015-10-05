TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    c_collection.c \
    cJSON.c \
    c_str.c \
    murmurhash.c \
    rehash_dict.c \
    solider.c \
    heartbeat.c \
    unix_sock.c \
    configuration.c

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    c_collection.h \
    cJSON.h \
    c_str.h \
    murmurhash.h \
    rehash_dict.h \
    solider.h \
    heartbeat.h \
    unix_sock.h \
    configuration.h \
    macro_definition.h

LIBS += -lpthread
