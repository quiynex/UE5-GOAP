// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GOAPAction.h"
#include "GOAPPlanner.h"
#include "GridNode.h"
#include "StateMachine.h"
#include "GameFramework/Actor.h"
#include "Ship.generated.h"

class ALevelGenerator;

UCLASS()
class FIT3094_A1_CODE_API AShip : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	enum ACTOR_STATES
	{
		State_Nothing,
		State_Idle,
		State_Move,
		State_Action
	};

	enum AGENT_TYPE
	{
		Woodcutter,
		Stonemason,
		Farmer,
		Builder
	};
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere)
		UMaterialInterface* WoodCutterMaterial;
	UPROPERTY(EditAnywhere)
		UMaterialInterface* StoneMasonMaterial;
	UPROPERTY(EditAnywhere)
		UMaterialInterface* FarmerMaterial;
	UPROPERTY(EditAnywhere)
		UMaterialInterface* BuilderMaterial;
	UMaterialInstanceDynamic* MaterialInstance;
	float CrashTime = 0;
	FLinearColor OldColour;
	TArray<UActorComponent*> Meshes;

	UPROPERTY(EditAnywhere)
		float MoveSpeed;
	UPROPERTY(EditAnywhere)
		float Tolerance;

	// replan helper speed
	float ActualSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere)
		int NumWood;
	UPROPERTY(VisibleAnywhere)
		int NumStone;
	UPROPERTY(VisibleAnywhere)
		int NumGrain;

	TArray<GridNode*> Path;
	GridNode* GoalNode;
	ALevelGenerator* LevelGenerator;
	FVector Direction;
	AShip* PotentialCrash = nullptr;
	bool bAtGoal = false;
	int XPos;
	int YPos;
	
	TArray<AActor*> PathDisplayActors;

	StateMachine<ACTOR_STATES, AShip>* ActionStateMachine;

	void OnIdleEnter();
	void OnIdleTick(float DeltaTime);
	void OnIdleExit();

	void OnMoveEnter();
	void OnMoveTick(float DeltaTime);
	void OnMoveExit();

	void OnActionEnter();
	void OnActionTick(float DeltaTime);
	void OnActionExit();

	float MaxIdleTime;
	float CurrentIdleTime;
	
	TArray<GOAPAction*> AvailableActions;
	TQueue<GOAPAction*> PlannedActions;
	AGENT_TYPE AgentType;

	TMap<STATE_KEY, int> GetWorldState();
	TArray<TTuple<STATE_KEY, int, char>> PickGoal();

	void OnPlanFailed(TMap<STATE_KEY, int> FailedGoalState);
	void OnPlanAborted(GOAPAction* FailedAction);
	void AddActions();
	
	bool bActionsAdded = false;
	
};
