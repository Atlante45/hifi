//
//  LeapMotionProvider.cpp
//
//  Created by David Rowe on 15 Jun 2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <mutex>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QtPlugin>

#include <plugins/InputPlugin.h>
#include <plugins/RuntimePlugin.h>

#include "LeapMotionPlugin.h"

class LeapMotionProvider : public QObject, public InputProvider {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID InputProvider_iid FILE "plugin.json")
    Q_INTERFACES(InputProvider)

public:
    LeapMotionProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~LeapMotionProvider() {}

    virtual InputPluginList getInputPlugins() override {
        static std::once_flag once;
        std::call_once(once, [&] {
            InputPluginPointer plugin(new LeapMotionPlugin());
            if (plugin->isSupported()) {
                _inputPlugins.push_back(plugin);
            }
        });
        return _inputPlugins;
    }

    virtual void destroyInputPlugins() override { _inputPlugins.clear(); }

private:
    InputPluginList _inputPlugins;
};

#include "LeapMotionProvider.moc"
