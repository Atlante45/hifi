//
//  NullDisplayPlugin.cpp
//
//  Created by Bradley Austin Davis on 2014/04/13.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "NullDisplayPlugin.h"

#include <FramebufferCache.h>
#include <gpu/Context.h>
#include <gpu/Frame.h>
#include <ui-plugins/PluginContainer.h>
#include <QtGui/QImage>

const QString NullDisplayPlugin::NAME("NullDisplayPlugin");

glm::uvec2 NullDisplayPlugin::getRecommendedRenderSize() const {
    return glm::uvec2(100, 100);
}

void NullDisplayPlugin::submitFrame(const gpu::FramePointer& frame) {
    if (frame) {
        _gpuContext->consumeFrameUpdates(frame);
    }
}

QImage NullDisplayPlugin::getScreenshot(float aspectRatio) const {
    return QImage();
}

QImage NullDisplayPlugin::getSecondaryCameraScreenshot() const {
    return QImage();
}
