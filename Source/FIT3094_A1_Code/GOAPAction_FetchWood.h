#pragma once

#include "GOAPAction.h"

class FIT3094_A1_CODE_API GOAPAction_FetchWood : public GOAPAction {
public:
    GOAPAction_FetchWood();
    ~GOAPAction_FetchWood();

    float TotalActionTime = 1.333f;
    float CurrentActionTime = 0.0f;
    int MaxTakeable = 2;

    virtual void Reset() override;
    virtual bool IsActionDone() override;
    virtual bool SetupAction(AShip* Ship) override;
    virtual bool CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState) override;
    virtual void ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState) override;
    virtual bool Tick(AShip* Ship, float DeltaTime) override;
    virtual bool RequiresInRange() override;
    virtual void SetTarget(AShip* Ship) override;
};