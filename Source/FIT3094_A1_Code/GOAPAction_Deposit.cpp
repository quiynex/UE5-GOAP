#include "GOAPAction_Deposit.h"
#include "Ship.h"
#include "Resource.h"
#include "LevelGenerator.h"

GOAPAction_Deposit::GOAPAction_Deposit()
{
    ActionCost = 1.0f;
}

GOAPAction_Deposit::~GOAPAction_Deposit()
{
}

void GOAPAction_Deposit::Reset()
{
    GOAPAction::Reset();

    ActionCost = 1.0f;
    CurrentActionTime = 0.0f;
    TotalActionTime = 1.0f;
}

bool GOAPAction_Deposit::IsActionDone()
{
    if (CurrentActionTime >= TotalActionTime) return true;
    return false;
}

bool GOAPAction_Deposit::SetupAction(AShip *Ship)
{
    // if (Ship->AgentType == AShip::AGENT_TYPE::Builder) return false;
    return true;
}

bool GOAPAction_Deposit::CheckPreconditions(AShip *Ship, TMap<STATE_KEY, int> CurrentState)
{
    // check if effects are met in the current state
    if (!CurrentState.Contains(STATE_KEY::AgentWood))
        CurrentState.Add(STATE_KEY::AgentWood, 0);

    if (!CurrentState.Contains(STATE_KEY::AgentGrain))
        CurrentState.Add(STATE_KEY::AgentGrain, 0);

    if (!CurrentState.Contains(STATE_KEY::AgentStone))
        CurrentState.Add(STATE_KEY::AgentStone, 0);

    if (
        CurrentState[STATE_KEY::AgentWood] == 0
        && CurrentState[STATE_KEY::AgentGrain] == 0
        && CurrentState[STATE_KEY::AgentStone] == 0
    ) return true;

    return false;
}

void GOAPAction_Deposit::ApplyEffects(AShip *Ship, TMap<STATE_KEY, int> &SuccessorState)
{
    // apply preconditions to successor state
    SuccessorState.Add(STATE_KEY::AgentGrain, Ship->NumGrain);
    SuccessorState.Add(STATE_KEY::AgentWood, Ship->NumWood);
    SuccessorState.Add(STATE_KEY::AgentStone, Ship->NumStone);
}

bool GOAPAction_Deposit::Tick(AShip *Ship, float DeltaTime)
{
    if (!IsValid(Target)) return false;
    CurrentActionTime += DeltaTime;
    if (CurrentActionTime < TotalActionTime) return true;

    AResource* TargetResource = Cast<AResource>(Target);
    if (TargetResource->ResourceType != GRID_TYPE::Home) return false;

    Ship->LevelGenerator->TotalWood += Ship->NumWood;
    Ship->LevelGenerator->TotalStone += Ship->NumStone;
    Ship->LevelGenerator->TotalGrain += Ship->NumGrain;
    Ship->LevelGenerator->Points += Ship->NumWood + 2 *  Ship->NumStone + 3 * Ship->NumGrain;

    Ship->NumWood = 0;
    Ship->NumGrain = 0;
    Ship->NumStone = 0;

    return true;
}

bool GOAPAction_Deposit::RequiresInRange()
{
    // AResource* TargetResource = Cast<AResource>(Target);
    // if (Ship->LevelGenerator->WorldArray[TargetResource->YPos][TargetResource->XPos]->ObjectAtLocation == Ship)
    //     return true;
    return true;
}

void GOAPAction_Deposit::SetTarget(AShip *Ship)
{
    Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, GRID_TYPE::Home, BUILDING_TYPE::Market);
}
