//
//  Created by Ryan Huffman on 2016-12-14
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Trace.h"

#include <array>
#include <chrono>
#include <assert.h>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryFile>
#include <QtCore/QDataStream>
#include <QtCore/QTextStream>

#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>

#include <BuildInfo.h>

#include "Gzip.h"
#include "PortableHighResolutionClock.h"
#include "SharedLogging.h"
#include "shared/FileUtils.h"
#include "shared/GlobalAppProperties.h"

// When profiling something that may happen many times per frame, use a xxx_detail category so that they may easily be filtered out of trace results
Q_LOGGING_CATEGORY(trace_app, "trace.app")
Q_LOGGING_CATEGORY(trace_app_detail, "trace.app.detail")
Q_LOGGING_CATEGORY(trace_metadata, "trace.metadata")
Q_LOGGING_CATEGORY(trace_network, "trace.network")
Q_LOGGING_CATEGORY(trace_parse, "trace.parse")
Q_LOGGING_CATEGORY(trace_render, "trace.render")
Q_LOGGING_CATEGORY(trace_render_detail, "trace.render.detail")
Q_LOGGING_CATEGORY(trace_render_gpu, "trace.render.gpu")
Q_LOGGING_CATEGORY(trace_render_gpu_gl, "trace.render.gpu.gl")
Q_LOGGING_CATEGORY(trace_render_gpu_gl_detail, "trace.render.gpu.gl.detail")
Q_LOGGING_CATEGORY(trace_render_qml, "trace.render.qml")
Q_LOGGING_CATEGORY(trace_render_qml_gl, "trace.render.qml.gl")
Q_LOGGING_CATEGORY(trace_render_qml_gl_detail, "trace.render.qml.gl.detail")
Q_LOGGING_CATEGORY(trace_render_overlays, "trace.render.overlays")
Q_LOGGING_CATEGORY(trace_resource, "trace.resource")
Q_LOGGING_CATEGORY(trace_resource_network, "trace.resource.network")
Q_LOGGING_CATEGORY(trace_resource_parse, "trace.resource.parse")
Q_LOGGING_CATEGORY(trace_resource_parse_geometry, "trace.resource.parse.geometry")
Q_LOGGING_CATEGORY(trace_resource_parse_image, "trace.resource.parse.image")
Q_LOGGING_CATEGORY(trace_resource_parse_image_raw, "trace.resource.parse.image.raw")
Q_LOGGING_CATEGORY(trace_resource_parse_image_ktx, "trace.resource.parse.image.ktx")
Q_LOGGING_CATEGORY(trace_script, "trace.script")
Q_LOGGING_CATEGORY(trace_script_entities, "trace.script.entities")
Q_LOGGING_CATEGORY(trace_simulation, "trace.simulation")
Q_LOGGING_CATEGORY(trace_simulation_detail, "trace.simulation.detail")
Q_LOGGING_CATEGORY(trace_simulation_animation, "trace.simulation.animation")
Q_LOGGING_CATEGORY(trace_simulation_animation_detail, "trace.simulation.animation.detail")
Q_LOGGING_CATEGORY(trace_simulation_avatar, "trace.simulation.avatar");
Q_LOGGING_CATEGORY(trace_simulation_physics, "trace.simulation.physics")
Q_LOGGING_CATEGORY(trace_simulation_physics_detail, "trace.simulation.physics.detail")
Q_LOGGING_CATEGORY(trace_startup, "trace.startup")
Q_LOGGING_CATEGORY(trace_workload, "trace.workload")
Q_LOGGING_CATEGORY(trace_test, "trace.test")

