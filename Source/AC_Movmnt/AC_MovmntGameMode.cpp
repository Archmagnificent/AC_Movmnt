// Copyright Epic Games, Inc. All Rights Reserved.

#include "AC_MovmntGameMode.h"
#include "AC_MovmntCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAC_MovmntGameMode::AAC_MovmntGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
