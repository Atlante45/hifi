//
//  Created by Ryan Huffman on 2016-12-14
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_Trace_h
#define hifi_Trace_h

#include <cstdint>
#include <mutex>

#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QLoggingCategory>

#include "DependencyManager.h"

namespace tracing {

using TraceTimestamp = uint64_t;

enum EventType : char {
    DurationBegin = 'B',
    DurationEnd = 'E',

    Complete = 'X',
    Instant = 'i',
    Counter = 'C',

    AsyncNestableStart = 'b',
    AsyncNestableInstant = 'n',
    AsyncNestableEnd = 'e',

    FlowStart = 's',
    FlowStep = 't',
    FlowEnd = 'f',

    Sample = 'P',

    ObjectCreated = 'N',
    ObjectSnapshot = 'O',
    ObjectDestroyed = 'D',

    Metadata = 'M',

    MemoryDumpGlobal = 'V',
    MemoryDumpProcess = 'v',

    Mark = 'R',

    ClockSync = 'c',

    ContextEnter = '(',
    ContextLeave = ')'
};

enum Category : uint8_t {
    app,
    app_detail,
    metadata,
    network,
    parse,
    render,
    render_detail,
    render_gpu,
    render_gpu_gl,
    render_gpu_gl_detail,
    render_qml,
    render_qml_gl,
    render_qml_gl_detail,
    render_overlays,
    resource,
    resource_network,
    resource_parse,
    resource_parse_geometry,
    resource_parse_image,
    resource_parse_image_raw,
    resource_parse_image_ktx,
    script,
    script_entities,
    simulation,
    simulation_detail,
    simulation_animation,
    simulation_animation_detail,
    simulation_avatar,
    simulation_physics,
    simulation_physics_detail,
    startup,
    workload,
    test,

    NUM_CATEGORIES
};

struct TraceEvent {
    QString id;
    QString name;
    EventType type;
    qint64 timestamp;
    qint64 threadID;
    Category category;
    QVariantMap args;
    QVariantMap extra;

    void writeJson(QTextStream& out) const;
};

bool isTracingEnabled();
bool isTracingEnabled(Category category);

void startTracing();
void stopTracing();
void serialize(const QString& file);

void traceEvent(Category category, const QString& name, EventType type,
                const QString& id = QString(), const QVariantMap& args = QVariantMap(),
                const QVariantMap& extra = QVariantMap());

void traceMetadata(Category category, const QString& name, EventType type,
                   const QString& id = QString(), const QVariantMap& args = QVariantMap(),
                   const QVariantMap& extra = QVariantMap());

}

#endif // hifi_Trace_h
