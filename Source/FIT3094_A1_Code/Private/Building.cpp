// Fill out your copyright notice in the Description page of Project Settings.


#include "Building.h"

// Sets default values
ABuilding::ABuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int ABuilding::GetTimeRequired()
{
	float TimeRequired = 1;
	switch(BuildingType)
	{
	case(BUILDING_TYPE::Market):
		 TimeRequired = 5;
		break;
	case(BUILDING_TYPE::Theatre):
		TimeRequired = 10;
		break;
	case(BUILDING_TYPE::University):
		TimeRequired = 15;
		break;
	}
	return TimeRequired;
}

int ABuilding::GetTimeRequired(BUILDING_TYPE Type)
{
    float TimeRequired = 1;
	switch(Type)
	{
	case(BUILDING_TYPE::Market):
		 TimeRequired = 5;
		break;
	case(BUILDING_TYPE::Theatre):
		TimeRequired = 10;
		break;
	case(BUILDING_TYPE::University):
		TimeRequired = 15;
		break;
	}
	return TimeRequired;
}

FVector ABuilding::GetResourceCost()
{
	switch(BuildingType)
	{
		case(BUILDING_TYPE::Market):
			ResourceCost = FVector(5, 0, 0);
			break;
		case(BUILDING_TYPE::Theatre):
			ResourceCost = FVector(10, 5, 0);
			break;
		case(BUILDING_TYPE::University):
			ResourceCost = FVector(15, 10, 5);
			break;
	}
	return ResourceCost;
}

FVector ABuilding::GetResourceCost(BUILDING_TYPE Type)
{
	FVector ResourcesTaken;
	switch(Type)
	{
	case(BUILDING_TYPE::Market):
		ResourcesTaken = FVector(5, 0, 0);
		break;
	case(BUILDING_TYPE::Theatre):
		ResourcesTaken = FVector(10, 5, 0);
		break;
	case(BUILDING_TYPE::University):
		ResourcesTaken = FVector(15, 10, 5);
		break;
	}
	return ResourcesTaken;
}

int ABuilding::GetPointsProvided()
{
	switch(BuildingType)
	{
	case(BUILDING_TYPE::Market):
		PointsProvided = 50;
		break;
	case(BUILDING_TYPE::Theatre):
		PointsProvided = 250;
		break;
	case(BUILDING_TYPE::University):
		PointsProvided = 1000;
		break;
	}
	return PointsProvided;
}

int ABuilding::GetPointsProvided(BUILDING_TYPE Type)
{
	int PointsProvided = 0;
    switch(Type)
	{
	case(BUILDING_TYPE::Market):
		PointsProvided = 50;
		break;
	case(BUILDING_TYPE::Theatre):
		PointsProvided = 250;
		break;
	case(BUILDING_TYPE::University):
		PointsProvided = 1000;
		break;
	}
	return PointsProvided;
}
