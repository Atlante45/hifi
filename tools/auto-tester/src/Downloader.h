//
//  Downloader.h
//
//  Created by Nissim Hadar on 1 Mar 2018.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_downloader_h
#define hifi_downloader_h

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>

class Downloader : public QObject {
    Q_OBJECT
public:
    explicit Downloader(QUrl fileURL, QObject* parent = 0);

    QByteArray downloadedData() const;

signals:
    void downloaded();

private slots:
    void fileDownloaded(QNetworkReply* pReply);

private:
    QNetworkAccessManager _networkAccessManager;
    QByteArray _downloadedData;
};

#endif // hifi_downloader_h