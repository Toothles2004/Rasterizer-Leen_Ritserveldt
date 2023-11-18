#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix
			right = Vector3::Cross(Vector3::UnitY, forward);
			right.Normalize();

			up = Vector3::Cross(forward, right);
			up.Normalize();

			invViewMatrix = Matrix{ right, up, forward, origin } * Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw);
			viewMatrix = invViewMatrix.Inverse();

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
			Matrix::CreateLookAtLH(origin, forward, up);
		}

		void CalculateProjectionMatrix()
		{
			//TODO W3

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			//Camera Update Logic
			//...

			const float deltaTime = pTimer->GetElapsed();
			const float initFov = fov;

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			const bool keysUsed =
			(
				pKeyboardState[SDL_SCANCODE_UP] |
				pKeyboardState[SDL_SCANCODE_DOWN] |
				pKeyboardState[SDL_SCANCODE_W] |
				pKeyboardState[SDL_SCANCODE_S] |
				pKeyboardState[SDL_SCANCODE_D] |
				pKeyboardState[SDL_SCANCODE_A] |
				pKeyboardState[SDL_SCANCODE_RIGHT] |
				pKeyboardState[SDL_SCANCODE_LEFT]
			);

			if (keysUsed || mouseState & SDL_BUTTON_RMASK || mouseState & SDL_BUTTON_LMASK)
			{
				constexpr float moveSpeed{ 10.f };
				constexpr float rotSpeed{ 10.f * TO_RADIANS };
				//constexpr float fovSpeed{ 1500.f * TO_RADIANS };

				const bool keyUp = pKeyboardState[SDL_SCANCODE_UP] + pKeyboardState[SDL_SCANCODE_W];
				const bool keyDown = pKeyboardState[SDL_SCANCODE_DOWN] + pKeyboardState[SDL_SCANCODE_S];
				const bool keyLeft = pKeyboardState[SDL_SCANCODE_LEFT] + pKeyboardState[SDL_SCANCODE_A];
				const bool keyRight = pKeyboardState[SDL_SCANCODE_RIGHT] + pKeyboardState[SDL_SCANCODE_D];

				const bool buttonLeft = static_cast<bool>(mouseState & SDL_BUTTON_LMASK);
				const bool buttonRight = static_cast<bool>(mouseState & SDL_BUTTON_RMASK);


				//CALCULATE CAMERA MOVEMENT:
				origin += 
					moveSpeed *                           //Multiply the movement vectors with the speed
					deltaTime *
					(
						//Calculate forward
						(
							(buttonLeft * mouseY *				//MBL: move forward using only mouseY
							(buttonRight - 1)) +				//(Not when MBR is pressed)									
							(keyUp) -							//[UP] & [W] : move forward
							(keyDown)
						) *										//[DOWN] & [S] : move backwards
						forward +

						//Calculate right
						((keyRight) -							//[RIGHT] & [D] : move right
						(keyLeft)) *							//[LEFT] & [A] : move left
						right +

						//Calculate up
						(buttonLeft & buttonRight) *			//MBL & MRB : move up using mouse y
						-mouseY *
						up
					);

				//CALCULATE CAMERA ROTATION:
				totalYaw += rotSpeed * deltaTime * (mouseX * (buttonLeft ^ buttonRight));				//MBL xor MBR:  move camera yaw using mouseX
				totalPitch += rotSpeed * deltaTime * (-mouseY) * (buttonRight);							//MBR : move camera pitch using mouseY

				forward = 
					Matrix::CreateRotation(
					totalPitch,
					totalYaw,
					0).TransformVector(Vector3::UnitZ);	// Matrix magic

				forward.Normalize();
			}

			//checks for angle change
			if (std::abs(initFov - fovAngle) >= 0.00001f)
			{
				fov = std::atanf(fovAngle);
			}

			//Update Matrices
			CalculateViewMatrix();
			if(abs(initFov - fov) > 0.0001f)
			{
				CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
			}
		}
	};
}
