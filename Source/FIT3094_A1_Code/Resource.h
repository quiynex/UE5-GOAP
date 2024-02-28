// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridNode.h"
#include "GameFramework/Actor.h"
#include "Resource.generated.h"

UCLASS()
class FIT3094_A1_CODE_API AResource : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AResource();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		int ResourceCount;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere)
		UMaterial* WoodMaterial;
	UPROPERTY(EditAnywhere)
		UMaterial* StoneMaterial;
	UPROPERTY(EditAnywhere)
		UMaterial* GrainMaterial;
	UPROPERTY(EditAnywhere)
		UMaterial* BuildingSlotMaterial;

	void ChangeMaterial();
	
	
	UPROPERTY(VisibleAnywhere)
		GRID_TYPE ResourceType;
	int XPos;
	int YPos;

};
