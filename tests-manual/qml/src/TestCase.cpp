#include "TestCase.h"

#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>

void TestCase::destroy() {
}
void TestCase::update() {
}

void TestCase::init() {
    _glf.initializeOpenGLFunctions();
    _discardLamdba = hifi::qml::OffscreenSurface::getDiscardLambda();
}

QUrl TestCase::getTestResource(const QString& relativePath) {
    static QString dir;
    if (dir.isEmpty()) {
        QDir path(__FILE__);
        path.cdUp();
        dir = path.cleanPath(path.absoluteFilePath("../")) + "/";
        qDebug() << "Resources Path: " << dir;
    }
    return QUrl::fromLocalFile(dir + relativePath);
}
