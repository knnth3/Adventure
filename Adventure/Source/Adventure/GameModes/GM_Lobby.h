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

protected:

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby Gamemode")
	void OnGameStart();
	
private:

	uint32_t m_playerCount;

	//Menu
	class UW_Lobby* m_LobbyMenu;
	TSubclassOf<class UW_Lobby> MenuClass;
};
