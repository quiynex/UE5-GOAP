#include "GOAPAction_Build.h"
#include "Ship.h"
#include "LevelGenerator.h"

GOAPAction_Build::GOAPAction_Build()
{
    ActionCost = 1.0f;
}

GOAPAction_Build::GOAPAction_Build(BUILDING_TYPE BuildingType)
{
    ActionCost = 1.0f;
    Type = BuildingType;
    TotalActionTime = ABuilding::GetTimeRequired(Type);
}

GOAPAction_Build::~GOAPAction_Build()
{
}

void GOAPAction_Build::Reset()
{
    GOAPAction::Reset();

    CurrentActionTime = 0.0f;
}

bool GOAPAction_Build::IsActionDone()
{
    if (CurrentActionTime >= TotalActionTime) return true;
    return false;
}

bool GOAPAction_Build::SetupAction(AShip *Ship)
{
    if (Ship->AgentType == AShip::AGENT_TYPE::Builder) {
        return true;
    }
    return false;
}

bool GOAPAction_Build::CheckPreconditions(AShip *Ship, TMap<STATE_KEY, int> CurrentState)
{
    // check if effects are met in the current state
    if (int* Points = CurrentState.Find(STATE_KEY::NumPoints)) {
        if (*Points >= Ship->GetWorldState()[STATE_KEY::NumPoints] + ABuilding::GetPointsProvided(Type)) return true;
    }
    return false;
}

void GOAPAction_Build::ApplyEffects(AShip *Ship, TMap<STATE_KEY, int> &SuccessorState)
{
    // apply preconditions to successor state
    SuccessorState.Add(STATE_KEY::NumPoints, Ship->GetWorldState()[STATE_KEY::NumPoints]);
}

bool GOAPAction_Build::Tick(AShip *Ship, float DeltaTime)
{
    if (!IsValid(Target)) return false;
    CurrentActionTime += DeltaTime;
    if (CurrentActionTime < TotalActionTime) return true;

    AResource* TargetResource = Cast<AResource>(Target);
    if (TargetResource->ResourceType != GRID_TYPE::BuildingSlot) return false;

    auto ResourceRequirement = ABuilding::GetResourceCost(Type);
    auto CurrentWorldState = Ship->GetWorldState();
    if (
        CurrentWorldState[STATE_KEY::TotalWood] < ResourceRequirement.X
        || CurrentWorldState[STATE_KEY::TotalStone] < ResourceRequirement.Y
        || CurrentWorldState[STATE_KEY::TotalGrain] < ResourceRequirement.Z
    ) return false;

    Ship->LevelGenerator->TotalWood -= ResourceRequirement.X;
    Ship->LevelGenerator->TotalStone -= ResourceRequirement.Y;
    Ship->LevelGenerator->TotalGrain -= ResourceRequirement.Z;
    Ship->LevelGenerator->Points += ABuilding::GetPointsProvided(Type);

    // Spawn building
    Ship->GetWorld()->SpawnActor<ABuilding>(Target->GetActorLocation(), Target->GetActorRotation());
    Ship->GetWorld()->DestroyActor(Target);
    Target = nullptr;

    return true;
}

bool GOAPAction_Build::RequiresInRange()
{
    return true;
}

void GOAPAction_Build::SetTarget(AShip *Ship)
{
    Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, GRID_TYPE::BuildingSlot, BUILDING_TYPE::Market);
}
