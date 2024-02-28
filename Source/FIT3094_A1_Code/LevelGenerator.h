// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "GridNode.h"
#include "Resource.h"
#include "Ship.h"
#include "GameFramework/Actor.h"
#include "LevelGenerator.generated.h"

UCLASS()
class FIT3094_A1_CODE_API ALevelGenerator : public AActor
{
	GENERATED_BODY()

private:

	struct TCompareActor {
		FVector ShipLocation;

		TCompareActor(FVector ShipLocationIn) {
			ShipLocation = ShipLocationIn;
		}

		FORCEINLINE bool operator()(const AActor& A, const AActor& B) const {
			return FVector::Dist(ShipLocation, A.GetActorLocation()) < FVector::Dist(ShipLocation, B.GetActorLocation());
		}
	};

public:
	
	// Sets default values for this actor's properties
	ALevelGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static const int MAX_MAP_SIZE = 200;
	static const int GRID_SIZE_WORLD = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MapSizeX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MapSizeY;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> DeepBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> LandBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ShallowBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AResource> WoodBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AResource> StoneBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AResource> GrainBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AResource> BuildLocationBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BuildingBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ShipBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> PathDisplayBlueprint;
	UPROPERTY(EditAnywhere)
	AActor* Camera;

	UPROPERTY(EditAnywhere)
		bool CollisionAndReplanning = false;
	

	bool bCameraRotated = false;

	GridNode* WorldArray[MAX_MAP_SIZE][MAX_MAP_SIZE];
	TArray<AActor*> PathDisplayActors;
	TArray<AActor*> Terrain;
	TArray<FVector2d> ShipSpawns;
	TArray<FVector2d> ResourceSpawns;
	TArray<AShip*> Ships;
	
	int TotalIndex = 50;
	//Town Centres/Wood/Stone/Grain/Taverns/BuildingSlots
	int ResourcesToSpawn [6] = {5, 60, 60, 60, 30};
	//Woodcutters/Stonemasons/Farmers/Builders
	int ShipsToSpawn [4] = {3, 3, 3, 2};
	
	int TotalWood = 0;
	int TotalStone = 0;
	int TotalGrain = 0;
	int Points = 0;
	float TimeLimit = 180;

	int NumReplans = 0;
	int NumCollisions = 0;

	void SpawnWorldActors(TArray<TArray<char>> Grid);
	void GenerateNodeGrid(TArray<TArray<char>> Grid);
	void ResetAllNodes();
	float CalculateDistanceBetween(GridNode* First, GridNode* Second);
	void GenerateWorldFromFile(TArray<FString> WorldArrayStrings);
	void GenerateScenarioFromFile(TArray<FString> ScenarioArrayStrings);
	void InitialisePaths();
	void RenderPath(AShip* Ship);
	void ResetPath();
	void InitialiseLevel();
	void DestroyAllActors();
	void CalculatePath(AShip* Ship, GridNode* Resource);

	// replan helper function
	void CalculatePathReplan(AShip* Ship, GridNode* Resource, GridNode* CrashLocation);

	// replan helper function
	AActor* CalculateNextGoal(AActor* Ship, GRID_TYPE ResourceType);

	
	AActor* CalculateNearestGoal(AActor* Ship, GRID_TYPE ResourceType, BUILDING_TYPE BuildingType);
	GridNode* FindGridNode(AActor* ActorResource);
	void Replan(AShip* Ship);
	
};