namespace tracing {

static const char* CATEGORY_NAMES[] = {
    "trace.app",
    "trace.app.detail",
    "trace.metadata",
    "trace.network",
    "trace.parse",
    "trace.render",
    "trace.render.detail",
    "trace.render.gpu",
    "trace.render.gpu.gl",
    "trace.render.gpu.gl.detail",
    "trace.render.qml",
    "trace.render.qml.gl",
    "trace.render.qml.gl.detail",
    "trace.render.overlays",
    "trace.resource",
    "trace.resource.network",
    "trace.resource.parse",
    "trace.resource.parse.geometry",
    "trace.resource.parse.image",
    "trace.resource.parse.image.raw",
    "trace.resource.parse.image.ktx",
    "trace.script",
    "trace.script.entities",
    "trace.simulation",
    "trace.simulation.detail",
    "trace.simulation.animation",
    "trace.simulation.animation.detail",
    "trace.simulation.avatar",
    "trace.simulation.physics",
    "trace.simulation.physics.detail",
    "trace.startup",
    "trace.workload",
    "trace.test"
};
static_assert(sizeof(CATEGORY_NAMES) / sizeof(CATEGORY_NAMES[0]) == NUM_CATEGORIES,
              "CATEGORY_NAMES array has the wrong number of element");

static const QLoggingCategory* LOGGING_CATEGORIES[] = {
    &trace_app(),
    &trace_app_detail(),
    &trace_metadata(),
    &trace_network(),
    &trace_parse(),
    &trace_render(),
    &trace_render_detail(),
    &trace_render_gpu(),
    &trace_render_gpu_gl(),
    &trace_render_gpu_gl_detail(),
    &trace_render_qml(),
    &trace_render_qml_gl(),
    &trace_render_qml_gl_detail(),
    &trace_render_overlays(),
    &trace_resource(),
    &trace_resource_network(),
    &trace_resource_parse(),
    &trace_resource_parse_geometry(),
    &trace_resource_parse_image(),
    &trace_resource_parse_image_raw(),
    &trace_resource_parse_image_ktx(),
    &trace_script(),
    &trace_script_entities(),
    &trace_simulation(),
    &trace_simulation_detail(),
    &trace_simulation_animation(),
    &trace_simulation_animation_detail(),
    &trace_simulation_avatar(),
    &trace_simulation_physics(),
    &trace_simulation_physics_detail(),
    &trace_startup(),
    &trace_workload(),
    &trace_test()
};
static_assert(sizeof(LOGGING_CATEGORIES) / sizeof(LOGGING_CATEGORIES[0]) == NUM_CATEGORIES,
              "LOGGING_CATEGORIES array has the wrong number of element");

using Data = TraceEvent;
using DataStorage = std::list<Data>;

class ThreadLocalList;

auto processID = QCoreApplication::applicationPid();

struct Tracer {
    std::atomic_bool enabled { false };

    std::mutex masterMutex;
    DataStorage masterList;

    std::mutex metadataMutex;
    DataStorage metadataList;

    std::vector<ThreadLocalList*> threads;

    std::array<bool, NUM_CATEGORIES> categoriesEnabled;
} tracer;

class ThreadLocalList {
public:
    ThreadLocalList() {
        std::lock_guard<std::mutex> guard(tracer.masterMutex);

        // Register this thread in the master list
        tracer.threads.push_back(this);
    }
    ~ThreadLocalList() {
        std::lock(mutex, tracer.masterMutex);
        std::lock_guard<std::mutex> guard1(mutex, std::adopt_lock);
        std::lock_guard<std::mutex> guard2(tracer.masterMutex, std::adopt_lock);

        // Add this thread's data into the master list
        tracer.masterList.splice(std::end(tracer.masterList), threadList);

        // Unregister this thread from the master list
        auto it = std::remove(std::begin(tracer.threads), std::end(tracer.threads), this);
        assert(it != std::end(tracer.threads));
        tracer.threads.erase(it, std::end(tracer.threads));
    }

    void add(Data data) {
        std::lock_guard<std::mutex> guard(mutex);
        threadList.push_back(data);
    }

