//
//  Created by Anthony J. Thibault 2018/08/06
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AnimStats_h
#define hifi_AnimStats_h

#include <AnimContext.h>
#include <OffscreenQmlElement.h>

class AnimStats : public QQuickItem {
    Q_OBJECT
    HIFI_QML_DECL

    Q_PROPERTY(QStringList animAlphaValues READ animAlphaValues NOTIFY animAlphaValuesChanged)
    Q_PROPERTY(QStringList animVars READ animVars NOTIFY animVarsChanged)
    Q_PROPERTY(QStringList animStateMachines READ animStateMachines NOTIFY animStateMachinesChanged)
    Q_PROPERTY(QString positionText READ positionText NOTIFY positionTextChanged)
    Q_PROPERTY(QString rotationText READ rotationText NOTIFY rotationTextChanged)
    Q_PROPERTY(QString velocityText READ velocityText NOTIFY velocityTextChanged)

public:
    static AnimStats* getInstance();

    AnimStats(QQuickItem* parent = nullptr);

    void updateStats(bool force = false);

    QStringList animAlphaValues() const { return _animAlphaValues; }
    QStringList animVars() const { return _animVarsList; }
    QStringList animStateMachines() const { return _animStateMachines; }

    QString positionText() const { return _positionText; }
    QString rotationText() const { return _rotationText; }
    QString velocityText() const { return _velocityText; }

public slots:
    void forceUpdateStats() { updateStats(true); }

signals:

    void animAlphaValuesChanged();
    void animVarsChanged();
    void animStateMachinesChanged();
    void positionTextChanged();
    void rotationTextChanged();
    void velocityTextChanged();

private:
    QStringList _animAlphaValues;
    AnimContext::DebugAlphaMap _prevDebugAlphaMap; // alpha values from previous frame
    std::map<QString, qint64> _animAlphaValueChangedTimers; // last time alpha value has changed

    QStringList _animVarsList;
    std::map<QString, QString> _prevAnimVars; // anim vars from previous frame
    std::map<QString, qint64> _animVarChangedTimers; // last time animVar value has changed.

    QStringList _animStateMachines;

    QString _positionText;
    QString _rotationText;
    QString _velocityText;
};

#endif // hifi_AnimStats_h
