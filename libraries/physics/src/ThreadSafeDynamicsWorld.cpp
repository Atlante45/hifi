/*
 * Bullet Continuous Collision Detection and Physics Library
 * Copyright (c) 2003-2009 Erwin Coumans  http://bulletphysics.org
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it freely,
 * subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you
 * use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Copied and modified from btDiscreteDynamicsWorld.cpp by AndrewMeadows on 2014.11.12.
 * */

#include "ThreadSafeDynamicsWorld.h"

#include <LinearMath/btQuickprof.h>

#include "Profile.h"

ThreadSafeDynamicsWorld::ThreadSafeDynamicsWorld(btDispatcher* dispatcher, btBroadphaseInterface* pairCache,
                                                 btConstraintSolver* constraintSolver,
                                                 btCollisionConfiguration* collisionConfiguration) :
    btDiscreteDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration) {
}

int ThreadSafeDynamicsWorld::stepSimulationWithSubstepCallback(btScalar timeStep, int maxSubSteps, btScalar fixedTimeStep,
                                                               SubStepCallback onSubStep) {
    DETAILED_PROFILE_RANGE(simulation_physics, "stepWithCB");
    BT_PROFILE("stepSimulationWithSubstepCallback");
    int subSteps = 0;
    if (maxSubSteps) {
        // fixed timestep with interpolation
        m_fixedTimeStep = fixedTimeStep;
        m_localTime += timeStep;
        if (m_localTime >= fixedTimeStep) {
            subSteps = int(m_localTime / fixedTimeStep);
            m_localTime -= subSteps * fixedTimeStep;
        }
    } else {
        // variable timestep
        fixedTimeStep = timeStep;
        m_localTime = m_latencyMotionStateInterpolation ? 0 : timeStep;
        m_fixedTimeStep = 0;
        if (btFuzzyZero(timeStep)) {
            subSteps = 0;
            maxSubSteps = 0;
        } else {
            subSteps = 1;
            maxSubSteps = 1;
        }
    }

    if (subSteps) {
        // clamp the number of substeps, to prevent simulation grinding spiralling down to a halt
        int clampedSimulationSteps = (subSteps > maxSubSteps) ? maxSubSteps : subSteps;
        _numSubsteps += clampedSimulationSteps;
        ObjectMotionState::setWorldSimulationStep(_numSubsteps);

        saveKinematicState(fixedTimeStep * clampedSimulationSteps);

        {
            DETAILED_PROFILE_RANGE(simulation_physics, "applyGravity");
            BT_PROFILE("applyGravity");
            applyGravity();
        }

        for (int i = 0; i < clampedSimulationSteps; i++) {
            DETAILED_PROFILE_RANGE(simulation_physics, "substep");
            internalSingleStepSimulation(fixedTimeStep);
            onSubStep();
        }
    }

    // NOTE: We do NOT call synchronizeMotionStates() after each substep (to avoid multiple locks on the
    // object data outside of the physics engine).  A consequence of this is that the transforms of the
    // external objects only ever update at the end of the full step.

    // NOTE: We do NOT call synchronizeMotionStates() here.  Instead it is called by an external class
    // that knows how to lock threads correctly.

    clearForces();

    return subSteps;
}

// call this instead of non-virtual btDiscreteDynamicsWorld::synchronizeSingleMotionState()
void ThreadSafeDynamicsWorld::synchronizeMotionState(btRigidBody* body) {
    btAssert(body);
    btAssert(body->getMotionState());

    if (body->isKinematicObject()) {
        ObjectMotionState* objectMotionState = static_cast<ObjectMotionState*>(body->getMotionState());
        if (objectMotionState->hasInternalKinematicChanges()) {
            // this is a special case where the kinematic motion has been updated by an Action
            // so we supply the body's current transform to the MotionState
            objectMotionState->clearInternalKinematicChanges();
            body->getMotionState()->setWorldTransform(body->getWorldTransform());
        }
        return;
    }
    btTransform interpolatedTransform;
    btTransformUtil::integrateTransform(body->getInterpolationWorldTransform(), body->getInterpolationLinearVelocity(),
                                        body->getInterpolationAngularVelocity(),
                                        (m_latencyMotionStateInterpolation && m_fixedTimeStep)
                                            ? m_localTime - m_fixedTimeStep
                                            : m_localTime * body->getHitFraction(),
                                        interpolatedTransform);
    body->getMotionState()->setWorldTransform(interpolatedTransform);
}

void ThreadSafeDynamicsWorld::synchronizeMotionStates() {
    PROFILE_RANGE(simulation_physics, "SyncMotionStates");
    BT_PROFILE("syncMotionStates");
    _changedMotionStates.clear();

    // NOTE: m_synchronizeAllMotionStates is 'false' by default for optimization.
    // See PhysicsEngine::init() where we call _dynamicsWorld->setForceUpdateAllAabbs(false)
    if (m_synchronizeAllMotionStates) {
        // iterate  over all collision objects
        for (int i = 0; i < m_collisionObjects.size(); i++) {
            btCollisionObject* colObj = m_collisionObjects[i];
            btRigidBody* body = btRigidBody::upcast(colObj);
            if (body && body->getMotionState()) {
                synchronizeMotionState(body);
                _changedMotionStates.push_back(static_cast<ObjectMotionState*>(body->getMotionState()));
            }
        }
    } else {
        // iterate over all active rigid bodies
        // TODO? if this becomes a performance bottleneck we could derive our own SimulationIslandManager
        // that remembers a list of objects deactivated last step
        _activeStates.clear();
        _deactivatedStates.clear();
        for (int i = 0; i < m_nonStaticRigidBodies.size(); i++) {
            btRigidBody* body = m_nonStaticRigidBodies[i];
            ObjectMotionState* motionState = static_cast<ObjectMotionState*>(body->getMotionState());
            if (motionState) {
                if (body->isActive()) {
                    synchronizeMotionState(body);
                    _changedMotionStates.push_back(motionState);
                    _activeStates.insert(motionState);
                } else if (_lastActiveStates.find(motionState) != _lastActiveStates.end()) {
                    // this object was active last frame but is no longer
                    _deactivatedStates.push_back(motionState);
                }
            }
        }
    }
    _activeStates.swap(_lastActiveStates);
}

void ThreadSafeDynamicsWorld::saveKinematicState(btScalar timeStep) {
    DETAILED_PROFILE_RANGE(simulation_physics, "saveKinematicState");
    BT_PROFILE("saveKinematicState");
    for (int i = 0; i < m_nonStaticRigidBodies.size(); i++) {
        btRigidBody* body = m_nonStaticRigidBodies[i];
        if (body && body->isKinematicObject() && body->getActivationState() != ISLAND_SLEEPING) {
            if (body->getMotionState()) {
                btMotionState* motionState = body->getMotionState();
                ObjectMotionState* objectMotionState = static_cast<ObjectMotionState*>(motionState);
                objectMotionState->saveKinematicState(timeStep);
            } else {
                body->saveKinematicState(timeStep);
            }
        }
    }
}
