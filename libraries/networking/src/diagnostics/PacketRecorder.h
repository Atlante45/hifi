//
//  PacketRecorder.h
//  libraries/networking/src/diagnostics
//
//  Created by Clement Brisset on 6/22/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#ifndef hifi_PacketRecorder_h
#define hifi_PacketRecorder_h

#include <chrono>
#include <memory>

#include <QProcess>

#include <DependencyManager.h>

class QHostAddress;

#ifdef Q_OS_WIN
const QString DUMPCAT_INSTALL_DIRECTORY { "C:/Program Files/Wireshark/" };
#else
const QString DUMPCAT_INSTALL_DIRECTORY { "/Applications/Wireshark.app/Contents/MacOS/" };
#endif

const std::chrono::minutes DEFAULT_CAPTURE_TIME { 5 };
const int DEFAULT_FILES_NUMBER { 5 };

class PacketRecorder : public QObject {
    Q_OBJECT

    using ProcessPointer = std::unique_ptr<QProcess>;

public:
    PacketRecorder(QObject* parent) : QObject(parent) {}
    ~PacketRecorder();

    QString getDumpcapPath() const;
    bool isDumpcapInstalled() const;
    bool canRecord() const;

    QString getDumpcapLocation() const { return _dumpcapLocation; }
    void setDumpcapLocation(const QString& dumpcapLocation) { _dumpcapLocation = dumpcapLocation; }

    void setOutputFileName(const QString& name) { _outputFileName = name; }
    const QString& getOutputFileName() const { return _outputFileName; }

    bool isRecording() const;

    bool startRecording();
    void stopRecording();

private slots:
    void onStarted() const;
    void onStateChanged(QProcess::ProcessState newState) const;
    void onStdout() const;
    void onStderr() const;
    void onError(QProcess::ProcessError error) const;
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus) const;

private:
    int fetchNetworkInterfaceIndexForAddress(const QHostAddress& address) const;

    ProcessPointer _dumpcapProcess;

    QString _dumpcapLocation { DUMPCAT_INSTALL_DIRECTORY };
    QString _outputFileName;

    std::chrono::seconds _duration { DEFAULT_CAPTURE_TIME };
    int _files { DEFAULT_FILES_NUMBER };
};

#endif // hifi_PacketRecorder_h
