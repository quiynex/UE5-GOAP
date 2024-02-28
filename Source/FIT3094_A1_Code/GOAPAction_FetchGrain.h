#pragma once

#include "GOAPAction.h"

class FIT3094_A1_CODE_API GOAPAction_FetchGrain : public GOAPAction {
public:
    GOAPAction_FetchGrain();
    ~GOAPAction_FetchGrain();

    float TotalActionTime = 3.333f;
    float CurrentActionTime = 0.0f;

    virtual void Reset() override;
    virtual bool IsActionDone() override;
    virtual bool SetupAction(AShip* Ship) override;
    virtual bool CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState) override;
    virtual void ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState) override;
    virtual bool Tick(AShip* Ship, float DeltaTime) override;
    virtual bool RequiresInRange() override;
    virtual void SetTarget(AShip* Ship) override;

};