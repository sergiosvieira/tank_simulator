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
    model/FirstRemotePolicy.cpp \
    model/IntelligentPolicy.cpp \
    model/Model.cpp \
    model/OffPolicy.cpp \
    model/RandomPolicy.cpp \
    events/SpecifiedTasksEvent.cpp \
    model/Task.cpp \
    model/Vehicle.cpp \

HEADERS += \
    events/SpecifiedTasksEvent.h \
    logger.h \
    metric.h \
    core/Event.h \
    core/Simulator.h \
    core/EnergyManager.h \
    events/CPUEvent.h \
    events/DecisionEvent.h \
    events/TaskGenerationEvent.h \
    model/CPU.h \
    model/EventType.h \
    model/FirstRemotePolicy.h \
    model/IntelligentPolicy.h \
    model/Model.h \
    model/OffPolicy.h \
    model/RSU.h \
    model/RandomPolicy.h \
    model/Task.h \
    model/Vehicle.h \
    model/Battery.h \
    utils/IdManager.h \
    utils/Rng.h
