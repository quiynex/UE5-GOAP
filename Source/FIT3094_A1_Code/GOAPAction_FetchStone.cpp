#include "GOAPAction_FetchStone.h"
#include "Ship.h"
#include "Resource.h"
#include "LevelGenerator.h"

GOAPAction_FetchStone::GOAPAction_FetchStone()
{
    ActionCost = 1.0f;
}

GOAPAction_FetchStone::~GOAPAction_FetchStone()
{
}

void GOAPAction_FetchStone::Reset()
{
    GOAPAction::Reset();

    ActionCost = 1.0f;
    TotalActionTime = 2.0f;
    CurrentActionTime = 0.0f;
}

bool GOAPAction_FetchStone::IsActionDone()
{
    if (CurrentActionTime >= TotalActionTime) return true;
    return false;
}

bool GOAPAction_FetchStone::SetupAction(AShip *Ship)
{
    if (Ship->AgentType == AShip::AGENT_TYPE::Stonemason) {
        return true;
    }
    return false;
}

bool GOAPAction_FetchStone::CheckPreconditions(AShip *Ship, TMap<STATE_KEY, int> CurrentState)
{
    // check if effects are met in the current state
    if (int* ShipStone = CurrentState.Find(STATE_KEY::AgentStone)) {
        if (*ShipStone > 0) return true;
    }
    return false;
}

void GOAPAction_FetchStone::ApplyEffects(AShip *Ship, TMap<STATE_KEY, int> &SuccessorState)
{
    // apply preconditions to successor state
    SuccessorState.Add(STATE_KEY::AgentStone, 0);
}

bool GOAPAction_FetchStone::Tick(AShip *Ship, float DeltaTime)
{
    if (!IsValid(Target)) return false;
    CurrentActionTime += DeltaTime;
    if (CurrentActionTime < TotalActionTime) return true;

    AResource* TargetResource = Cast<AResource>(Target);
    if (TargetResource->ResourceType != GRID_TYPE::Stone || TargetResource->ResourceCount <= 0) return false;

    int NumTaken = (TargetResource->ResourceCount >= 2)? 2 : TargetResource->ResourceCount;
    Ship->NumStone += NumTaken;
    TargetResource->ResourceCount -= NumTaken;

    if (TargetResource->ResourceCount <= 0) {
        Ship->GetWorld()->DestroyActor(Target);
        Target = nullptr;
    }

    return true;
}

bool GOAPAction_FetchStone::RequiresInRange()
{
    return true;
}

void GOAPAction_FetchStone::SetTarget(AShip *Ship)
{
    Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, GRID_TYPE::Stone, BUILDING_TYPE::Market);
}
