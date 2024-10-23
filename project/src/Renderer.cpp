//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <iostream>

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const Matrix cameraToWorld = camera.CalculateCameraToWorld();

	float aspectRatio{m_Width/float(m_Height)};
	float FOV{ tan((pScene->GetCamera().fovAngle) / 2) };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			// float gradient = px / static_cast<float>(m_Width);
			// gradient += py / static_cast<float>(m_Height);
			// gradient /= 2.0f;

			float xNdc{ ((2 * (px + 0.5f) / m_Width) - 1) * FOV * aspectRatio };
			float yNdc{ (1 - (2 * (py + 0.5f) / m_Height)) * FOV };

			Vector3 rayDirection{ xNdc , yNdc, 1 };
			rayDirection = cameraToWorld.TransformVector(rayDirection);
			rayDirection.Normalize();


			//For Each pixel...
			// ... Ray Direction calculations above ...
			// Ray we are casting from the camera towards each pixel
			Ray viewRay{ camera.origin ,rayDirection };

			// Color to write to the color buffer (default = black)
			ColorRGB finalColor{ 0,0,0 };

			//HitRecord containing more info about potential hit
			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);
			
			if (closestHit.didHit)
			{
				// T_value visualization
				// const float scaled_t = closestHit.t / 500.f;
				// finalColor = { scaled_t,scaled_t,scaled_t };
			
				// If we hit something, set finalColor to material color, else keep BLACK
				// Use HitRecord::materialIndex to find the corresponding material
				Ray lightRay{};
			
				for (int indexLights{}; indexLights < lights.size(); ++indexLights)
				{
					
					lightRay.origin = closestHit.origin + (closestHit.normal*0.01f);
					lightRay.direction = LightUtils::GetDirectionToLight(pScene->GetLights()[indexLights], lightRay.origin);
					
					lightRay.min = 0.0001f;
					lightRay.max = lightRay.direction.Magnitude();
				
					lightRay.direction.Normalize(); 
					

					 // BRDFs!
					 ColorRGB Ergb{ LightUtils::GetRadiance(pScene->GetLights()[indexLights],closestHit.origin) };
					 closestHit.normal.Normalize();

					 float lambertsCos = Vector3::Dot(closestHit.normal, lightRay.direction);
					 lambertsCos = std::max(lambertsCos, 0.0001f);

					 ColorRGB BRDFrgb = materials[closestHit.materialIndex]->Shade(closestHit,lightRay.direction, viewRay.direction);
					 
					 switch (m_CurrentLightingMode)
					 {
					 case dae::Renderer::LightingMode::ObservedArea:
					 	if (lambertsCos > 0)
					 		finalColor += { lambertsCos,lambertsCos,lambertsCos };
					 	break;
					 case dae::Renderer::LightingMode::Radiance:
					 	finalColor += Ergb;
					 	break;
					 case dae::Renderer::LightingMode::BRDF:
						 finalColor += BRDFrgb;
					 	break;
					 case dae::Renderer::LightingMode::Combined:
						 finalColor += Ergb * BRDFrgb  * lambertsCos;
					 	break;
					 }

					// HARD SHADOW
					if(m_ShadowsEnabled)
					{
						if (pScene->DoesHit(lightRay))
						{
							finalColor *= 0.5;
						}
					}
					
					
				}
			}
			

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));

			


		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
	case dae::Renderer::LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		break;
	case dae::Renderer::LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::BRDF;
		break;
	case dae::Renderer::LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::Combined;
		break;
	case dae::Renderer::LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		break;
	}
}
