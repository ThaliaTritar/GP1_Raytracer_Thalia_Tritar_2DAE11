#pragma once
#include "Maths.h"
#include <iostream>

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			ColorRGB rho{ cd * kd };
			return rho / M_PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			ColorRGB rho{ cd * kd };
			return rho / M_PI;
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//CHECK!
			float nDOTv = Vector3::Dot(n.Normalized(), v.Normalized());
			//nDOTv = std::max(nDOTv, 0.0001f);

			Vector3 reflectVector = l.Normalized() - 2 * (nDOTv) * n.Normalized();

			float cosAlpha = Vector3::Dot(reflectVector.Normalized(), v.Normalized());
			//cosAlpha = std::max(cosAlpha, 0.0001f);
			
			return  ks * (pow(cosAlpha, exp))* ColorRGB{ 1.f,1.f,1.f };
		}	

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			float hDOTv = Vector3::Dot(h.Normalized(), v.Normalized());
			hDOTv=std::max(hDOTv, 0.0001f);
			ColorRGB F = f0 + (colors::White - f0) * (std::pow(1.f - hDOTv, 5.f));
			F.MaxToOne();
			ColorRGB poop = F;
			
			
			return	F;
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			float alpha = roughness * roughness;

			float nDOTh = Vector3::Dot(n.Normalized(), h.Normalized());
			nDOTh = std::max(nDOTh, 0.0001f);
		
			 float toBeSquared = (Square(nDOTh) * (Square(alpha) - 1) + 1);
			 
			 return Square(alpha) / (float(M_PI) * Square(toBeSquared));
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			float nDOTv = Vector3::Dot(n.Normalized(), v.Normalized());
			nDOTv = std::max(nDOTv, 0.0001f);

			float alpha = roughness * roughness;
			float k = (Square(alpha + 1)) / 8;

			float denom = nDOTv * (1 - k) + k;

			float G = nDOTv / denom;
			
			return G;


		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			return GeometryFunction_SchlickGGX(n, v, roughness) * GeometryFunction_SchlickGGX(n, l, roughness);
		}

	}
}