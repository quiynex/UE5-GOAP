// Fill out your copyright notice in the Description page of Project Settings.
#include "Ship.h"

#include "GOAPPlanner.h"
#include "LevelGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "GOAPAction_FetchWood.h"
#include "GOAPAction_FetchGrain.h"
#include "GOAPAction_FetchStone.h"
#include "GOAPAction_Deposit.h"
#include "GOAPAction_Build.h"

// Sets default values
AShip::AShip()
{
 	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	MoveSpeed = 500;
	Tolerance = MoveSpeed / 20;
	GoalNode = nullptr;

	ActionStateMachine = new StateMachine<ACTOR_STATES, AShip>(this, State_Nothing);
	ActionStateMachine->RegisterState(State_Idle, &AShip::OnIdleEnter, &AShip::OnIdleTick, &AShip::OnIdleExit);
	ActionStateMachine->RegisterState(State_Move, &AShip::OnMoveEnter, &AShip::OnMoveTick, &AShip::OnMoveExit);
	ActionStateMachine->RegisterState(State_Action, &AShip::OnActionEnter, &AShip::OnActionTick, &AShip::OnActionExit);
	ActionStateMachine->ChangeState(State_Idle);

	MaxIdleTime = 0.5;
	CurrentIdleTime = 0;
}

// Called when the game starts or when spawned
void AShip::BeginPlay()
{
	Super::BeginPlay();
	LevelGenerator = Cast<ALevelGenerator>(UGameplayStatics::GetActorOfClass(GetWorld(), ALevelGenerator::StaticClass()));
	GetComponents(UStaticMeshComponent::StaticClass(), Meshes);
	
}

// Called every frame
void AShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(CrashTime > 0)
	{
		CrashTime -= DeltaTime;
		if(CrashTime < 0)
		{
			MaterialInstance->SetVectorParameterValue("Colour", OldColour);
		}
		else
		{
			MaterialInstance->SetVectorParameterValue("Colour", FLinearColor::Red);
		}
	}
	if(NumWood > 0 || NumStone > 0 || NumGrain > 0)
	{
		MaterialInstance->SetScalarParameterValue("Emissive", 0.35);
	}
	else
	{
		MaterialInstance->SetScalarParameterValue("Emissive", 0);
	}

	ActionStateMachine->Tick(DeltaTime);
	
}

void AShip::OnIdleEnter()
{
	
}

void AShip::OnIdleTick(float DeltaTime)
{
	if(CurrentIdleTime >= MaxIdleTime)
	{
		CurrentIdleTime = 0;

		if(!bActionsAdded)
		{
			AddActions();
		}

		//Change the bForwardSearch parameter in this function call to false if using backwards planning
		if(GOAPPlanner::Plan(this, false))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has found a plan. Executing plan!"), *GetName());
			ActionStateMachine->ChangeState(State_Action);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s was unable to find a plan. Idling for %f seconds"), *GetName(), MaxIdleTime);
		}
	}
	else
	{
		CurrentIdleTime += DeltaTime;
	}
}

void AShip::OnIdleExit()
{
}

void AShip::OnMoveEnter()
{
	if(PlannedActions.IsEmpty())
	{
		ActionStateMachine->ChangeState(State_Idle);
		return;
	}

	GOAPAction* CurrentAction = *PlannedActions.Peek();

	if(!CurrentAction->RequiresInRange() && CurrentAction->Target == nullptr)
	{
		ActionStateMachine->ChangeState(State_Idle);
		return;
	}

	if(CurrentAction->RequiresInRange())
	{
		CurrentAction->SetTarget(this);

		if (CurrentAction->Target == nullptr) {
			PlannedActions.Empty();
			ActionStateMachine->ChangeState(State_Idle);
		}

		GridNode* GoalLocation = LevelGenerator->FindGridNode(CurrentAction->Target);
		if(GoalLocation)
		{
			GoalNode = GoalLocation;
			LevelGenerator->CalculatePath(this, GoalLocation);
		}
	}
}