    void collect(DataStorage* storage) {
        std::lock_guard<std::mutex> guard(mutex, std::adopt_lock);

        // Add this thread's data into the provided storage
        storage->splice(std::end(*storage), threadList);
    }

private:
    std::mutex mutex;
    DataStorage threadList;
};

thread_local ThreadLocalList threadList;

void addMetadata(Data data) {
    std::lock_guard<std::mutex> guard(tracer.metadataMutex);
    tracer.metadataList.push_back(data);
}

void collectData(DataStorage* storage) {
    std::lock_guard<std::mutex> guard(tracer.masterMutex);
    storage->clear();
    std::swap(*storage, tracer.masterList);
    for (auto& thread : tracer.threads) {
        thread->collect(storage);
    }
}

void clearEvents() {
    std::lock_guard<std::mutex> guard(tracer.masterMutex);
    for (auto& thread : tracer.threads) {
        thread->collect(&tracer.masterList);
    }
    tracer.masterList.clear();
}




bool isTracingEnabled() {
    return tracer.enabled.load();
}
    
bool isTracingEnabled(Category category) {
    return tracer.enabled.load() && tracer.categoriesEnabled[category];
}

void startTracing() {
    if (tracer.enabled.load()) {
        qWarning() << "Tried to enable tracer, but already enabled";
        return;
    }

    clearEvents();
    for (int i = 0; i < NUM_CATEGORIES; ++i) {
        assert(LOGGING_CATEGORIES[i]->categoryName() == CATEGORY_NAMES[i]);
        tracer.categoriesEnabled[i] = LOGGING_CATEGORIES[i]->isDebugEnabled();
    }
    tracer.enabled.store(true);
}

void stopTracing() {
    if (!tracer.enabled.load()) {
        qWarning() << "Cannot stop tracing, already disabled";
        return;
    }
    tracer.enabled.store(false);
}


void TraceEvent::writeJson(QTextStream& out) const {
#if 0
    // FIXME QJsonObject serialization is very slow, so we should be using manual JSON serialization
    out << "{";
    out << "\"name\":\"" << name << "\",";
    out << "\"cat\":\"" << CATEGORY_NAMES[category] << "\",";
    out << "\"ph\":\"" << QString(type) << "\",";
    out << "\"ts\":\"" << timestamp << "\",";
    out << "\"pid\":\"" << processID << "\",";
    out << "\"tid\":\"" << threadID << "\"";
    //if (!extra.empty()) {
    //    auto it = extra.begin();
    //    for (; it != extra.end(); it++) {
    //        ev[it.key()] = QJsonValue::fromVariant(it.value());
    //    }
    //}
    //if (!args.empty()) {
    //    out << ",\"args\":'
    //}
    out << '}';
#else
    QJsonObject ev {
        { "name", QJsonValue(name) },
        { "cat", CATEGORY_NAMES[category] },
        { "ph", QString(type) },
        { "ts", timestamp },
        { "pid", processID },
        { "tid", threadID }
    };
    if (!id.isEmpty()) {
        ev["id"] = id;
    }
    if (!args.empty()) {
        ev["args"] = QJsonObject::fromVariantMap(args);
    }
    if (!extra.empty()) {
        auto it = extra.begin();
        for (; it != extra.end(); it++) {
            ev[it.key()] = QJsonValue::fromVariant(it.value());
        }
    }
    out << QJsonDocument(ev).toJson(QJsonDocument::Compact);
#endif
}

void serialize(const QString& filename) {
    QString fullPath = FileUtils::replaceDateTimeTokens(filename);
    fullPath = FileUtils::computeDocumentPath(fullPath);
    if (!FileUtils::canCreateFile(fullPath)) {
        return;
    }

    DataStorage currentEvents;
    collectData(&currentEvents);

    // If we can't open a temp file for writing, fail early
    QByteArray data;
    {
        QTextStream out(&data);
        out << "[";
        for (const auto& event : currentEvents) {
            event.writeJson(out);
            out << ",";
        }
        out.seek(out.pos() - 1);
        out << "]";
    }

    if (fullPath.endsWith(".gz")) {
        QByteArray compressed;
        gzip(data, compressed);
        data = compressed;
    }

    {
        QFile file(fullPath);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug(shared) << "failed to open file '" << fullPath << "'";
            return;
        }
        file.write(data);
        file.close();
    }
}

void traceEvent(Category category,
                         const QString& name, EventType type, const QString& id,
                         const QVariantMap& args, const QVariantMap& extra) {

    assert(type != Metadata);
    if (!tracer.enabled.load()) {
        return;
    }

    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(p_high_resolution_clock::now().time_since_epoch()).count();
    auto threadID = int64_t(QThread::currentThreadId());

    threadList.add({
        id,
        name,
        type,
        timestamp,
        threadID,
        category,
        args,
        extra
    });
}

void traceMetadata(Category category,
                            const QString& name, EventType type, const QString& id,
                            const QVariantMap& args, const QVariantMap& extra){

    // We always want to store metadata events even if tracing is not enabled so that when
    // tracing is enabled we will be able to associate that metadata with that trace.
    // Metadata events should be used sparingly - as of 12/30/16 the Chrome Tracing
    // spec only supports thread+process metadata, so we should only expect to see metadata
    // events created when a new thread or process is created.
    assert(type == Metadata);

    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(p_high_resolution_clock::now().time_since_epoch()).count();
    auto threadID = int64_t(QThread::currentThreadId());

    addMetadata({
        id,
        name,
        type,
        timestamp,
        threadID,
        category,
        args,
        extra
    });
}

}
