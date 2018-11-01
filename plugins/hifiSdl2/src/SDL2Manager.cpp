//
//  SDL2Manager.cpp
//  input-plugins/src/input-plugins
//
//  Created by Sam Gondelman on 6/5/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "SDL2Manager.h"

#include <QtCore/QCoreApplication>

#include <PerfStat.h>
#include <Preferences.h>
#include <SettingHandle.h>
#include <controllers/UserInputMapper.h>

static_assert(
    (int)controller::A == (int)SDL_CONTROLLER_BUTTON_A && (int)controller::B == (int)SDL_CONTROLLER_BUTTON_B &&
        (int)controller::X == (int)SDL_CONTROLLER_BUTTON_X && (int)controller::Y == (int)SDL_CONTROLLER_BUTTON_Y &&
        (int)controller::BACK == (int)SDL_CONTROLLER_BUTTON_BACK &&
        (int)controller::GUIDE == (int)SDL_CONTROLLER_BUTTON_GUIDE &&
        (int)controller::START == (int)SDL_CONTROLLER_BUTTON_START &&
        (int)controller::LS == (int)SDL_CONTROLLER_BUTTON_LEFTSTICK &&
        (int)controller::RS == (int)SDL_CONTROLLER_BUTTON_RIGHTSTICK &&
        (int)controller::LB == (int)SDL_CONTROLLER_BUTTON_LEFTSHOULDER &&
        (int)controller::RB == (int)SDL_CONTROLLER_BUTTON_RIGHTSHOULDER &&
        (int)controller::DU == (int)SDL_CONTROLLER_BUTTON_DPAD_UP &&
        (int)controller::DD == (int)SDL_CONTROLLER_BUTTON_DPAD_DOWN &&
        (int)controller::DL == (int)SDL_CONTROLLER_BUTTON_DPAD_LEFT &&
        (int)controller::DR == (int)SDL_CONTROLLER_BUTTON_DPAD_RIGHT && (int)controller::LX == (int)SDL_CONTROLLER_AXIS_LEFTX &&
        (int)controller::LY == (int)SDL_CONTROLLER_AXIS_LEFTY && (int)controller::RX == (int)SDL_CONTROLLER_AXIS_RIGHTX &&
        (int)controller::RY == (int)SDL_CONTROLLER_AXIS_RIGHTY && (int)controller::LT == (int)SDL_CONTROLLER_AXIS_TRIGGERLEFT &&
        (int)controller::RT == (int)SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
    "SDL2 equivalence: Enums and values from StandardControls.h are assumed to match enums from SDL_gamecontroller.h");

const char* SDL2Manager::NAME = "SDL2";
const char* SDL2Manager::SDL2_ID_STRING = "SDL2";

const bool DEFAULT_ENABLED = true;

SDL_JoystickID SDL2Manager::getInstanceId(SDL_GameController* controller) {
    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
    return SDL_JoystickInstanceID(joystick);
}

void SDL2Manager::init() {
}

QStringList SDL2Manager::getSubdeviceNames() {
    return _subdeviceNames;
}

void SDL2Manager::deinit() {
    _openJoysticks.clear();

    SDL_Quit();
}

bool SDL2Manager::activate() {
    // FIXME for some reason calling this code in the `init` function triggers a crash
    // on OSX in PR builds, but not on my local debug build.  Attempting a workaround by
    //
    static std::once_flag once;
    std::call_once(once, [&] {
        loadSettings();

        auto preferences = DependencyManager::get<Preferences>();
        static const QString SDL2_PLUGIN { "Game Controller" };
        {
            auto getter = [this]() -> bool { return _enabled; };
            auto setter = [this](bool value) {
                _enabled = value;
                saveSettings();
                emit deviceStatusChanged(getName(), isRunning());
            };
            auto preference = new CheckPreference(SDL2_PLUGIN, "Enabled", getter, setter);
            preferences->addPreference(preference);
        }

        bool initSuccess = (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) == 0);

        if (initSuccess) {
            int joystickCount = SDL_NumJoysticks();

            for (int i = 0; i < joystickCount; i++) {
                SDL_GameController* controller = SDL_GameControllerOpen(i);

                if (controller) {
                    SDL_JoystickID id = getInstanceId(controller);
                    if (!_openJoysticks.contains(id)) {
                        // Joystick* joystick = new Joystick(id, SDL_GameControllerName(controller), controller);
                        Joystick::Pointer joystick = std::make_shared<Joystick>(id, controller);
                        _openJoysticks[id] = joystick;
                        auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
                        userInputMapper->registerDevice(joystick);
                        auto name = SDL_GameControllerName(controller);
                        _subdeviceNames << name;
                        emit joystickAdded(joystick.get());
                        emit subdeviceConnected(getName(), name);
                    }
                }
            }

            _isInitialized = true;
        } else {
            qDebug() << "Error initializing SDL2 Manager";
        }
    });

    if (!_isInitialized) {
        return false;
    }

    InputPlugin::activate();
    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
    for (auto joystick : _openJoysticks) {
        userInputMapper->registerDevice(joystick);
        emit joystickAdded(joystick.get());
    }
    return true;
}

