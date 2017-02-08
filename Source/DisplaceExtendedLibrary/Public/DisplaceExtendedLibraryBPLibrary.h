// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//AI
#include "AIController.h"		//MoveToWithFilter

#include "AISystem.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AIPerceptionComponent.h"

//Audio
#include "Components/AudioComponent.h"
#include "AudioDecompress.h"
#include "AudioDevice.h"
#include "ActiveSound.h"
#include "Audio.h"
#include "Developer/TargetPlatform/Public/Interfaces/IAudioFormat.h"
#include "VorbisAudioInfo.h"

//Texture2D
//#include "Engine/Texture2D.h"
#include "DDSLoader.h"

//Kris Nodes
#include "ImageUtils.h"
#include "ImageWrapper.h"	//requires "ImageWrapper" in public dependencies in build CS

#include "Engine.h"
#include "DisplaceExtendedLibraryBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

USTRUCT(BlueprintType)
struct FDisplaceInput
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		FString ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		FString KeyAsString;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		uint32 bShift : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		uint32 bCtrl : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		uint32 bAlt : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		uint32 bCmd : 1;


	FDisplaceInput() {}
	FDisplaceInput(const FString InActionName, const FKey InKey, const bool bInShift, const bool bInCtrl, const bool bInAlt, const bool bInCmd)
		: Key(InKey)
		, KeyAsString(Key.GetDisplayName().ToString())
		, bShift(bInShift)
		, bCtrl(bInCtrl)
		, bAlt(bInAlt)
		, bCmd(bInCmd)
	{
		ActionName = InActionName;
	}

	FDisplaceInput(const FInputActionKeyMapping& Action)
		: Key(Action.Key)
		, KeyAsString(Action.Key.GetDisplayName().ToString())
		, bShift(Action.bShift)
		, bCtrl(Action.bCtrl)
		, bAlt(Action.bAlt)
		, bCmd(Action.bCmd)
	{
		ActionName = Action.ActionName.ToString();
	}
};

USTRUCT(BlueprintType)
struct FDisplaceInputAxis
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		FString AxisName = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		FString KeyAsString = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Song")
		float Scale = 1;

	FDisplaceInputAxis() {}
	FDisplaceInputAxis(const FString InAxisName, FKey InKey, float InScale)
		: AxisName(InAxisName)
		, KeyAsString(InKey.GetDisplayName().ToString())
		, Key(InKey)
		, Scale(InScale)
	{ }

	FDisplaceInputAxis(const FInputAxisKeyMapping& Axis)
		: Key(Axis.Key)
		, KeyAsString(Axis.Key.GetDisplayName().ToString())
		, Scale(Axis.Scale)
	{
		AxisName = Axis.AxisName.ToString();
	}
};

UCLASS()
class UDisplaceExtendedLibraryBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

UFUNCTION(BlueprintPure, Category = "Displace Media|Key Rebinding")
static void VictoryGetAllAxisAndActionMappingsForKey(FKey Key, TArray<FDisplaceInput>& ActionBindings, TArray<FDisplaceInputAxis>& AxisBindings);

//	Axis Mapping
UFUNCTION(BlueprintPure, Category = "Displace Media|Key Rebinding")
static FDisplaceInputAxis VictoryGetVictoryInputAxis(const FKeyEvent& KeyEvent);

UFUNCTION(BlueprintPure, Category = "Displace Media|Key Rebinding")
static void VictoryGetAllAxisKeyBindings(TArray<FDisplaceInputAxis>& Bindings);

UFUNCTION(BlueprintCallable, Category = "Displace Media|Key Rebinding")
static void VictoryRemoveAxisKeyBind(FDisplaceInputAxis ToRemove);

/** You can leave the AsString field blank :) Returns false if the key could not be found as an existing mapping!  Enjoy! <3  Rama */
UFUNCTION(BlueprintCallable, Category = "Displace Media|Key Rebinding")
static bool VictoryReBindAxisKey(FDisplaceInputAxis Original, FDisplaceInputAxis NewBinding);

static FORCEINLINE void UpdateAxisMapping(FInputAxisKeyMapping& Destination, const FDisplaceInputAxis& VictoryInputBind)
{
	Destination.Key = VictoryInputBind.Key;
	Destination.Scale = VictoryInputBind.Scale;
}


//	Action Mapping
UFUNCTION(BlueprintPure, Category = "Displace Media|Key Rebinding")
static FDisplaceInput VictoryGetVictoryInput(const FKeyEvent& KeyEvent);

static FORCEINLINE void UpdateActionMapping(FInputActionKeyMapping& Destination, const FDisplaceInput& VictoryInputBind)
{
	Destination.Key = VictoryInputBind.Key;
	Destination.bShift = VictoryInputBind.bShift;
	Destination.bCtrl = VictoryInputBind.bCtrl;
	Destination.bAlt = VictoryInputBind.bAlt;
	Destination.bCmd = VictoryInputBind.bCmd;
}

UFUNCTION(BlueprintPure, Category = "Displace Media|Key Rebinding")
static void VictoryGetAllActionKeyBindings(TArray<FDisplaceInput>& Bindings);

/** You can leave the AsString field blank :) Returns false if the key could not be found as an existing mapping!  Enjoy! <3  Rama */
UFUNCTION(BlueprintCallable, Category = "Displace Media|Key Rebinding")
static bool VictoryReBindActionKey(FDisplaceInput Original, FDisplaceInput NewBinding);

UFUNCTION(BlueprintCallable, Category = "Displace Media|Key Rebinding")
static void VictoryRemoveActionKeyBind(FDisplaceInput ToRemove);

UFUNCTION(BlueprintCallable, Category = "Displace Media|Math")
static FVector ZeroVector(FVector Vector, bool X, bool Y, bool Z);

UFUNCTION(BlueprintCallable, Category = "Displace Media|Math")
static FRotator ZeroRot(FRotator StartingRotation, bool X, bool Y, bool Z);

/* UFUNCTION(BlueprintCallable, Category = "Displace Media|AI")
void GetSensesConfig(UAIPerceptionComponent* AIPerceptionComponent, TArray<UAISenseConfig*>& SensesConfig); */

UFUNCTION(BlueprintCallable, Category = "Displace Media|AI|Perception")
static void SetSightRadius(UAIPerceptionComponent* PerceptionComponent, float SightRadius, float LoseSightRadius);

UFUNCTION(BlueprintCallable, Category = "Displace Media|AI|Perception")
static void GetSightRadius(UAIPerceptionComponent* PerceptionComponent, bool& Successful, float& SightRadius, float& LoseSightRadius);
};
