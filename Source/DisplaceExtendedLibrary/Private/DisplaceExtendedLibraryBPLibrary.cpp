#include "DisplaceExtendedLibraryPrivatePCH.h"
#include "DisplaceExtendedLibraryBPLibrary.h"

// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformDriver.h"

//MD5 Hash
#include "Runtime/Core/Public/Misc/SecureHash.h"

#include "StaticMeshResources.h"

#include "HeadMountedDisplay.h"

#include "GenericTeamAgentInterface.h"

//For PIE error messages
#include "MessageLog.h"
#define LOCTEXT_NAMESPACE "Fun BP Lib"

//Use MessasgeLog like this: (see GameplayStatics.cpp
/*
#if WITH_EDITOR
FMessageLog("PIE").Error(FText::Format(LOCTEXT("SpawnObjectWrongClass", "SpawnObject wrong class: {0}'"), FText::FromString(GetNameSafe(*ObjectClass))));
#endif // WITH_EDITOR
*/


#include "Runtime/ImageWrapper/Public/Interfaces/IImageWrapper.h"
#include "Runtime/ImageWrapper/Public/Interfaces/IImageWrapperModule.h"

//Body Setup
#include "PhysicsEngine/BodySetup.h"


//Apex issues, can add iOS here  <3 Rama
#if PLATFORM_ANDROID || PLATFORM_HTML5_BROWSER || PLATFORM_IOS
#ifdef WITH_APEX
#undef WITH_APEX
#endif
#define WITH_APEX 0
#endif //APEX EXCLUSIONS

//~~~ PhysX ~~~
#include "PhysXIncludes.h"
#include "PhysXPublic.h"		//For the ptou conversions

//For Scene Locking using Epic's awesome helper macros like SCOPED_SCENE_READ_LOCK
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"
//~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//									Saxon Rah Random Nodes
// Chrono and Random

//Order Matters, 
//		has to be after PhysX includes to avoid isfinite name definition issues
#include <chrono>
#include <random>

#include <string>





UDisplaceExtendedLibraryBPLibrary::UDisplaceExtendedLibraryBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

FDisplaceInput UDisplaceExtendedLibraryBPLibrary::VictoryGetVictoryInput(const FKeyEvent& KeyEvent)
{
	FDisplaceInput VInput;

	VInput.Key = KeyEvent.GetKey();
	VInput.KeyAsString = VInput.Key.GetDisplayName().ToString();

	VInput.bAlt = KeyEvent.IsAltDown();
	VInput.bCtrl = KeyEvent.IsControlDown();
	VInput.bShift = KeyEvent.IsShiftDown();
	VInput.bCmd = KeyEvent.IsCommandDown();

	return VInput;
}
FDisplaceInputAxis UDisplaceExtendedLibraryBPLibrary::VictoryGetVictoryInputAxis(const FKeyEvent& KeyEvent)
{
	FDisplaceInputAxis VInput;

	VInput.Key = KeyEvent.GetKey();
	VInput.KeyAsString = VInput.Key.GetDisplayName().ToString();

	VInput.Scale = 1;

	return VInput;
}

void UDisplaceExtendedLibraryBPLibrary::VictoryGetAllAxisKeyBindings(TArray<FDisplaceInputAxis>& Bindings)
{
	Bindings.Empty();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	if (!Settings) return;

	const TArray<FInputAxisKeyMapping>& Axi = Settings->AxisMappings;

	for (const FInputAxisKeyMapping& Each : Axi)
	{
		Bindings.Add(FDisplaceInputAxis(Each));
	}
}
void UDisplaceExtendedLibraryBPLibrary::VictoryRemoveAxisKeyBind(FDisplaceInputAxis ToRemove)
{
	//GetMutableDefault
	UInputSettings* Settings = GetMutableDefault<UInputSettings>();
	if (!Settings) return;

	TArray<FInputAxisKeyMapping>& Axi = Settings->AxisMappings;

	bool Found = false;
	for (int32 v = 0; v < Axi.Num(); v++)
	{
		if (Axi[v].Key == ToRemove.Key)
		{
			Found = true;
			Axi.RemoveAt(v);
			v = 0;
			continue;
		}
	}

	if (Found)
	{
		//SAVES TO DISK
		Settings->SaveKeyMappings();

		//REBUILDS INPUT, creates modified config in Saved/Config/Windows/Input.ini
		for (TObjectIterator<UPlayerInput> It; It; ++It)
		{
			It->ForceRebuildingKeyMaps(true);
		}
	}
}

