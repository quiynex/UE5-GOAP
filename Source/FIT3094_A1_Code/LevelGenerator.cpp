// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"

#include "FIT3094_A1_CodeGameModeBase.h"
#include "Ship.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALevelGenerator::ALevelGenerator()
{
 	// Set this actor to call Tick() every frame. 
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevelGenerator::BeginPlay()
{
	Super::BeginPlay();

	AFIT3094_A1_CodeGameModeBase* GameModeBase = Cast<AFIT3094_A1_CodeGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GenerateWorldFromFile(GameModeBase->GetMapArray(GameModeBase->GetAssessedMapFile()));
	GenerateScenarioFromFile(GameModeBase->GetMapArray(GameModeBase->GetScenarioFile()));
	InitialiseLevel();
}

// Called every frame
void ALevelGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeLimit -= DeltaTime;
	if(TimeLimit <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("You got %d points!"), Points);
		UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
	}

	GEngine->AddOnScreenDebugMessage(5, 15, FColor::Red, FString::Printf(TEXT("Time Left: %d"), (int)TimeLimit));
	GEngine->AddOnScreenDebugMessage(1, 15, FColor::Yellow, FString::Printf(TEXT("Amount of Grain: %d"), TotalGrain));
	GEngine->AddOnScreenDebugMessage(2, 15, FColor::White, FString::Printf(TEXT("Amount of Stone: %d"), TotalStone));
	GEngine->AddOnScreenDebugMessage(3, 15, FColor::Orange, FString::Printf(TEXT("Amount of Wood: %d"), TotalWood));
	GEngine->AddOnScreenDebugMessage(4, 15, FColor::Green, FString::Printf(TEXT("Points: %d"), Points));
	if(CollisionAndReplanning)
	{
		if(NumReplans > 0)
		{
			GEngine->AddOnScreenDebugMessage(6, 15, FColor::Cyan, FString::Printf(TEXT("Ratio of (Replans-Crashes)/Replans: %f"), ((float)(NumReplans - NumCollisions)) / (float)NumReplans));
		}
	}
	
}

void ALevelGenerator::SpawnWorldActors(TArray<TArray<char>> Grid)
{
	if(DeepBlueprint && ShallowBlueprint && LandBlueprint)
	{
		for(int Y = 0; Y < MapSizeY; Y++)
		{
			for(int X = 0; X < MapSizeX; X++)
			{
				float XPos = X * GRID_SIZE_WORLD;
				float YPos = Y * GRID_SIZE_WORLD;

				FVector Position(XPos, YPos, 0);

				switch(Grid[Y][X])
				{
				case '.':
					Terrain.Add(GetWorld()->SpawnActor(DeepBlueprint, &Position));
					break;
				case 'T':
					Terrain.Add(GetWorld()->SpawnActor(ShallowBlueprint, &Position));
					break;
				case '@':
					Terrain.Add(GetWorld()->SpawnActor(LandBlueprint, &Position));
					break;
				default:
					break;
				}
			}
		}
	}

	if(Camera)
	{
		FVector CameraPosition = Camera->GetActorLocation();

		CameraPosition.X = MapSizeX * 0.5 * GRID_SIZE_WORLD;
		CameraPosition.Y = MapSizeY * 0.5 * GRID_SIZE_WORLD;

		if(!bCameraRotated)
		{
			bCameraRotated = true;
			FRotator CameraRotation = Camera->GetActorRotation();

			CameraRotation.Pitch = 270;
			CameraRotation.Roll = 180;

			Camera->SetActorRotation(CameraRotation);
			Camera->AddActorLocalRotation(FRotator(0,0,90));
		}
		
		Camera->SetActorLocation(CameraPosition);
		
	}
}

void ALevelGenerator::GenerateNodeGrid(TArray<TArray<char>> Grid)
{
	for(int Y = 0; Y < MapSizeY; Y++)
	{
		for(int X = 0; X < MapSizeX; X++)
		{
			WorldArray[Y][X] = new GridNode();
			WorldArray[Y][X]->Y = Y;
			WorldArray[Y][X]->X = X;

			switch(Grid[Y][X])
			{
			case '.':
				WorldArray[Y][X]->GridType = GRID_TYPE::DeepWater;
				break;
			case '@':
				WorldArray[Y][X]->GridType = GRID_TYPE::Land;
				break;
			case 'T':
				WorldArray[Y][X]->GridType = GRID_TYPE::ShallowWater;
				break;
			default:
				break;
			}
			
		}
	}
}

