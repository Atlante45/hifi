//
//  NetworkingScriptingInterface.cpp
//  interface/src/scripting
//
//  Created by Clement Brisset on 6/22/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "NetworkingScriptingInterface.h"

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

NetworkingScriptingInterface::NetworkingScriptingInterface(QObject* parent) : QObject(parent) {
    _outputLocation = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/captured-packets";
}

bool NetworkingScriptingInterface::startRecording() {
    if (_packetRecorder) {
        return false;
    }

    QDir dir;
    dir.mkpath(_outputLocation);
    _packetRecorder.reset(new PacketRecorder(this));
    _packetRecorder->setOutputFileName(_outputLocation + "/interface-capture.pcap");

    return _packetRecorder->startRecording();
}

void NetworkingScriptingInterface::stopRecording() {
    if (!_packetRecorder) {
        return;
    }

    _packetRecorder->stopRecording();
    _packetRecorder.reset();
}
