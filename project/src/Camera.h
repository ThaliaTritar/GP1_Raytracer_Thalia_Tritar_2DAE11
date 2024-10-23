#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};

		Matrix CalculateCameraToWorld()
		{
			Vector3 worldUp{Vector3::UnitY};

			right = Vector3::Cross(worldUp, forward);
			right.Normalize();
			up = Vector3::Cross(forward, right);
			up.Normalize();

			cameraToWorld = {
				{ right.x,	right.y,	right.z,	0},
				{ up.x,		up.y,		up.z,		0},
				{forward.x,	forward.y,	forward.z,	0},
				{ origin.x,	origin.y,	origin.z,	0}
			};

			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			const float moveSpeed{5.f};
			const float rotationSpeed{5.f};
			
			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if(pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * deltaTime * moveSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin += right * deltaTime * moveSpeed * -1;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{	
				origin += forward * deltaTime * moveSpeed * -1;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * deltaTime * moveSpeed;
			}


			//Mouse Input
			
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			
			Matrix finalRotation{};

			if (mouseState == 1)
			{
				totalYaw += float(mouseX) / 360 * float(M_PI);
				origin -= forward * mouseY * 0.2f;

				finalRotation = Matrix::CreateRotation(totalPitch, totalYaw, 0);

				forward = finalRotation.TransformVector(Vector3::UnitZ);
				forward.Normalize();
				
			}
			else if (mouseState == 4)
			{
				totalYaw += float(mouseX) / 360 * float(M_PI);
				totalPitch += float(mouseY) / 360 * float(M_PI);

				finalRotation = Matrix::CreateRotation(totalPitch, totalYaw, 0);

				forward = finalRotation.TransformVector(Vector3::UnitZ);
				forward.Normalize();

			}
			
		}
	};
}
