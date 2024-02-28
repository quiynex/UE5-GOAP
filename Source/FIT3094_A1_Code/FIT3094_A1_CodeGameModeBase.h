// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "FIT3094_A1_CodeGameModeBase.generated.h"


/**
 * 
 */
UCLASS()
class FIT3094_A1_CODE_API AFIT3094_A1_CodeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Utility Functions")
		TArray<FString> GetMapFileList();

	UFUNCTION(BlueprintCallable, Category = "Utility Functions")
		FString GetRandomMapText();

	UFUNCTION(BlueprintCallable, Category = "Utility Functions")
		TArray<FString> GetMapArray(FString MapString);
		
	UFUNCTION()
	    FString GetAssessedMapFile();
	UFUNCTION()
		FString GetScenarioFile();
	
	
};
