TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD

SOURCES += \
    main.cpp \
    core/Simulator.cpp \
    events/TaskGenerationEvent.cpp \
    model/CPU.cpp \
    model/Model.cpp \
    model/OffPolicy.cpp \
    model/Task.cpp \
    model/Vehicle.cpp

HEADERS += \
    logger.h \
    metric.h \
    core/Event.h \
    core/Simulator.h \
    events/CPUEvent.h \
    events/DecisionEvent.h \
    events/TaskGenerationEvent.h \
    model/CPU.h \
    model/EventType.h \
    model/Model.h \
    model/OffPolicy.h \
    model/RSU.h \
    model/Task.h \
    model/Vehicle.h \
    utils/IdManager.h \
    utils/Rng.h
