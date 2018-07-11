// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <list>
#include <map>
#include <vector>
#include "Basics.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_GameBuilderUI.generated.h"

#ifdef DELETE
#undef DELETE
#endif

/**
 * 
 */

UENUM(BlueprintType)
enum class GAMEBUILDER_ACTION : uint8
{
	DELETE,
	CREATE,
	MOVE
};

UENUM(BlueprintType)
enum class GAMEBUILDER_OBJECT_TYPE : uint8
{
	ANY,
	INTERACTABLE,
	SPAWN,
	NPC
};

USTRUCT(BlueprintType)
struct ADVENTURE_API FGAMEBUILDER_OBJECT
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER OBJECT")
	int ModelIndex;

	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER OBJECT")
	FGridCoordinate Location;

	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER OBJECT")
	GAMEBUILDER_OBJECT_TYPE Type;
};

USTRUCT(BlueprintType)
struct ADVENTURE_API FGAMEBUILDER_EDIT_EVENT
{
	GENERATED_BODY()
					
	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER EDIT EVENT")
	int ModelIndex;

	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER EDIT EVENT")
	GAMEBUILDER_ACTION Action;							    					    
														    
	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER EDIT EVENT")
	FGridCoordinate CurrentLocation;

	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER EDIT EVENT")
	FGridCoordinate DesiredLocation;
};

UCLASS()
class ADVENTURE_API UW_GameBuilderUI : public UUserWidget
{
	GENERATED_BODY()
	

public:

	virtual bool Initialize()override;
	virtual void BeginDestroy() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	//Returns the affected objects new ID (-1 if deleted or invalid)
	UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	bool SubmitEditEvent(GAMEBUILDER_OBJECT_TYPE Type, const FGAMEBUILDER_EDIT_EVENT& Event);

	//Returns the previous submitted action, (removes from log after)
	UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	FGAMEBUILDER_EDIT_EVENT GetPreviousEvent();

	//UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	//void GetInteractables(TArray<FGAMEBUILDER_OBJECT>& interactables)const;

	//UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	//void GetSpawns(TArray<FGAMEBUILDER_OBJECT>& spawns)const;

	//UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	//void GetNPCs(TArray<FGAMEBUILDER_OBJECT>& NPCs)const;

	UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	void SetMapName(const FName& MapName);

	UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	FName GetMapName();

	UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	void GetObjecteByCoordinate(const FGridCoordinate& coordinate, FGAMEBUILDER_OBJECT& Type)const;

	UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	void GetAllObjects(TArray<FGAMEBUILDER_OBJECT>& Objects)const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxLogEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowNameTags;

	UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	bool CreateGrid(const FGridCoordinate& Size);

	UFUNCTION(BlueprintCallable, Category = "GameBuilderUI")
	void ClearGrid();

private:
	bool DeleteObject(const FGridCoordinate& Location);
	bool CreateObject(const FGridCoordinate& Location, const FGAMEBUILDER_OBJECT& Object);
	void HandleLogOverflow();
	bool IsSpaceOccupied(const FGridCoordinate& coordinate, GAMEBUILDER_OBJECT_TYPE Type = GAMEBUILDER_OBJECT_TYPE::ANY);
	FGAMEBUILDER_OBJECT* At(const FGridCoordinate& Location)const;


	FName m_mapName;
	bool bAlreadyInit;
	FGridCoordinate GridSize;
	FGAMEBUILDER_OBJECT** m_LogicalMap;
	std::list<FGAMEBUILDER_EDIT_EVENT> m_editLog;

	//Categories
	//std::vector<FGAMEBUILDER_OBJECT> m_npcs;
	//std::vector<FGAMEBUILDER_OBJECT> m_spawns;
	//std::vector<FGAMEBUILDER_OBJECT> m_interactables;
	
};
