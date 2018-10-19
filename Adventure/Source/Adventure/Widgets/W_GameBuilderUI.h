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

UCLASS()
class ADVENTURE_API UW_GameBuilderUI : public UUserWidget
{
	GENERATED_BODY()
	

public:

	
};
