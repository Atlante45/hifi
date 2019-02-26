//
//  UpdateSceneTask.cpp
//  render-utils/src/
//
//  Created by Sam Gateau on 6/21/2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "UpdateSceneTask.h"

#include <render/HighlightStage.h>
#include <render/SceneTask.h>
#include <render/TransitionStage.h>
#include "BackgroundStage.h"
#include "BloomStage.h"
#include "DeferredLightingEffect.h"
#include "HazeStage.h"
#include "LightStage.h"

void UpdateSceneTask::build(JobModel& task, const render::Varying& input, render::Varying& output) {
    task.addJob<LightStageSetup>("LightStageSetup");
    task.addJob<BackgroundStageSetup>("BackgroundStageSetup");
    task.addJob<HazeStageSetup>("HazeStageSetup");
    task.addJob<BloomStageSetup>("BloomStageSetup");
    task.addJob<render::TransitionStageSetup>("TransitionStageSetup");
    task.addJob<render::HighlightStageSetup>("HighlightStageSetup");

    task.addJob<DefaultLightingSetup>("DefaultLightingSetup");

    task.addJob<render::PerformSceneTransaction>("PerformSceneTransaction");
}