void ALevelGenerator::ResetAllNodes()
{
	for( int Y = 0; Y < MapSizeY; Y++)
	{
		for(int X = 0; X < MapSizeX; X++)
		{
			WorldArray[Y][X]->F = 0;
			WorldArray[Y][X]->G = 0;
			WorldArray[Y][X]->H = 0;
			WorldArray[Y][X]->Parent = nullptr;
		}
	}
}

float ALevelGenerator::CalculateDistanceBetween(GridNode* First, GridNode* Second)
{
	FVector DistToTarget = FVector(Second->X - First->X,Second->Y - First->Y, 0);
	return DistToTarget.Size();
}

void ALevelGenerator::GenerateWorldFromFile(TArray<FString> WorldArrayStrings)
{
	if(WorldArrayStrings.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Map file not found!"))
		return;
	}

	FString Height = WorldArrayStrings[1];
	Height.RemoveFromStart("height ");
	MapSizeY = FCString::Atoi(*Height);

	FString Width = WorldArrayStrings[2];
	Width.RemoveFromStart("width ");
	MapSizeX = FCString::Atoi(*Width);

	TArray<TArray<char>> CharMapArray;
	CharMapArray.Init( TArray<char>(), MAX_MAP_SIZE);
	
	for(int i = 0; i < CharMapArray.Num(); i++)
	{
		CharMapArray[i].Init('x', MAX_MAP_SIZE);
	}
	
	for(int LineNum = 4; LineNum < MapSizeY + 4; LineNum++)
	{
		for(int CharNum = 0; CharNum < MapSizeX; CharNum++)
		{
			CharMapArray[LineNum-4][CharNum] = WorldArrayStrings[LineNum][CharNum];
		}
	}

	GenerateNodeGrid(CharMapArray);
	SpawnWorldActors(CharMapArray);
	
}

void ALevelGenerator::GenerateScenarioFromFile(TArray<FString> ScenarioArrayStrings)
{
	if(ScenarioArrayStrings.Num() == 0)
	{
		return;
	}
	
	for(int i = 1; i < ScenarioArrayStrings.Num(); i++)
	{
		TArray<FString> SplitLine;
		FString CurrentLine = ScenarioArrayStrings[i];
		
		CurrentLine.ParseIntoArray(SplitLine,TEXT("\t"));

		int ShipX = FCString::Atoi(*SplitLine[4]);
		int ShipY = FCString::Atoi(*SplitLine[5]);
		int ResourceX = FCString::Atoi(*SplitLine[6]);
		int ResourceY = FCString::Atoi(*SplitLine[7]);

		ShipSpawns.Add(FVector2d(ShipX, ShipY));
		ResourceSpawns.Add(FVector2d(ResourceX, ResourceY));
	}
}

void ALevelGenerator::InitialisePaths()
{
	ResetPath();
}

void ALevelGenerator::RenderPath(AShip* Ship)
{
	GridNode* CurrentNode = Ship->GoalNode;

	if(CurrentNode)
	{
		if (CurrentNode->Parent == nullptr)
			return;

		Ship->Path.Empty();

		while(CurrentNode->Parent != nullptr)
		{
			FVector Position(CurrentNode->X * GRID_SIZE_WORLD, CurrentNode->Y * GRID_SIZE_WORLD, 10);
			// AActor* PathActor = GetWorld()->SpawnActor(PathDisplayBlueprint, &Position);
			// PathDisplayActors.Add(PathActor);

			Ship->Path.EmplaceAt(0, WorldArray[CurrentNode->Y][CurrentNode->X]);
			// Ship->PathDisplayActors.Add(PathActor);
			CurrentNode = CurrentNode->Parent;
		}
	}
}

void ALevelGenerator::ResetPath()
{
	ResetAllNodes();

	for(int i = 0; i < PathDisplayActors.Num(); i++)
	{
		PathDisplayActors[i]->Destroy();
	}
	PathDisplayActors.Empty();

	for(int i = 0; i < Ships.Num(); i++)
	{
		Ships[i]->Path.Empty();
	}
}

