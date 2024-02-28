// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

class AShip;

class GOAPAction;

enum STATE_KEY
{
	AgentWood,
	AgentStone,
	AgentGrain,
	HasWood,
	HasStone,
	HasGrain,
	TotalWood,
	TotalStone,
	TotalGrain,
	NumBuildingSlots,
	NumBuildings,
	NumLumberyards,
	NumMines,
	NumFarms,
	NumPoints,
	TimeLeft
};

//This is a search node in the entire state space
struct GOAPNode
{
	GOAPNode* Parent;

	float RunningCost;

	TMap<STATE_KEY, int> State;

	GOAPAction* Action;
};

class FIT3094_A1_CODE_API GOAPPlanner
{
public:
	
	static bool Plan(AShip* Ship, bool bForwardSearch);
	
	static bool IsGoal(TMap<STATE_KEY, int>& StartState, TMap<STATE_KEY, int>& CurrentState, TArray<TTuple<STATE_KEY, int, char>>& GoalConditions);

	static TArray<GOAPNode*> Expand(GOAPNode* Node, AShip* Ship);

	static bool SameState(TMap<STATE_KEY, int>& Conditions, TMap<STATE_KEY, int>& State);

	static int NodeHeuristic(TMap<STATE_KEY, int>& StartState, TMap<STATE_KEY, int>& State, TArray<TTuple<STATE_KEY, int, char>>& GoalConditions);
};
