// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GOAPPlanner.h"

/**
 * 
 */

class AShip;

class FIT3094_A1_CODE_API GOAPAction
{
public:
	GOAPAction();
	virtual ~GOAPAction();

	AActor* Target;
	
	bool bInRange;

	float ActionCost;

	//Called by GOAPPlanner
	//As Actions are objects, they need to be reinitialised every time GOAPPlanner::Plan is called
	virtual void Reset();

	//Called by the FSM when executing a planned action
	//Returns true if the action has finished executing, false otherwise
	virtual bool IsActionDone() = 0;

	//This checks to see if the action is valid for the agent @param Ship
	//Returns true if the action is valid for the agent, false otherwise
	virtual bool SetupAction(AShip* Ship) = 0;

	//If applied in a forward direction, check to see if preconditions are met in current state
	//If applied in a backward direction, check if the effects are met in the current state
	virtual bool CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState) = 0;

	//If applied in a forward direction, apply effects to current state
	//If applied in a backward direction, apply preconditions to current state
	virtual void ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState) = 0;

	//This is called by the FSM to execute a planned action
	virtual bool Tick(AShip* Ship, float DeltaTime) = 0;

	//Called by the FSM when executing a planned action
	//Some Actions can only be executing when within Range of a location in the world (GOAPAction::Target)
	//Returns true if ship is at the GOAPAction::Target, false otherwise
	virtual bool RequiresInRange() = 0;

	virtual void SetTarget(AShip* Ship) = 0;
	
};
