// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GM_Lobby.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API AGM_Lobby : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	AGM_Lobby();

	UFUNCTION(BlueprintCallable, Category = "Lobby Gamemode")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Lobby Gamemode")
	void SetMapToLoad(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Lobby Gamemode")
	void GetMapToLoad(FString& Name)const;

	//Menu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby Gamemode")
	TSubclassOf<class UW_Lobby> DefaultLobbyUIClass;

	UFUNCTION(BlueprintImplementableEvent, Category = "Multiplayer")
	void OnServerTravelRequested(const FString& MapName);

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
private:

	uint32_t m_playerCount;
	FString m_MapSaveName;
};