void SDL2Manager::deactivate() {
    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
    for (auto joystick : _openJoysticks) {
        userInputMapper->removeDevice(joystick->getDeviceID());
        emit joystickRemoved(joystick.get());
    }
    InputPlugin::deactivate();
}

const char* SETTINGS_ENABLED_KEY = "enabled";

void SDL2Manager::saveSettings() const {
    Settings settings;
    QString idString = getID();
    settings.beginGroup(idString);
    { settings.setValue(QString(SETTINGS_ENABLED_KEY), _enabled); }
    settings.endGroup();
}

void SDL2Manager::loadSettings() {
    Settings settings;
    QString idString = getID();
    settings.beginGroup(idString);
    {
        _enabled = settings.value(SETTINGS_ENABLED_KEY, QVariant(DEFAULT_ENABLED)).toBool();
        emit deviceStatusChanged(getName(), isRunning());
    }
    settings.endGroup();
}

bool SDL2Manager::isSupported() const {
    return true;
}

void SDL2Manager::pluginFocusOutEvent() {
    for (auto joystick : _openJoysticks) {
        joystick->focusOutEvent();
    }
}

void SDL2Manager::pluginUpdate(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) {
    if (!_enabled) {
        return;
    }

    if (_isInitialized) {
        auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
        for (auto joystick : _openJoysticks) {
            joystick->update(deltaTime, inputCalibrationData);
        }

        PerformanceTimer perfTimer("SDL2Manager::update");
        SDL_GameControllerUpdate();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_CONTROLLERAXISMOTION) {
                Joystick::Pointer joystick = _openJoysticks[event.caxis.which];
                if (joystick) {
                    joystick->handleAxisEvent(event.caxis);
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP) {
                Joystick::Pointer joystick = _openJoysticks[event.cbutton.which];
                if (joystick) {
                    joystick->handleButtonEvent(event.cbutton);
                }

#if 0
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
                    // this will either start or stop a global back event
                    QEvent::Type backType = (event.type == SDL_CONTROLLERBUTTONDOWN)
                    ? HFBackEvent::startType()
                    : HFBackEvent::endType();
                    HFBackEvent backEvent(backType);
                    
                    qApp->sendEvent(qApp, &backEvent);
                }
#endif

            } else if (event.type == SDL_CONTROLLERDEVICEADDED) {
                SDL_GameController* controller = SDL_GameControllerOpen(event.cdevice.which);
                SDL_JoystickID id = getInstanceId(controller);
                if (!_openJoysticks.contains(id)) {
                    // Joystick* joystick = new Joystick(id, SDL_GameControllerName(controller), controller);
                    Joystick::Pointer joystick = std::make_shared<Joystick>(id, controller);
                    _openJoysticks[id] = joystick;
                    userInputMapper->registerDevice(joystick);
                    QString name = SDL_GameControllerName(controller);
                    emit joystickAdded(joystick.get());
                    emit subdeviceConnected(getName(), name);
                    _subdeviceNames << name;
                }
            } else if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
                if (_openJoysticks.contains(event.cdevice.which)) {
                    Joystick::Pointer joystick = _openJoysticks[event.cdevice.which];
                    _openJoysticks.remove(event.cdevice.which);
                    userInputMapper->removeDevice(joystick->getDeviceID());
                    QString name = SDL_GameControllerName(joystick->getGameController());
                    emit joystickRemoved(joystick.get());
                    _subdeviceNames.removeOne(name);
                }
            }
        }
    }
}