void ALevelGenerator::InitialiseLevel()
{
	DestroyAllActors();

	//Spawn ships
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < ShipsToSpawn[i]; j++)
		{
			int ShipXPos = ShipSpawns[j + TotalIndex].X;
			int ShipYPos = ShipSpawns[j + TotalIndex].Y;
			TotalIndex++;
			FVector ShipPosition(ShipXPos* GRID_SIZE_WORLD, ShipYPos* GRID_SIZE_WORLD, 20);
			AShip* Ship = Cast<AShip>(GetWorld()->SpawnActor(ShipBlueprint, &ShipPosition));

			Ship->XPos = ShipXPos;
			Ship->YPos = ShipYPos;
			Ships.Add(Ship);
		
			switch(i)
			{
			case 0:
				Ship->AgentType = AShip::Woodcutter;
				break;
			case 1:
				Ship->AgentType = AShip::Stonemason;
				break;
			case 2:
				Ship->AgentType = AShip::Farmer;
				break;
			case 3:
				Ship->AgentType = AShip::Builder;
				break;
			}
			
			switch(Ship->AgentType)
			{
			case AShip::Woodcutter:
				Ship->MaterialInstance = UMaterialInstanceDynamic::Create(Ship->WoodCutterMaterial, Ship);
				Ship->WoodCutterMaterial->GetVectorParameterValue(FName(TEXT("Colour")), Ship->OldColour);
				break;
			case AShip::Stonemason:
				Ship->MaterialInstance = UMaterialInstanceDynamic::Create(Ship->StoneMasonMaterial, Ship);
				Ship->StoneMasonMaterial->GetVectorParameterValue(FName(TEXT("Colour")), Ship->OldColour);

				break;
			case AShip::Farmer:
				Ship->MaterialInstance = UMaterialInstanceDynamic::Create(Ship->FarmerMaterial, Ship);
				Ship->FarmerMaterial->GetVectorParameterValue(FName(TEXT("Colour")), Ship->OldColour);

				break;
			case AShip::Builder:
				Ship->MaterialInstance = UMaterialInstanceDynamic::Create(Ship->BuilderMaterial, Ship);
				Ship->BuilderMaterial->GetVectorParameterValue(FName(TEXT("Colour")), Ship->OldColour);
				break;
			}
			
			for(int k = 0; k < Ship->Meshes.Num(); k++)
			{
				Cast<UStaticMeshComponent>(Ship->Meshes[k])->SetMaterial(0, Ship->MaterialInstance);
			}
		}
		
	}

	for(int i  = 0; i < 5; i++)
	{
		for(int j = 0; j < ResourcesToSpawn[i]; j++)
		{
			int ResourceXPos = ResourceSpawns[j + TotalIndex].X;
			int ResourceYPos = ResourceSpawns[j + TotalIndex].Y;
			TotalIndex++;

			FVector ResourcePosition(ResourceXPos* GRID_SIZE_WORLD, ResourceYPos* GRID_SIZE_WORLD, 20);
			AResource* Resource = Cast<AResource>(GetWorld()->SpawnActor(GrainBlueprint, &ResourcePosition));
			Resource->XPos = ResourceXPos;
			Resource->YPos = ResourceYPos;
			if(ResourceXPos >= MapSizeX || ResourceYPos >= MapSizeY)
			{
				continue;
			}
			WorldArray[ResourceYPos][ResourceXPos]->ResourceAtLocation = Resource;

			switch(i)
			{
			case 0:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::Home;
				Resource->ResourceType = GRID_TYPE::Home;
				break;
			case 1:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::Wood;
				Resource->ResourceType = GRID_TYPE::Wood;
				break;
			case 2:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::Stone;
				Resource->ResourceType = GRID_TYPE::Stone;
				break;
			case 3:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::Grain;
				Resource->ResourceType = GRID_TYPE::Grain;
				break;
			case 4:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::BuildingSlot;
				Resource->ResourceType = GRID_TYPE::BuildingSlot;
				break;
			}
			Resource->ChangeMaterial();
		}
	}
	
	InitialisePaths();
}

void ALevelGenerator::DestroyAllActors()
{
	for(int i = 0; i < PathDisplayActors.Num(); i++)
	{
		PathDisplayActors[i]->Destroy();
	}
	PathDisplayActors.Empty();
	
	for(int i = 0; i < Ships.Num(); i++)
	{
		Ships[i]->Destroy();
	}
	Ships.Empty();
	
}

