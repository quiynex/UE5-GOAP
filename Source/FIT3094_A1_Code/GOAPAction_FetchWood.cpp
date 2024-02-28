#include "GOAPAction_FetchWood.h"
#include "Ship.h"
#include "Resource.h"
#include "LevelGenerator.h"

GOAPAction_FetchWood::GOAPAction_FetchWood()
{
    ActionCost = 1.0f;
}

GOAPAction_FetchWood::~GOAPAction_FetchWood()
{
}

void GOAPAction_FetchWood::Reset()
{
    GOAPAction::Reset();

    CurrentActionTime = 0.0f;
}

bool GOAPAction_FetchWood::IsActionDone()
{
    if (CurrentActionTime >= TotalActionTime) return true;
    return false;
}

bool GOAPAction_FetchWood::SetupAction(AShip *Ship)
{
    if (Ship->AgentType == AShip::AGENT_TYPE::Woodcutter) {
        TotalActionTime = 1.333f;
        MaxTakeable = 2;
    }
    else {
        TotalActionTime = 2.0f;
        MaxTakeable = 1;
    }

    return true;
}

bool GOAPAction_FetchWood::CheckPreconditions(AShip *Ship, TMap<STATE_KEY, int> CurrentState)
{
    // check if effects are met in the current state
    if (int* ShipWood = CurrentState.Find(STATE_KEY::AgentWood)) {
        if (*ShipWood > 0) return true;
    }
    return false;

    // forward
    // if (CurrentState[STATE_KEY::AgentWood] == 0) return true;
    // return false;
}

void GOAPAction_FetchWood::ApplyEffects(AShip *Ship, TMap<STATE_KEY, int> &SuccessorState)
{
    // apply preconditions to successor state
    SuccessorState.Add(STATE_KEY::AgentWood, 0);

    // forward
    // SuccessorState.Add(STATE_KEY::AgentWood, 1);
}

bool GOAPAction_FetchWood::Tick(AShip *Ship, float DeltaTime)
{
    if (!IsValid(Target)) return false;
    CurrentActionTime += DeltaTime;
    if (CurrentActionTime < TotalActionTime) return true;

    AResource* TargetResource = Cast<AResource>(Target);
    if (TargetResource->ResourceType != GRID_TYPE::Wood || TargetResource->ResourceCount <= 0) return false;

    int NumTaken = (TargetResource->ResourceCount >= MaxTakeable)? MaxTakeable : TargetResource->ResourceCount;
    Ship->NumWood += NumTaken;
    TargetResource->ResourceCount -= NumTaken;

    if (TargetResource->ResourceCount <= 0) {
        Ship->GetWorld()->DestroyActor(Target);
        Target = nullptr;
    }

    return true;
}

bool GOAPAction_FetchWood::RequiresInRange()
{
    // AResource* TargetResource = Cast<AResource>(Target);
    // if (Ship->LevelGenerator->WorldArray[TargetResource->YPos][TargetResource->XPos]->ObjectAtLocation == Ship)
    //     return true;
    return true;
}

void GOAPAction_FetchWood::SetTarget(AShip* Ship)
{
    Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, GRID_TYPE::Wood, BUILDING_TYPE::Market);
}