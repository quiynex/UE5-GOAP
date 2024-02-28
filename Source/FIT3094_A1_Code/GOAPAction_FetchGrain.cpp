#include "GOAPAction_FetchGrain.h"
#include "Ship.h"
#include "Resource.h"
#include "LevelGenerator.h"

GOAPAction_FetchGrain::GOAPAction_FetchGrain()
{
    ActionCost = 1.0f;
}

GOAPAction_FetchGrain::~GOAPAction_FetchGrain()
{
}

void GOAPAction_FetchGrain::Reset()
{
    GOAPAction::Reset();

    ActionCost = 1.0f;
    TotalActionTime = 3.333f;
    CurrentActionTime = 0.0f;
}

bool GOAPAction_FetchGrain::IsActionDone()
{
    if (CurrentActionTime >= TotalActionTime) return true;
    return false;
}

bool GOAPAction_FetchGrain::SetupAction(AShip *Ship)
{
    if (Ship->AgentType == AShip::AGENT_TYPE::Farmer) {
        return true;
    }
    return false;
}

bool GOAPAction_FetchGrain::CheckPreconditions(AShip *Ship, TMap<STATE_KEY, int> CurrentState)
{
    // check if effects are met in the current state
    if (int* ShipGrain = CurrentState.Find(STATE_KEY::AgentGrain)) {
        if (*ShipGrain > 0) return true;
    }
    return false;
}

void GOAPAction_FetchGrain::ApplyEffects(AShip *Ship, TMap<STATE_KEY, int> &SuccessorState)
{
    // apply preconditions to successor state
    SuccessorState.Add(STATE_KEY::AgentGrain, 0);
}

bool GOAPAction_FetchGrain::Tick(AShip *Ship, float DeltaTime)
{
    if (!IsValid(Target)) return false;
    CurrentActionTime += DeltaTime;
    if (CurrentActionTime < TotalActionTime) return true;

    AResource* TargetResource = Cast<AResource>(Target);
    if (TargetResource->ResourceType != GRID_TYPE::Grain || TargetResource->ResourceCount <= 0) return false;

    int NumTaken = (TargetResource->ResourceCount >= 2)? 2 : TargetResource->ResourceCount;
    Ship->NumGrain += NumTaken;
    TargetResource->ResourceCount -= NumTaken;

    if (TargetResource->ResourceCount <= 0) {
        Ship->GetWorld()->DestroyActor(Target);
        Target = nullptr;
    }

    return true;
}

bool GOAPAction_FetchGrain::RequiresInRange()
{
    return true;
}

void GOAPAction_FetchGrain::SetTarget(AShip *Ship)
{
    Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, GRID_TYPE::Grain, BUILDING_TYPE::Market);
}
