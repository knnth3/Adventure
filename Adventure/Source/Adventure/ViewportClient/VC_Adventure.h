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

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "VC_Adventure.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API UVC_Adventure : public UGameViewportClient
{
	GENERATED_BODY()
	
public: 

	/**
	* Called after rendering the player views and HUDs to render menus, the console, etc.
	* This is the last rendering call in the render loop
	*
	* @param Canvas        The canvas to use for rendering.
	*/
	virtual void PostRender(UCanvas* Canvas) override;

	/** Clear fading state */
	virtual void ClearFade();

	/** Used for Fade to and from black */
	virtual void Fade(const float Duration, const bool bToBlack, const bool bLoadNewLevel = false);

	/** Does the actual screen fading */
	void DrawScreenFade(UCanvas* Canvas);

private:

	bool m_bLoadNewLevel = 0;
	bool m_bFading = 1;
	bool m_bToBlack = 1;
	float m_FadeAlpha;
	float m_FadeStartTime;
	float m_FadeDuration;	
};
