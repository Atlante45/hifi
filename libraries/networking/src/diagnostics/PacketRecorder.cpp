//
//  PacketRecorder.cpp
//  libraries/networking/src/diagnostics
//
//  Created by Clement Brisset on 6/22/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "PacketRecorder.h"

#include <algorithm>

#include <QDebug>
#include <QFile>

#include "../LimitedNodeList.h"

#ifdef Q_OS_WIN
static const QString DUMPCAP_BINARY_NAME { "dumpcap.exe" };
#else
static const QString DUMPCAP_BINARY_NAME { "dumpcap" };
#endif

PacketRecorder::~PacketRecorder() {
    if (_dumpcapProcess) {
        stopRecording();
    }
}

QString PacketRecorder::getDumpcapPath() const {
    return _dumpcapLocation + DUMPCAP_BINARY_NAME;
}

bool PacketRecorder::isDumpcapInstalled() const {
    return QFile::exists(getDumpcapPath());
}

bool PacketRecorder::canRecord() const {
    return isDumpcapInstalled() && !_outputFileName.isEmpty();
}

bool PacketRecorder::isRecording() const {
    return _dumpcapProcess && _dumpcapProcess->state() == QProcess::Running;
}

int PacketRecorder::fetchNetworkInterfaceIndexForAddress(const QHostAddress& address) const {
    QProcess process;
    QStringList arguments;
    arguments << "-DM";
    process.start(getDumpcapPath(), arguments, QIODevice::ReadOnly);
    process.waitForFinished();
    if (process.exitStatus() != QProcess::NormalExit &&
        process.state() != QProcess::NotRunning) {
        process.kill();
        return -1;
    }

    auto output = process.readAll();
    auto lines = output.split('\n');
    for (auto line : lines) {
        auto items = line.split('\t');
        if (items.size() == 7) {
            auto addresses = items[4].split(',');
            if (addresses.contains(address.toString().toUtf8())) {
                bool ok = false;
                int index = items[0].split('.')[0].toInt(&ok);
                if (ok) {
                    return index;
                }
            }
        }
    }
    return -1;
}

bool PacketRecorder::startRecording() {
    assert(!_dumpcapProcess);

    if (!canRecord()) {
        return false;
    }

    auto nodeList = DependencyManager::get<LimitedNodeList>();
    auto address = nodeList->getLocalSockAddr().getAddress();

    int interfaceIndex = fetchNetworkInterfaceIndexForAddress(address);
    if (interfaceIndex == -1) {
        qDebug() << "Could not find network interface index.";
        return false;
    }

    QStringList arguments;

    arguments << "-i" << QString::number(interfaceIndex); // interface
    auto port = QString::number(nodeList->getSocketLocalPort());
    arguments << "-f" << ("src port " + port + " || dst port " + port);
    arguments << "-w" << _outputFileName; // output

    if (_duration.count() > 0 && _files > 0) {
        arguments << "-b" << ("duration:" + QString::number(_duration.count())); // capture length
        arguments << "-b" << ("files:" + QString::number(_files)); // number of files
    }

    _dumpcapProcess.reset(new QProcess());

    QObject::connect(_dumpcapProcess.get(), &QProcess::started,
                     this, &PacketRecorder::onStarted);
    QObject::connect(_dumpcapProcess.get(), &QProcess::stateChanged,
                     this, &PacketRecorder::onStateChanged);
    QObject::connect(_dumpcapProcess.get(), &QProcess::readyReadStandardOutput,
                     this, &PacketRecorder::onStdout);
    QObject::connect(_dumpcapProcess.get(), &QProcess::readyReadStandardError,
                     this, &PacketRecorder::onStderr);
    QObject::connect(_dumpcapProcess.get(), &QProcess::errorOccurred,
                     this, &PacketRecorder::onError);
    QObject::connect(_dumpcapProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, &PacketRecorder::onFinished);

    qDebug() << "Launching" << getDumpcapPath() << "with:";
    qDebug() << "    Args:" << arguments.join(" ");
    _dumpcapProcess->start(getDumpcapPath(), arguments, QIODevice::ReadOnly);

    return true;
}

void PacketRecorder::stopRecording() {
    assert(_dumpcapProcess);
    if (!_dumpcapProcess) {
        return;
    }

    _dumpcapProcess->terminate();
    _dumpcapProcess->waitForFinished(1000);
    _dumpcapProcess->kill();
    _dumpcapProcess.reset();
}

void PacketRecorder::onStarted() const {
    qDebug() << "[dumpcap] Started";
}

void PacketRecorder::onStateChanged(QProcess::ProcessState newState) const {
    qDebug() << "[dumpcap] State changed:" << newState;
}

void PacketRecorder::onStdout() const {
    const QRegExp re{ "^Packets: \\d*$" };
    auto logLines = _dumpcapProcess->readAllStandardOutput().trimmed().split('\n');
    auto it = std::remove_if(std::begin(logLines), std::end(logLines), [&](const QByteArray& value) {
        return value.isEmpty() || re.exactMatch(value);
    });
    logLines.erase(it, std::end(logLines));

    if (!logLines.isEmpty()) {
        const QByteArray PREFIX = "[dumpcap] [stdout] ";
        qDebug() << qPrintable(PREFIX + logLines.join('\n' + PREFIX));
    }
}

void PacketRecorder::onStderr() const {
    const QRegExp re { "^Packets: \\d*$" };
    auto logLines = _dumpcapProcess->readAllStandardError().trimmed().split('\n');
    auto it = std::remove_if(std::begin(logLines), std::end(logLines), [&](const QByteArray& value) {
        return value.isEmpty() || re.exactMatch(value);
    });
    logLines.erase(it, std::end(logLines));

    if (!logLines.isEmpty()) {
        const QByteArray PREFIX = "[dumpcap] [stderr] ";
        qDebug() << qPrintable(PREFIX + logLines.join('\n' + PREFIX));
    }
}

void PacketRecorder::onError(QProcess::ProcessError error) const {
    qDebug() << "[dumpcap] Error:" << error;
}

void PacketRecorder::onFinished(int exitCode, QProcess::ExitStatus exitStatus) const {
    qDebug() << "[dumpcap] ExitCode:" << exitCode << ", ExitStatus:" << exitStatus;
}
