//
//  Created by Ryan Huffman on 2016-12-14
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#ifndef HIFI_PROFILE_
#define HIFI_PROFILE_

#include "Trace.h"
#include "SharedUtil.h"

class Duration {
public:
    Duration(tracing::Category category, const QString& name, uint32_t argbColor = 0xff0000ff, uint64_t payload = 0, const QVariantMap& args = QVariantMap());
    ~Duration();

    static uint64_t beginRange(tracing::Category category, const char* name, uint32_t argbColor);
    static void endRange(tracing::Category category, uint64_t rangeId);

private:
    QString _name;
    tracing::Category _category;
};


inline void syncBegin(tracing::Category category, const QString& name, const QString& id, const QVariantMap& args = QVariantMap(), const QVariantMap& extra = QVariantMap()) {
    if (tracing::isTracingEnabled(category)) {
        tracing::traceEvent(category, name, tracing::DurationBegin, id, args, extra);
    }
}


inline void syncEnd(tracing::Category category, const QString& name, const QString& id, const QVariantMap& args = QVariantMap(), const QVariantMap& extra = QVariantMap()) {
    if (tracing::isTracingEnabled(category)) {
        tracing::traceEvent(category, name, tracing::DurationEnd, id, args, extra);
    }
}

inline void asyncBegin(tracing::Category category, const QString& name, const QString& id, const QVariantMap& args = QVariantMap(), const QVariantMap& extra = QVariantMap()) {
    if (tracing::isTracingEnabled(category)) {
        tracing::traceEvent(category, name, tracing::AsyncNestableStart, id, args, extra);
    }
}


inline void asyncEnd(tracing::Category category, const QString& name, const QString& id, const QVariantMap& args = QVariantMap(), const QVariantMap& extra = QVariantMap()) {
    if (tracing::isTracingEnabled(category)) {
        tracing::traceEvent(category, name, tracing::AsyncNestableEnd, id, args, extra);
    }
}

inline void instant(tracing::Category category, const QString& name, const QString& scope = "t", const QVariantMap& args = QVariantMap(), QVariantMap extra = QVariantMap()) {
    if (tracing::isTracingEnabled(category)) {
        extra["s"] = scope;
        tracing::traceEvent(category, name, tracing::Instant, "", args, extra);
    }
}

inline void counter(tracing::Category category, const QString& name, const QVariantMap& args, const QVariantMap& extra = QVariantMap()) {
    if (tracing::isTracingEnabled(category)) {
        tracing::traceEvent(category, name, tracing::Counter, "", args, extra);
    }
}

inline void metadata(const QString& metadataType, const QVariantMap& args) {
    tracing::traceMetadata(tracing::metadata, metadataType, tracing::Metadata, "", args, QVariantMap());
}

#define PROFILE_RANGE(category, name) Duration profileRangeThis(tracing::category, name);
#define PROFILE_RANGE_EX(category, name, argbColor, payload, ...) Duration profileRangeThis(tracing::category, name, argbColor, (uint64_t)payload, ##__VA_ARGS__);
#define PROFILE_RANGE_BEGIN(category, rangeId, name, argbColor) rangeId = Duration::beginRange(tracing::category, name, argbColor)
#define PROFILE_RANGE_END(category, rangeId) Duration::endRange(tracing::category, rangeId)
#define PROFILE_SYNC_BEGIN(category, name, id, ...) syncBegin(tracing::category, name, id, ##__VA_ARGS__);
#define PROFILE_SYNC_END(category, name, id, ...) syncEnd(tracing::category, name, id, ##__VA_ARGS__);
#define PROFILE_ASYNC_BEGIN(category, name, id, ...) asyncBegin(tracing::category, name, id, ##__VA_ARGS__);
#define PROFILE_ASYNC_END(category, name, id, ...) asyncEnd(tracing::category, name, id, ##__VA_ARGS__);
#define PROFILE_COUNTER_IF_CHANGED(category, name, type, value) { static type lastValue = 0; type newValue = value;  if (newValue != lastValue) { counter(tracing::category, name, { { name, newValue }}); lastValue = newValue; } }
#define PROFILE_COUNTER(category, name, ...) counter(tracing::category, name, ##__VA_ARGS__);
#define PROFILE_INSTANT(category, name, ...) instant(tracing::category, name, ##__VA_ARGS__);
#define PROFILE_SET_THREAD_NAME(threadName) metadata("thread_name", { { "name", threadName } });

#define SAMPLE_PROFILE_RANGE(chance, category, name, ...) if (randFloat() <= chance) { PROFILE_RANGE(category, name); }
#define SAMPLE_PROFILE_RANGE_EX(chance, category, name, ...) if (randFloat() <= chance) { PROFILE_RANGE_EX(category, name, argbColor, payload, ##__VA_ARGS__); }
#define SAMPLE_PROFILE_COUNTER(chance, category, name, ...) if (randFloat() <= chance) { PROFILE_COUNTER(category, name, ##__VA_ARGS__); }
#define SAMPLE_PROFILE_INSTANT(chance, category, name, ...) if (randFloat() <= chance) { PROFILE_INSTANT(category, name, ##__VA_ARGS__); }

// uncomment WANT_DETAILED_PROFILING definition to enable profiling in high-frequency contexts
//#define WANT_DETAILED_PROFILING
#ifdef WANT_DETAILED_PROFILING
#define DETAILED_PROFILE_RANGE(category, name) Duration profileRangeThis(tracing::category, name);
#define DETAILED_PROFILE_RANGE_EX(category, name, argbColor, payload, ...) Duration profileRangeThis(tracing::category, name, argbColor, (uint64_t)payload, ##__VA_ARGS__);
#else // WANT_DETAILED_PROFILING
#define DETAILED_PROFILE_RANGE(category, name) ; // no-op
#define DETAILED_PROFILE_RANGE_EX(category, name, argbColor, payload, ...) ; // no-op
#endif // WANT_DETAILED_PROFILING

#endif
