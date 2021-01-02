// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VC_Adventure.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "GI_Adventure.h"
#include "Adventure.h"


void UVC_Adventure::PostRender(UCanvas* Canvas)
{
	Super::PostRender(Canvas);

	// Fade if requested, you could use the same DrawScreenFade method from any canvas such as the HUD
	if (m_bFading)
	{
		DrawScreenFade(Canvas);
	}
}

void UVC_Adventure::ClearFade()
{
	m_bFading = false;
}

void UVC_Adventure::Fade(const float Duration, const bool bToBlack, const bool bLoadNewLevel)
{
	// const UWorld* World = GetWorld();
	if (World)
	{
		m_bFading = true;
		m_bLoadNewLevel = bLoadNewLevel;
		m_bToBlack = bToBlack;
		m_FadeDuration = Duration;
		m_FadeStartTime = World->GetTimeSeconds();
	}
}

void UVC_Adventure::DrawScreenFade(UCanvas* Canvas)
{
	if (m_bFading)
	{
		const UWorld* world = GetWorld();
		if (world)
		{
			const float Time = world->GetTimeSeconds();
			const float Alpha = FMath::Clamp((Time - m_FadeStartTime) / m_FadeDuration, 0.f, 1.f);

			// Make sure that we stay black in a fade to black
			if (Alpha == 1.f && !m_bToBlack)
			{
				m_bFading = false;
			}
			else
			{
				FColor OldColor = Canvas->DrawColor;
				FLinearColor FadeColor = FLinearColor::Black;
				FadeColor.A = m_bToBlack ? Alpha : 1 - Alpha;
				Canvas->DrawColor = FadeColor.ToFColor(true); // TheJamsh: "4.10 cannot convert directly to FColor, so need to use FLinearColor::ToFColor() :)
				Canvas->DrawTile(Canvas->DefaultTexture, 0, 0, Canvas->ClipX, Canvas->ClipY, 0, 0, Canvas->DefaultTexture->GetSizeX(), Canvas->DefaultTexture->GetSizeY());
				Canvas->DrawColor = OldColor;

				if (m_bLoadNewLevel && Alpha == 1.f)
				{
					UE_LOG(LogNotice, Warning, TEXT("<ViewportClient>: Begining map transition."));
					m_bLoadNewLevel = false;
					UGI_Adventure* gameInstance = Cast<UGI_Adventure>(GetGameInstance());
					if (gameInstance)
					{
						m_bLoadNewLevel = false;
						gameInstance->LoadNextMap();
					}
				}
			}
		}
	}
}