void ALevelGenerator::CalculatePath(AShip* Ship, GridNode* Resource)
{
	ResetAllNodes();
	GridNode* CurrentNode = nullptr;
	bool bIsGoalFound = false;

	TArray<GridNode*> NodesToVisit;
	TArray<GridNode*> Closed;

	GridNode* StartNode = WorldArray[(int)(Ship->GetActorLocation().Y / GRID_SIZE_WORLD)][(int)(Ship->GetActorLocation().X / GRID_SIZE_WORLD)];
	StartNode->G = 0;
	StartNode->H = CalculateDistanceBetween(StartNode, Resource);
	StartNode->F = StartNode->G + StartNode->H;

	Closed.Add(StartNode);
	NodesToVisit.Add(StartNode);

	while (!NodesToVisit.IsEmpty())
	{
		float SmallestF = 0;
		int Index = 0;
		for(int j = 0; j < NodesToVisit.Num(); j++)
		{
			if(j == 0)
			{
				SmallestF = NodesToVisit[j]->F;
				Index = j;
			}
			else
			{
				if(NodesToVisit[j]->F < SmallestF)
				{
					SmallestF = NodesToVisit[j]->F;
					Index = j;
				}
			}
		}

		CurrentNode = NodesToVisit[Index];
		NodesToVisit.RemoveAt(Index);
		Closed.Add(CurrentNode);

		if (CurrentNode == Resource)
		{
			bIsGoalFound = true;
			break;
		}

		TArray<GridNode*> Neighbours;
		// Check the left neighbour
		// Check to ensure not out of range
		if (CurrentNode->X - 1 >= 0)
		{
			// Get the Left neighbor from the list
			Neighbours.Add(WorldArray[CurrentNode->Y][CurrentNode->X - 1]);
		}

		// Check the top neighbour
		// Check to ensure not out of range
		if (CurrentNode->Y + 1 < MapSizeY - 1)
		{
			// Get the top neighbor from the list
			Neighbours.Add(WorldArray[CurrentNode->Y + 1][CurrentNode->X]);
		}

		// Check the right neighbour
		// Check to ensure not out of range
		if (CurrentNode->X + 1 < MapSizeX - 1)
		{
			// Get the right neighbor from the list
			Neighbours.Add(WorldArray[CurrentNode->Y][CurrentNode->X + 1]);
		}

		// Check the bottom neighbour
		// Check to ensure not out of range
		if (CurrentNode->Y - 1 >= 0)
		{
			// Get the bottom neighbor from the list
			Neighbours.Add(WorldArray[CurrentNode->Y - 1][CurrentNode->X]);
		}

		for(int j = 0; j < Neighbours.Num(); j++)
		{
			if(!Closed.Contains(Neighbours[j]))
			{
				int PossibleG = CurrentNode->G + Neighbours[j]->GetTravelCost();
				bool PossibleGBetter = false;
				if(!NodesToVisit.Contains(Neighbours[j]))
				{
					NodesToVisit.Add(Neighbours[j]);
					Neighbours[j]->H = CalculateDistanceBetween(Neighbours[j], Resource);
					PossibleGBetter = true;
				}
				else if(PossibleG < Neighbours[j]->G)
				{
					PossibleGBetter = true;
				}

				if(PossibleGBetter)
				{
					Neighbours[j]->Parent = CurrentNode;
					Neighbours[j]->G = PossibleG;
					Neighbours[j]->F = Neighbours[j]->G + Neighbours[j]->H;
				}
			}
		}
	}

	if (bIsGoalFound)
	{
		RenderPath(Ship);
	}
}

