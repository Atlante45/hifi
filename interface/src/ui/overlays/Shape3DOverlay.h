//
//  Shape3DOverlay.h
//  interface/src/ui/overlays
//
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_Shape3DOverlay_h
#define hifi_Shape3DOverlay_h

#include "Volume3DOverlay.h"

#include <GeometryCache.h>

class Shape3DOverlay : public Volume3DOverlay {
    Q_OBJECT

public:
    static QString const TYPE;
    virtual QString getType() const override { return TYPE; }

    Shape3DOverlay() {}
    Shape3DOverlay(const Shape3DOverlay* shape3DOverlay);

    virtual void render(RenderArgs* args) override;
    virtual const render::ShapeKey getShapeKey() override;

    virtual Shape3DOverlay* createClone() const override;

    void setProperties(const QVariantMap& properties) override;
    QVariant getProperty(const QString& property) override;

    virtual scriptable::ScriptableModelBase getScriptableModel() override;

protected:
    Transform evalRenderTransform() override;

private:
    GeometryCache::Shape _shape { GeometryCache::Hexagon };
};

#endif // hifi_Shape3DOverlay_h
