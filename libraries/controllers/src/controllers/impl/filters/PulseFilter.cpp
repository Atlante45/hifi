//
//  Created by Bradley Austin Davis 2015/10/25
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "PulseFilter.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

using namespace controller;

const float PulseFilter::DEFAULT_LAST_EMIT_TIME = -::std::numeric_limits<float>::max();

float PulseFilter::apply(float value) const {
    float result = 0.0f;

    if (0.0f != value) {
        float now = secTimestampNow();
        float delta = now - _lastEmitTime;
        if (delta >= _interval) {
            _lastEmitTime = now;
            result = value;
        }
    } else if (_resetOnZero) {
        _lastEmitTime = DEFAULT_LAST_EMIT_TIME;
    }

    return result;
}

bool PulseFilter::parseParameters(const QJsonValue& parameters) {
    static const QString JSON_INTERVAL = QStringLiteral("interval");
    static const QString JSON_RESET = QStringLiteral("resetOnZero");
    if (parameters.isObject()) {
        auto obj = parameters.toObject();
        if (obj.contains(JSON_RESET)) {
            _resetOnZero = obj[JSON_RESET].toBool();
        }
    }
    return parseSingleFloatParameter(parameters, JSON_INTERVAL, _interval);
}
