#pragma once
#include "Maths.h"
#include "DataTypes.h"
#include "BRDFs.h"

namespace dae
{
#pragma region Material BASE
	class Material
	{
	public:
		Material() = default;
		virtual ~Material() = default;

		Material(const Material&) = delete;
		Material(Material&&) noexcept = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) noexcept = delete;

		/**
		 * \brief Function used to calculate the correct color for the specific material and its parameters
		 * \param hitRecord current hitrecord
		 * \param l light direction
		 * \param v view direction
		 * \return color
		 */
		virtual ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) = 0;
	};
#pragma endregion

#pragma region Material SOLID COLOR
	//SOLID COLOR
	//===========
	class Material_SolidColor final : public Material
	{
	public:
		Material_SolidColor(const ColorRGB& color) : m_Color(color)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord, const Vector3& l, const Vector3& v) override
		{
			return m_Color;
		}

	private:
		ColorRGB m_Color{ colors::White };
	};
#pragma endregion

#pragma region Material LAMBERT
	//LAMBERT
	//=======
	class Material_Lambert final : public Material
	{
	public:
		Material_Lambert(const ColorRGB& diffuseColor, float diffuseReflectance) :
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(diffuseReflectance) {}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor);
		}

	private:
		ColorRGB m_DiffuseColor{ colors::White };
		float m_DiffuseReflectance{ 1.f }; //kd
	};
#pragma endregion

#pragma region Material LAMBERT PHONG
	//LAMBERT-PHONG
	//=============
	class Material_LambertPhong final : public Material
	{
	public:
		Material_LambertPhong(const ColorRGB& diffuseColor, float kd, float ks, float phongExponent) :
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(kd), m_SpecularReflectance(ks),
			m_PhongExponent(phongExponent)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor)
				+ BRDF::Phong(m_SpecularReflectance, m_PhongExponent, l, -v, hitRecord.normal);
		}

	private:
		ColorRGB m_DiffuseColor{ colors::White };
		float m_DiffuseReflectance{ 0.5f }; //kd
		float m_SpecularReflectance{ 0.5f }; //ks
		float m_PhongExponent{ 1.f }; //Phong Exponent
	};
#pragma endregion

#pragma region Material COOK TORRENCE
	//COOK TORRENCE
	class Material_CookTorrence final : public Material
	{
	public:
		Material_CookTorrence(const ColorRGB& albedo, float metalness, float roughness) :
			m_Albedo(albedo), m_Metalness(metalness), m_Roughness(roughness)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			// Determine F0 value → (0.04, 0.04, 0.04) or Albedo based on Metalness
			ColorRGB f0{};
			if (m_Metalness == 1) 
				f0 = m_Albedo;
			else
				f0 = { 0.04f,0.04f,0.04f };

			// Calculate half vector between view direction and light direction
			const Vector3 h = (v + l).Normalized();

			// Calculate Fresnel (F)
			const ColorRGB F = BRDF::FresnelFunction_Schlick(h, v, f0);

			// Calculate Normal Distribution (D)
			const Vector3 n = hitRecord.normal.Normalized();
			const float D = BRDF::NormalDistribution_GGX(n, h, m_Roughness);

			// Calculate Geometry (G)
			const float G = BRDF::GeometryFunction_Smith(n, v, l, m_Roughness);

			// Calculate specular => Cook-Torrance → (DFG)/4(dot(v,n)dot(l,n))
			float vDOTn{ Vector3::Dot(v.Normalized(), n.Normalized())};
			vDOTn = std::max(vDOTn, 0.0001f);

			float lDOTn{ Vector3::Dot(l.Normalized(), n.Normalized())};
			lDOTn = std::max(lDOTn, 0.0001f);

			ColorRGB nom = D * F * G;
			float denom = 4 * vDOTn * lDOTn;
			ColorRGB specular = nom/denom;

			// Determine kd -> 1 - Fresnel, cancel out if it’s a metal (kd = 0)
			ColorRGB kd{0,0,0};
			if (m_Metalness == 0)  
				kd =  ColorRGB{1,1,1} - F ;

			// Calculate Diffuse => BRDF::Lambert using the kd
			ColorRGB diffuse = {};
			if (m_Metalness == 0)
				diffuse = BRDF::Lambert(kd, m_Albedo);

			// Return final color -> diffuse + specular
			ColorRGB finalColor{ diffuse + specular };
			finalColor.MaxToOne();
			return {finalColor };


		}

	private:
		ColorRGB m_Albedo{ 0.955f, 0.637f, 0.538f }; //Copper
		float m_Metalness{ 1.0f };
		float m_Roughness{ 0.1f }; // [1.0 > 0.0] >> [ROUGH > SMOOTH]
	};
#pragma endregion
}