void AShip::OnMoveTick(float DeltaTime)
{
	GOAPAction* CurrentAction = *PlannedActions.Peek();
	ActualSpeed = MoveSpeed;
	
	if(Path.Num() > 0)
	{
		if(Path[0]->ObjectAtLocation != this && Path[0]->ObjectAtLocation != nullptr && PotentialCrash == nullptr)
		{
			PotentialCrash = Cast<AShip>(Path[0]->ObjectAtLocation);
			UE_LOG(LogTemp, Warning, TEXT("Ship %s has a potential crash with Ship %s!"), *this->GetName(), *Path[0]->ObjectAtLocation->GetName());
			if(LevelGenerator)
			{
				if(LevelGenerator->CollisionAndReplanning)
				{
					LevelGenerator->Replan(this);
					LevelGenerator->NumReplans++;
				}
			}
		}
		else
		{
			Path[0]->ObjectAtLocation = this;
		}
		
		FVector CurrentPosition = GetActorLocation();

		float TargetXPos = Path[0]->X * ALevelGenerator::GRID_SIZE_WORLD;
		float TargetYPos = Path[0]->Y * ALevelGenerator::GRID_SIZE_WORLD;

		FVector TargetPosition(TargetXPos, TargetYPos, CurrentPosition.Z);

		Direction = TargetPosition - CurrentPosition;
		Direction.Normalize();

		CurrentPosition += Direction * ActualSpeed * DeltaTime;

		if(FVector::Dist(CurrentPosition, TargetPosition) <= Tolerance)
		{
			CurrentPosition = TargetPosition;
			
			if(PotentialCrash)
			{
				if(FVector::Dist(GetActorLocation(), PotentialCrash->GetActorLocation()) <= 90 || FMath::RadiansToDegrees( acos( Direction.Dot(PotentialCrash->Direction))) == 180)
				{
					UE_LOG(LogTemp, Warning, TEXT("Ship %s CRASHED WITH Ship %s!"), *this->GetName(), *Path[0]->ObjectAtLocation->GetName());

					CrashTime = 0.5f;
					
					if(LevelGenerator)
					{
						if(LevelGenerator->CollisionAndReplanning)
						{
							LevelGenerator->NumCollisions++;
						}
					}
				}
				PotentialCrash = nullptr;
			}
			
			if(Path[0] == GoalNode)
			{
				bAtGoal = true;
			}
			else
			{
				Path[0]->ObjectAtLocation = nullptr;
			}
			
			XPos = Path[0]->X;
			YPos = Path[0]->Y;
			Path.RemoveAt(0);
		}
		
		SetActorLocation(CurrentPosition);
		SetActorRotation(Direction.Rotation());
		
	}
	else
	{
		for(int i = 0; i < PathDisplayActors.Num(); i++)
		{
			if(PathDisplayActors[i])
			{
				PathDisplayActors[i]->Destroy();
			}
		}
		CurrentAction->bInRange = true;
		ActionStateMachine->ChangeState(State_Action);
	}

	
}

void AShip::OnMoveExit()
{
}

void AShip::OnActionEnter()
{
}

void AShip::OnActionTick(float DeltaTime)
{
	if(PlannedActions.IsEmpty())
	{
		ActionStateMachine->ChangeState(State_Idle);
		return;
	}

	GOAPAction* CurrentAction = *PlannedActions.Peek();

	if(CurrentAction->IsActionDone())
	{
		PlannedActions.Dequeue(CurrentAction);

		for(AActor* PathObject : PathDisplayActors)
		{
			if(PathObject)
			{
				PathObject->Destroy();
			}
		}
		PathDisplayActors.Empty();
	}
	if(!PlannedActions.IsEmpty())
	{
		CurrentAction = *PlannedActions.Peek();

		bool InRange = CurrentAction->RequiresInRange() ? CurrentAction->bInRange : true;

		if(InRange)
		{
			bool IsActionSuccessful = CurrentAction->Tick(this, DeltaTime);

			if(!IsActionSuccessful)
			{
				ActionStateMachine->ChangeState(State_Idle);
				OnPlanAborted(CurrentAction);
			}
		}
		else
		{
			ActionStateMachine->ChangeState(State_Move);
		}
	}
	else
	{
		ActionStateMachine->ChangeState(State_Idle);
	}
}

void AShip::OnActionExit()
{
	
}

