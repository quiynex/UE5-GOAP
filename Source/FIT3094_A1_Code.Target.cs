// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class FIT3094_A1_CodeTarget : TargetRules
{
	public FIT3094_A1_CodeTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "FIT3094_A1_Code" } );
	}
}
