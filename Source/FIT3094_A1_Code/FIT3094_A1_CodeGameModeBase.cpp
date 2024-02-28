// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "FIT3094_A1_CodeGameModeBase.h"

#include "Misc/FileHelper.h"

TArray<FString> AFIT3094_A1_CodeGameModeBase::GetMapFileList()
{
	TArray<FString> MapFiles;

	const FString MapsDir = FPaths::ProjectContentDir() + "MapFiles/";
	FPlatformFileManager::Get().GetPlatformFile().FindFiles(MapFiles, *MapsDir, nullptr);

	return MapFiles;
}

FString AFIT3094_A1_CodeGameModeBase::GetRandomMapText()
{
	TArray<FString> MapFiles = GetMapFileList();

	int32 MapPosition = FMath::RandRange(0, MapFiles.Num() - 1);
	FString MapPath = MapFiles[MapPosition];

	FString MapText;
	FFileHelper::LoadFileToString(MapText, *MapPath);

	return MapText;
}

TArray<FString> AFIT3094_A1_CodeGameModeBase::GetMapArray(FString MapString)
{
	TArray<FString> MapArray;

	FString MapText = MapString;
	MapText.ParseIntoArrayLines(MapArray);

	return MapArray;
}

FString AFIT3094_A1_CodeGameModeBase::GetAssessedMapFile()
{
	TArray<FString> MapFiles;

	const FString MapsDir = FPaths::ProjectContentDir() + "MapFiles/Assessed/Map/";
	FPlatformFileManager::Get().GetPlatformFile().FindFiles(MapFiles, *MapsDir, nullptr);
	
	FString MapText;
	FFileHelper::LoadFileToString(MapText, *MapFiles[0]);
	
	return MapText;
}

FString AFIT3094_A1_CodeGameModeBase::GetScenarioFile()
{
	TArray<FString> ScenarioFiles;

	const FString ScenarioDir = FPaths::ProjectContentDir() + "MapFiles/Assessed/Scen/";
	FPlatformFileManager::Get().GetPlatformFile().FindFiles(ScenarioFiles, *ScenarioDir, nullptr);

	FString ScenarioText;
	FFileHelper::LoadFileToString(ScenarioText, *ScenarioFiles[0]);
	
	return ScenarioText;
}
