// Fill out your copyright notice in the Description page of Project Settings.


#include "GOAPPlanner.h"

#include "GOAPAction.h"
#include "Ship.h"

bool GOAPPlanner::Plan(AShip* Ship, bool bForwardSearch)
{
	
	for(int i = 0; i < Ship->AvailableActions.Num(); i++)
	{
		Ship->AvailableActions[i]->Reset();
	}

	
	GOAPNode* GoalNode = new GOAPNode();
	TArray<TTuple<STATE_KEY, int, char>> GoalConditions = Ship->PickGoal();
	TMap<STATE_KEY, int> GoalState;
	for(TTuple<STATE_KEY, int, char> Condition : GoalConditions)
	{
		GoalState.Add(Condition.Get<0>(), Condition.Get<1>());
	}
	GoalNode->State = GoalState;
	GoalNode->Action = nullptr;
	GoalNode->RunningCost = 0;
	GoalNode->Parent = nullptr;

	
	GOAPNode* StartNode = new GOAPNode();
	StartNode->State = Ship->GetWorldState();
	StartNode->Action = nullptr;
	StartNode->RunningCost = 0;
	StartNode->Parent = nullptr;
	
	TArray<GOAPNode*> Open;
	TArray<GOAPNode*> Closed;

	if(bForwardSearch)
	{
		Open.Push(StartNode);
	}
	else
	{
		Open.Push(GoalNode);
	}

	int MaxRunningCost = 5;

	while (Open.Num() > 0)
	{
		float SmallestF = Open[0]->RunningCost + NodeHeuristic(StartNode->State, Open[0]->State, GoalConditions);
		int SmallestFIndex = 0;
		
		for(int i = 1; i < Open.Num(); i++)
		{
			int CurrentF = Open[i]->RunningCost + NodeHeuristic(StartNode->State, Open[i]->State, GoalConditions);
			if(CurrentF < SmallestF)
			{
				SmallestF = CurrentF;
				SmallestFIndex = i;
			}
		}
		
		GOAPNode* CurrentNode = Open[SmallestFIndex];
		Open.RemoveAt(SmallestFIndex);
		Closed.Add(CurrentNode);

		if(CurrentNode->RunningCost > MaxRunningCost)
		{
			return false;
		}

		
		if(IsGoal(StartNode->State, CurrentNode->State,GoalConditions))
		{
			
			TArray<GOAPAction*> ActionsToTake;
			while(CurrentNode->Parent)
			{
				ActionsToTake.Add(CurrentNode->Action);
				CurrentNode = CurrentNode->Parent;
			}
			
			for(int i = 0; i < ActionsToTake.Num(); i++)
			{
				Ship->PlannedActions.Enqueue(ActionsToTake[i]);
			}
			return true;
		}

		
		TArray<GOAPNode*> ConnectedNodes = Expand(CurrentNode, Ship);

		for(int i = 0; i < ConnectedNodes.Num(); i++)
		{
			int OpenTempTracker = 0;
			int ClosedTempTracker = 0;
			
			for(int j = 0; j < Open.Num(); j++)
			{
				if(!SameState(Open[j]->State, ConnectedNodes[i]->State))
				{
					OpenTempTracker++;
				}
			}
			for(int j = 0; j < Closed.Num(); j++)
			{
				if(!SameState(Closed[j]->State, ConnectedNodes[i]->State))
				{
					ClosedTempTracker++;
				}
			}
			if(ClosedTempTracker == Closed.Num())
			{
				int PossibleG = CurrentNode->RunningCost + ConnectedNodes[i]->Action->ActionCost;
				bool bPossibleGBetter = false;
				if(OpenTempTracker == Open.Num())
				{
					Open.Add(ConnectedNodes[i]);
					bPossibleGBetter = true;
				}
				else if(PossibleG < ConnectedNodes[i]->RunningCost)
				{
					bPossibleGBetter = true;
				}
				if(bPossibleGBetter)
				{
					ConnectedNodes[i]->Parent = CurrentNode;
					ConnectedNodes[i]->RunningCost = PossibleG;
				}
			}
		}
	}
	
	return false;
}

bool GOAPPlanner::IsGoal(TMap<STATE_KEY, int>& StartState, TMap<STATE_KEY, int>& CurrentState, TArray<TTuple<STATE_KEY, int, char>>& GoalConditions)
{
	for(TTuple<STATE_KEY, int, char> GoalCondition : GoalConditions)
	{
		if(CurrentState.Contains(GoalCondition.Get<0>()))
		{
			if(GoalCondition.Get<2>() == '<')
			{
				if(*CurrentState.Find(GoalCondition.Get<0>()) < *StartState.Find(GoalCondition.Get<0>()))
				{
					return false;
				}
			}
			else if(GoalCondition.Get<2>() == '>')
			{
				if(*CurrentState.Find(GoalCondition.Get<0>()) > *StartState.Find(GoalCondition.Get<0>()))
				{
					return false;
				}
			}
			else if(GoalCondition.Get<2>() == '=')
			{
				if(*CurrentState.Find(GoalCondition.Get<0>()) != *StartState.Find(GoalCondition.Get<0>()))
				{
					return false;
				}
			}
		}
	}
	if(GoalConditions.IsEmpty())
	{
		return false;
	}
	return true;
}

bool GOAPPlanner::SameState(TMap<STATE_KEY, int>& Conditions, TMap<STATE_KEY, int>& State)
{
	for(TTuple<STATE_KEY, int> Condition : Conditions)
	{
		int* CurrentStateCondition = State.Find(Condition.Key);

		if(CurrentStateCondition)
		{
			if(Condition.Value != *CurrentStateCondition)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

int GOAPPlanner::NodeHeuristic(TMap<STATE_KEY, int>& StartState, TMap<STATE_KEY, int>& State, TArray<TTuple<STATE_KEY, int, char>>& GoalConditions)
{
	int Heuristic = 0;
	for(TTuple<STATE_KEY, int, char> GoalCondition : GoalConditions)
	{
		if(StartState.Contains(GoalCondition.Get<0>()))
		{
			if(GoalCondition.Get<2>() == '<')
			{
				if(*State.Find(GoalCondition.Get<0>()) < *StartState.Find(GoalCondition.Get<0>()))
				{
					Heuristic++;
				}
			}
			else if(GoalCondition.Get<2>() == '>')
			{
				if(*State.Find(GoalCondition.Get<0>()) > *StartState.Find(GoalCondition.Get<0>()))
				{
					Heuristic++;
				}
			}
			else if(GoalCondition.Get<2>() == '=')
			{
				if(*State.Find(GoalCondition.Get<0>()) != *StartState.Find(GoalCondition.Get<0>()))
				{
					Heuristic++;
				}
			}
		}
	}
	return Heuristic;
}

//---------------------------------------------- YOUR CODE HERE ----------------------------------------------//
TArray<GOAPNode*> GOAPPlanner::Expand(GOAPNode* Node, AShip* Ship)
{
	TArray<GOAPNode*> ConnectedNodes;

	for (auto* Action: Ship->AvailableActions) {
		if (!Action->SetupAction(Ship))
			continue;

		if (Action->CheckPreconditions(Ship, Node->State)) {
			GOAPNode* Neighbor = new GOAPNode();
			Neighbor->Action = Action;
			Neighbor->State = Node->State;
			Action->ApplyEffects(Ship, Neighbor->State);
			ConnectedNodes.Add(Neighbor);
		}
	}
	
	return ConnectedNodes;
}