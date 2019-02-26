//
//  main.cpp
//  tools/skeleton-dump/src
//
//  Created by Anthony Thibault on 11/4/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

#include <QDebug>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <SharedUtil.h>

#include "SkeletonDumpApp.h"

int main(int argc, char* argv[]) {
    setupHifiApplication("Skeleton Dump App");

    SkeletonDumpApp app(argc, argv);
    return app.getReturnCode();
}
