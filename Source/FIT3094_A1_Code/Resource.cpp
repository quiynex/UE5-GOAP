// Fill out your copyright notice in the Description page of Project Settings.


#include "Resource.h"

// Sets default values
AResource::AResource()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	ResourceCount = 5;

}

// Called when the game starts or when spawned
void AResource::BeginPlay()
{
	Super::BeginPlay();

	Mesh->SetWorldScale3D(FVector(0.75, 0.75, 0.75));
	
}

// Called every frame
void AResource::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AResource::ChangeMaterial()
{
	switch(ResourceType)
	{
	case GRID_TYPE::Wood:
		Mesh->SetMaterial(0, WoodMaterial);
		break;
	case GRID_TYPE::Stone:
		Mesh->SetMaterial(0, StoneMaterial);
		break;
	case GRID_TYPE::Grain:
		Mesh->SetMaterial(0, GrainMaterial);
		break;
	case GRID_TYPE::BuildingSlot:
		Mesh->SetMaterial(0, BuildingSlotMaterial);
		break;
	}
}

