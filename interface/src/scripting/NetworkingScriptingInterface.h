//
//  NetworkingScriptingInterface.h
//  interface/src/scripting
//
//  Created by Clement Brisset on 6/22/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_NetworkingScriptingInterface_h
#define hifi_NetworkingScriptingInterface_h

#include <memory>

#include <QObject>

#include <diagnostics/PacketRecorder.h>

class NetworkingScriptingInterface : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString outputLocation READ getOutputLocation WRITE setOutputLocation);
public:
    NetworkingScriptingInterface(QObject* parent);

    QString getOutputLocation() const { return _outputLocation;  }
    void setOutputLocation(QString outputLocation) { _outputLocation = _outputLocation; }

    Q_INVOKABLE bool startRecording();
    Q_INVOKABLE void stopRecording();

private:
    std::unique_ptr<PacketRecorder> _packetRecorder;

    QString _dumpcapLocation;
    QString _outputLocation;
};

#endif // hifi_NetworkingScriptingInterface_h