void ALevelGenerator::CalculatePathReplan(AShip *Ship, GridNode *Resource, GridNode *CrashLocation)
{
	ResetAllNodes();

	GridNode* CurrentNode = nullptr;
	bool bIsGoalFound = false;

	TArray<GridNode*> NodesToVisit;
	TArray<GridNode*> Closed;

	GridNode* StartNode = WorldArray[(int)(Ship->GetActorLocation().Y / GRID_SIZE_WORLD)][(int)(Ship->GetActorLocation().X / GRID_SIZE_WORLD)];
	StartNode->G = 0;
	StartNode->H = CalculateDistanceBetween(StartNode, Resource);
	StartNode->F = StartNode->G + StartNode->H;

	Closed.Add(StartNode);
	NodesToVisit.Add(StartNode);

	while (!NodesToVisit.IsEmpty())
	{
		float SmallestF = 0;
		int Index = 0;
		for(int j = 0; j < NodesToVisit.Num(); j++)
		{
			if(j == 0)
			{
				SmallestF = NodesToVisit[j]->F;
				Index = j;
			}
			else
			{
				if(NodesToVisit[j]->F < SmallestF)
				{
					SmallestF = NodesToVisit[j]->F;
					Index = j;
				}
			}
		}

		CurrentNode = NodesToVisit[Index];
		NodesToVisit.RemoveAt(Index);
		Closed.Add(CurrentNode);

		if (CurrentNode == Resource)
		{
			bIsGoalFound = true;
			break;
		}

		TArray<GridNode*> Neighbours;
		// Check the left neighbour
		// Check to ensure not out of range
		if (CurrentNode->X - 1 >= 0)
		{
			// Get the Left neighbor from the list
			Neighbours.Add(WorldArray[CurrentNode->Y][CurrentNode->X - 1]);
		}

		// Check the top neighbour
		// Check to ensure not out of range
		if (CurrentNode->Y + 1 < MapSizeY - 1)
		{
			// Get the top neighbor from the list
			Neighbours.Add(WorldArray[CurrentNode->Y + 1][CurrentNode->X]);
		}

		// Check the right neighbour
		// Check to ensure not out of range
		if (CurrentNode->X + 1 < MapSizeX - 1)
		{
			// Get the right neighbor from the list
			Neighbours.Add(WorldArray[CurrentNode->Y][CurrentNode->X + 1]);
		}

		// Check the bottom neighbour
		// Check to ensure not out of range
		if (CurrentNode->Y - 1 >= 0)
		{
			// Get the bottom neighbor from the list
			Neighbours.Add(WorldArray[CurrentNode->Y - 1][CurrentNode->X]);
		}

		for(int j = 0; j < Neighbours.Num(); j++)
		{
			if (Neighbours[j] == CrashLocation)
				continue;

			if(!Closed.Contains(Neighbours[j]))
			{
				int PossibleG = CurrentNode->G + Neighbours[j]->GetTravelCost();
				bool PossibleGBetter = false;
				if(!NodesToVisit.Contains(Neighbours[j]))
				{
					NodesToVisit.Add(Neighbours[j]);
					Neighbours[j]->H = CalculateDistanceBetween(Neighbours[j], Resource);
					PossibleGBetter = true;
				}
				else if(PossibleG < Neighbours[j]->G)
				{
					PossibleGBetter = true;
				}

				if(PossibleGBetter)
				{
					Neighbours[j]->Parent = CurrentNode;
					Neighbours[j]->G = PossibleG;
					Neighbours[j]->F = Neighbours[j]->G + Neighbours[j]->H;
				}
			}
		}
	}

	if (bIsGoalFound)
	{
		RenderPath(Ship);
	}
}

AActor *ALevelGenerator::CalculateNextGoal(AActor *Ship, GRID_TYPE ResourceType)
{
	TArray<AActor*> Resources;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResource::StaticClass(), Resources);

	Resources.Heapify(TCompareActor(Ship->GetActorLocation()));
	while (!Resources.IsEmpty()) {
		AActor* ClosestResourceActor = nullptr;
		Resources.HeapPop(ClosestResourceActor, TCompareActor(Ship->GetActorLocation()));
		
		AResource* ClosestResource = Cast<AResource>(ClosestResourceActor);

		if (ClosestResource->ResourceType != ResourceType)
			continue;

		if (WorldArray[ClosestResource->YPos][ClosestResource->XPos]->ObjectAtLocation == nullptr)
			return ClosestResourceActor;
	}
	return nullptr;
}

AActor* ALevelGenerator::CalculateNearestGoal(AActor* Ship, GRID_TYPE ResourceType, BUILDING_TYPE BuildingType)
{
	float ShortestPath = 999999;

	AActor* ClosestResource = nullptr;

	TArray<AActor*> Resources;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResource::StaticClass(), Resources);
	for(AActor* Resource : Resources)
	{
		if(Cast<AResource>(Resource)->ResourceType == ResourceType)
		{
			float CurrentPath = FVector::Dist(Ship->GetActorLocation(), Resource->GetActorLocation());
			if(CurrentPath < ShortestPath)
			{
				ShortestPath = CurrentPath;
				ClosestResource = Resource;
			}
		}
	}
	
	// possible bug
	return ClosestResource;
}

GridNode* ALevelGenerator::FindGridNode(AActor* ActorResource)
{
	AResource* TargetResource = Cast<AResource>(ActorResource);
	return WorldArray[TargetResource->YPos][TargetResource->XPos];
}

void ALevelGenerator::Replan(AShip* Ship)
{
	//INSERT REPLANNING HERE
	GridNode* CrashLocation = Ship->Path[0];

	if (CrashLocation != Ship->GoalNode) {
		CalculatePathReplan(Ship, Ship->GoalNode, CrashLocation);
	}
	else {
		GOAPAction* CurrentAction = *Ship->PlannedActions.Peek();
		AActor* PossibleTarget = CalculateNextGoal(Ship, Cast<AResource>(CurrentAction->Target)->ResourceType);

		if (PossibleTarget == nullptr) {
			Ship->ActualSpeed = 0.0f;
			return;
		}

		CurrentAction->Target = PossibleTarget;
		Ship->GoalNode = FindGridNode(PossibleTarget);
		CalculatePathReplan(Ship, Ship->GoalNode, CrashLocation);
	}
	
}
