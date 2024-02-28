// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

UENUM()
enum class BUILDING_TYPE : uint8
{
	Market,
	Theatre,
	University,
};

UCLASS()
class FIT3094_A1_CODE_API ABuilding : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuilding();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere)
		BUILDING_TYPE BuildingType;

	//Wood = X, Stone = Y, Grain = Z
	FVector ResourceCost;
	int PointsProvided;
	float TimeToConstruct;

	int GetTimeRequired();
	static int GetTimeRequired(BUILDING_TYPE Type);
	FVector GetResourceCost();
	static FVector GetResourceCost(BUILDING_TYPE Type);
	int GetPointsProvided();
	static int GetPointsProvided(BUILDING_TYPE Type);
	
};