TMap<STATE_KEY, int> AShip::GetWorldState()
{
	TMap<STATE_KEY, int> WorldState;
	
	WorldState.Add(AgentWood, NumWood);
	WorldState.Add(HasWood, NumWood > 0);
	WorldState.Add(AgentStone, NumStone);
	WorldState.Add(HasStone, NumStone > 0);
	WorldState.Add(AgentGrain, NumGrain);
	WorldState.Add(HasGrain, NumGrain > 0);
	
	TArray<AActor*> Resources;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResource::StaticClass(), Resources);
	TArray<AActor*> Buildings;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), Buildings);

	TArray<AResource*> Lumberyards;
	TArray<AResource*> Mines;
	TArray<AResource*> Farms;
	TArray<AResource*> BuildingSlots;
	TArray<AResource*> Taverns;

	for(int i = 0; i < Resources.Num(); i++)
	{
		AResource* TempResource = Cast<AResource>(Resources[i]);
		if(TempResource)
		{
			switch (TempResource->ResourceType)
			{
				case(GRID_TYPE::Wood):
					Lumberyards.Add(TempResource);
					break;
				case(GRID_TYPE::Stone):
					Mines.Add(TempResource);
					break;
				case(GRID_TYPE::Grain):
					Farms.Add(TempResource);
					break;
				case(GRID_TYPE::BuildingSlot):
					BuildingSlots.Add(TempResource);
					break;
				default:
					break;
			}
		}
	}

	WorldState.Add(NumLumberyards, Lumberyards.Num());
	WorldState.Add(NumMines, Mines.Num());
	WorldState.Add(NumFarms, Farms.Num());
	WorldState.Add(NumBuildingSlots, BuildingSlots.Num());
	WorldState.Add(NumBuildings, Buildings.Num());

	WorldState.Add(NumPoints, LevelGenerator->Points);
	WorldState.Add(TotalWood, LevelGenerator->TotalWood);
	WorldState.Add(TotalStone, LevelGenerator->TotalStone);
	WorldState.Add(TotalGrain, LevelGenerator->TotalGrain);
	WorldState.Add(TimeLeft, LevelGenerator->TimeLimit);
	
	return WorldState;

}

