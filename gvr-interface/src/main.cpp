//
//  main.cpp
//  gvr-interface/src
//
//  Created by Stephen Birarda on 11/17/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "GVRInterface.h"
#include "GVRMainWindow.h"

int main(int argc, char* argv[]) {
    GVRInterface app(argc, argv);

    GVRMainWindow mainWindow;
#ifdef ANDROID
    mainWindow.showFullScreen();
#else
    mainWindow.showMaximized();
#endif

    app.setMainWindow(&mainWindow);

    return app.exec();
}