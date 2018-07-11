// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "VC_Adventure.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "GI_Adventure.h"
#include "Adventure.h"


void UVC_Adventure::PostRender(UCanvas* Canvas)
{
	Super::PostRender(Canvas);

	// Fade if requested, you could use the same DrawScreenFade method from any canvas such as the HUD
	if (bFading)
	{
		DrawScreenFade(Canvas);
	}
}

void UVC_Adventure::ClearFade()
{
	bFading = false;
}

void UVC_Adventure::Fade(const float Duration, const bool bToBlack, const bool bLoadNewLevel)
{
	const UWorld* World = GetWorld();
	if (World)
	{
		bFading = true;
		this->bLoadNewLevel = bLoadNewLevel;
		this->bToBlack = bToBlack;
		FadeDuration = Duration;
		FadeStartTime = World->GetTimeSeconds();
	}
}

void UVC_Adventure::DrawScreenFade(UCanvas* Canvas)
{
	if (bFading)
	{
		const UWorld* World = GetWorld();
		if (World)
		{
			const float Time = World->GetTimeSeconds();
			const float Alpha = FMath::Clamp((Time - FadeStartTime) / FadeDuration, 0.f, 1.f);

			// Make sure that we stay black in a fade to black
			if (Alpha == 1.f && !bToBlack)
			{
				bFading = false;
			}
			else
			{
				FColor OldColor = Canvas->DrawColor;
				FLinearColor FadeColor = FLinearColor::Black;
				FadeColor.A = bToBlack ? Alpha : 1 - Alpha;
				Canvas->DrawColor = FadeColor.ToFColor(true); // TheJamsh: "4.10 cannot convert directly to FColor, so need to use FLinearColor::ToFColor() :)
				Canvas->DrawTile(Canvas->DefaultTexture, 0, 0, Canvas->ClipX, Canvas->ClipY, 0, 0, Canvas->DefaultTexture->GetSizeX(), Canvas->DefaultTexture->GetSizeY());
				Canvas->DrawColor = OldColor;

				if (bLoadNewLevel && Alpha == 1.f)
				{
					UE_LOG(LogNotice, Warning, TEXT("Begining no fade transition."));
					bLoadNewLevel = false;
					UGI_Adventure* GameInstance = Cast<UGI_Adventure>(GetGameInstance());
					if (GameInstance)
					{
						bLoadNewLevel = false;
						GameInstance->LoadNextMap();
					}
				}
			}
		}
	}
}