#pragma once

#include <qml/OffscreenSurface.h>
#include <QtGui/QOpenGLFunctions_4_1_Core>
#include <QtGui/QWindow>
#include <functional>

class TestCase {
public:
    using QmlPtr = QSharedPointer<hifi::qml::OffscreenSurface>;
    using Builder = std::function<TestCase*(const QWindow*)>;
    TestCase(const QWindow* window) : _window(window) {}
    virtual void init();
    virtual void destroy();
    virtual void update();
    virtual void draw() = 0;
    static QUrl getTestResource(const QString& relativePath);

protected:
    QOpenGLFunctions_4_1_Core _glf;
    const QWindow* _window;
    std::function<void(uint32_t, void*)> _discardLamdba;
};