void UDisplaceExtendedLibraryBPLibrary::VictoryGetAllActionKeyBindings(TArray<FDisplaceInput>& Bindings)
{
	Bindings.Empty();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	if (!Settings) return;

	const TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	for (const FInputActionKeyMapping& Each : Actions)
	{
		Bindings.Add(FDisplaceInput(Each));
	}
}

void UDisplaceExtendedLibraryBPLibrary::VictoryRemoveActionKeyBind(FDisplaceInput ToRemove)
{
	//GetMutableDefault
	UInputSettings* Settings = GetMutableDefault<UInputSettings>();
	if (!Settings) return;

	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	bool Found = false;
	for (int32 v = 0; v < Actions.Num(); v++)
	{
		if (Actions[v].Key == ToRemove.Key)
		{
			Found = true;
			Actions.RemoveAt(v);
			v = 0;
			continue;
		}
	}

	if (Found)
	{
		//SAVES TO DISK
		Settings->SaveKeyMappings();

		//REBUILDS INPUT, creates modified config in Saved/Config/Windows/Input.ini
		for (TObjectIterator<UPlayerInput> It; It; ++It)
		{
			It->ForceRebuildingKeyMaps(true);
		}
	}
}

void UDisplaceExtendedLibraryBPLibrary::VictoryGetAllAxisAndActionMappingsForKey(FKey Key, TArray<FDisplaceInput>& ActionBindings, TArray<FDisplaceInputAxis>& AxisBindings)
{
	ActionBindings.Empty();
	AxisBindings.Empty();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	if (!Settings) return;

	const TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	for (const FInputActionKeyMapping& Each : Actions)
	{
		if (Each.Key == Key)
		{
			ActionBindings.Add(FDisplaceInput(Each));
		}
	}

	const TArray<FInputAxisKeyMapping>& Axi = Settings->AxisMappings;

	for (const FInputAxisKeyMapping& Each : Axi)
	{
		if (Each.Key == Key)
		{
			AxisBindings.Add(FDisplaceInputAxis(Each));
		}
	}
}
bool UDisplaceExtendedLibraryBPLibrary::VictoryReBindAxisKey(FDisplaceInputAxis Original, FDisplaceInputAxis NewBinding)
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	if (!Settings) return false;

	TArray<FInputAxisKeyMapping>& Axi = Settings->AxisMappings;

	//~~~

	bool Found = false;
	for (FInputAxisKeyMapping& Each : Axi)
	{
		//Search by original
		if (Each.AxisName.ToString() == Original.AxisName &&
			Each.Key == Original.Key
			) {
			//Update to new!
			UDisplaceExtendedLibraryBPLibrary::UpdateAxisMapping(Each, NewBinding);
			Found = true;
			break;
		}
	}

	if (Found)
	{
		//SAVES TO DISK
		const_cast<UInputSettings*>(Settings)->SaveKeyMappings();

		//REBUILDS INPUT, creates modified config in Saved/Config/Windows/Input.ini
		for (TObjectIterator<UPlayerInput> It; It; ++It)
		{
			It->ForceRebuildingKeyMaps(true);
		}
	}
	return Found;
}