//-----------------------------------------YOUR CODE HERE------------------------------------------------------//
TArray<TTuple<STATE_KEY, int, char>> AShip::PickGoal()
{
	TArray<TTuple<STATE_KEY, int, char>> GoalToPersue;

	switch (AgentType) {
		case AGENT_TYPE::Woodcutter: {
			if (NumWood == 0) {
				GoalToPersue.Add({STATE_KEY::AgentWood, 1, '>'});
			}
			else {
				GoalToPersue.Add({STATE_KEY::AgentWood, 0, '='});
				GoalToPersue.Add({STATE_KEY::AgentGrain, 0, '='});
				GoalToPersue.Add({STATE_KEY::AgentStone, 0, '='});
			}
			break;
		}

		case AGENT_TYPE::Stonemason: {
			auto CurrentWorldState = GetWorldState();
			int HomeStone = CurrentWorldState[STATE_KEY::TotalStone];

			if (NumStone == 0 && NumWood == 0) {
				if (HomeStone <= 20)
					GoalToPersue.Add({STATE_KEY::AgentStone, 1, '>'});
				else
					GoalToPersue.Add({STATE_KEY::AgentWood, 1, '>'});
			}
			else {
				GoalToPersue.Add({STATE_KEY::AgentWood, 0, '='});
				GoalToPersue.Add({STATE_KEY::AgentGrain, 0, '='});
				GoalToPersue.Add({STATE_KEY::AgentStone, 0, '='});
			}
			break;
		}

		case AGENT_TYPE::Farmer: {
			auto CurrentWorldState = GetWorldState();
			int HomeGrain = CurrentWorldState[STATE_KEY::TotalGrain];

			if (NumGrain == 0 && NumWood == 0) {
				if (HomeGrain <= 10)
					GoalToPersue.Add({STATE_KEY::AgentGrain, 1, '>'});
				else
					GoalToPersue.Add({STATE_KEY::AgentWood, 1, '>'});
			}
			else {
				GoalToPersue.Add({STATE_KEY::AgentWood, 0, '='});
				GoalToPersue.Add({STATE_KEY::AgentGrain, 0, '='});
				GoalToPersue.Add({STATE_KEY::AgentStone, 0, '='});
			}
			break;
		}

		default: {
			auto CurrentWorldState = GetWorldState();
			int TimeLeft = CurrentWorldState[STATE_KEY::TimeLeft];
			int HomeWood = CurrentWorldState[STATE_KEY::TotalWood];
			int HomeStone = CurrentWorldState[STATE_KEY::TotalStone];
			int HomeGrain = CurrentWorldState[STATE_KEY::TotalGrain];
			int NumPoints = CurrentWorldState[STATE_KEY::NumPoints];

			FVector UniversityReq = ABuilding::GetResourceCost(BUILDING_TYPE::University);
			FVector TheatreReq = ABuilding::GetResourceCost(BUILDING_TYPE::Theatre);
			FVector MarketReq = ABuilding::GetResourceCost(BUILDING_TYPE::Market);

			if (TimeLeft > 90) {
				if (HomeWood >= UniversityReq.X && HomeStone >= UniversityReq.Y && HomeGrain >= UniversityReq.Z)
					GoalToPersue.Add({STATE_KEY::NumPoints, NumPoints + ABuilding::GetPointsProvided(BUILDING_TYPE::University), '>'});
			}
			else if (TimeLeft <= 90 && TimeLeft > 30) {
				if (HomeWood >= UniversityReq.X && HomeStone >= UniversityReq.Y && HomeGrain >= UniversityReq.Z)
					GoalToPersue.Add({STATE_KEY::NumPoints, NumPoints + ABuilding::GetPointsProvided(BUILDING_TYPE::University), '>'});
				// else if (HomeWood >= TheatreReq.X && HomeStone >= TheatreReq.Y && HomeGrain >= TheatreReq.Z)
				// 	GoalToPersue.Add({STATE_KEY::NumPoints, NumPoints + ABuilding::GetPointsProvided(BUILDING_TYPE::Theatre), '>'});
			}
			else {
				if (HomeWood >= UniversityReq.X && HomeStone >= UniversityReq.Y && HomeGrain >= UniversityReq.Z)
					GoalToPersue.Add({STATE_KEY::NumPoints, NumPoints + ABuilding::GetPointsProvided(BUILDING_TYPE::University), '>'});
				// else if (HomeWood >= TheatreReq.X && HomeStone >= TheatreReq.Y && HomeGrain >= TheatreReq.Z)
				// 	GoalToPersue.Add({STATE_KEY::NumPoints, NumPoints + ABuilding::GetPointsProvided(BUILDING_TYPE::Theatre), '>'});
				// else if (HomeWood >= MarketReq.X && HomeStone >= MarketReq.Y && HomeGrain >= MarketReq.Z)
				// 	GoalToPersue.Add({STATE_KEY::NumPoints, NumPoints + ABuilding::GetPointsProvided(BUILDING_TYPE::Theatre), '>'});
			}

			break;
		}
	}

	return GoalToPersue;
}

void AShip::OnPlanFailed(TMap<STATE_KEY, int> FailedGoalState)
{
	// ...
	ActionStateMachine->ChangeState(State_Idle);
}

void AShip::OnPlanAborted(GOAPAction* FailedAction)
{
	// change target
	if (AgentType == AGENT_TYPE::Builder) {
		PlannedActions.Empty();
		ActionStateMachine->ChangeState(State_Idle);
	}
	else {
		ActionStateMachine->ChangeState(State_Move);
	}
}

void AShip::AddActions()
{
	bActionsAdded = true;
	
	AvailableActions.Add(new GOAPAction_FetchWood());
	AvailableActions.Add(new GOAPAction_FetchStone());
	AvailableActions.Add(new GOAPAction_FetchGrain());
	AvailableActions.Add(new GOAPAction_Deposit());
	AvailableActions.Add(new GOAPAction_Build(BUILDING_TYPE::University));
	AvailableActions.Add(new GOAPAction_Build(BUILDING_TYPE::Theatre));
	AvailableActions.Add(new GOAPAction_Build(BUILDING_TYPE::Market));
}