bool UDisplaceExtendedLibraryBPLibrary::VictoryReBindActionKey(FDisplaceInput Original, FDisplaceInput NewBinding)
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	if (!Settings) return false;

	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	//~~~

	bool Found = false;
	for (FInputActionKeyMapping& Each : Actions)
	{
		//Search by original
		if (Each.ActionName.ToString() == Original.ActionName &&
			Each.Key == Original.Key
			) {
			//Update to new!
			UDisplaceExtendedLibraryBPLibrary::UpdateActionMapping(Each, NewBinding);
			Found = true;
			break;
		}
	}

	if (Found)
	{
		//SAVES TO DISK
		const_cast<UInputSettings*>(Settings)->SaveKeyMappings();

		//REBUILDS INPUT, creates modified config in Saved/Config/Windows/Input.ini
		for (TObjectIterator<UPlayerInput> It; It; ++It)
		{
			It->ForceRebuildingKeyMaps(true);
		}
	}
	return Found;
}

FVector UDisplaceExtendedLibraryBPLibrary::ZeroVector(FVector Vector, bool X, bool Y, bool Z)
{
	if (X)
	{
		Vector.X = 0.0f;
	}
	if (Y)
	{
		Vector.Y = 0.0f;
	}
	if (Z)
	{
		Vector.Z = 0.0f;
	}
	return Vector;
}

FRotator UDisplaceExtendedLibraryBPLibrary::ZeroRot(FRotator StartingRotation, bool X, bool Y, bool Z)
{
	if (X)
	{
		StartingRotation.Pitch = 0.0f;
	}
	if (Y)
	{
		StartingRotation.Yaw = 0.0f;
	}
	if (Z)
	{
		StartingRotation.Roll = 0.0f;
	}
	return StartingRotation;
}


/* void GetSensesConfig(UAIPerceptionComponent* AIPerceptionComponent, TArray<UAISenseConfig*>& SensesConfig)
{
	SensesConfig = AIPerceptionComponent->SensesConfig;
} */




//void UDisplaceExtendedLibraryBPLibrary::ChangePerceptionRadius(AAIController* AIController, float NewRange)
void UDisplaceExtendedLibraryBPLibrary::ChangePerceptionSightRadius(UAIPerceptionComponent* PerceptionComponent, float NewRange)
{
	FAISenseID Id = UAISense::GetSenseID(UAISense_Sight::StaticClass());

	if (!Id.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Wrong Sense ID"));
		return;
	}

	//UAIPerceptionComponent* PerceptionComp = AIController->GetPerceptionComponent();
	auto Config = PerceptionComponent->GetSenseConfig(Id);

	if (Config == nullptr)
		return;

	auto ConfigSight = Cast<UAISenseConfig_Sight>(Config);

	// Save original lose range
	float LoseRange = ConfigSight->LoseSightRadius - ConfigSight->SightRadius;

	ConfigSight->SightRadius = NewRange;

	// Apply lose range to new radius of the sight
	ConfigSight->LoseSightRadius = ConfigSight->SightRadius + LoseRange;

	PerceptionComponent->RequestStimuliListenerUpdate();
}

void UDisplaceExtendedLibraryBPLibrary::GetPerceptionSightRadius(UAIPerceptionComponent* PerceptionComponent, bool& Successful, float& SightRadius, float&LoseSightRadius)
{
	FAISenseID Id = UAISense::GetSenseID(UAISense_Sight::StaticClass());

	if (!Id.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Wrong Sense ID"));
		return;
	}

	//UAIPerceptionComponent* PerceptionComp = AIController->GetPerceptionComponent();
	auto Config = PerceptionComponent->GetSenseConfig(Id);

	if (Config == nullptr)
	{
		Successful = false;
		return;
	}

	auto ConfigSight = Cast<UAISenseConfig_Sight>(Config);

	// Save original lose range
	SightRadius = ConfigSight->SightRadius;
	LoseSightRadius = ConfigSight->LoseSightRadius;

	Successful = true;
}