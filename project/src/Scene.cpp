#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene() :
		m_Materials({ new Material_SolidColor({1,0,0}) })
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for (auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		float smallestT{ closestHit.t };
		int closestIdx{-1};
		
		/////////////
		// SPHERE
		/////////////
		
		for (int sphereIndex{}; sphereIndex < m_SphereGeometries.size(); ++sphereIndex)
		{
			GeometryUtils::HitTest_Sphere(m_SphereGeometries[sphereIndex], ray, closestHit);
			if (closestHit.t < smallestT&& closestHit.t>0)
			{
				smallestT = closestHit.t;
				closestIdx = sphereIndex;
			}
		}
		
		///////////
		// PLANE
		///////////
		bool planeCloser{false};
		
		for (int planeIndex{}; planeIndex < m_PlaneGeometries.size(); ++planeIndex)
		{
			GeometryUtils::HitTest_Plane(m_PlaneGeometries[planeIndex], ray, closestHit);
			if (closestHit.t < smallestT&& closestHit.t>0)
			{
				smallestT = closestHit.t;
				closestIdx = planeIndex;
				planeCloser = true;
			}
		}
		
		////////////
		// TRIANGLE
		////////////
		
		
		
		///////////
		// check
		///////////
		if(closestIdx!=-1)
		{
			if (planeCloser) GeometryUtils::HitTest_Plane(m_PlaneGeometries[closestIdx], ray, closestHit);
			else   GeometryUtils::HitTest_Sphere(m_SphereGeometries[closestIdx], ray, closestHit);
		}
		
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		for (int sphereIndex{}; sphereIndex < m_SphereGeometries.size(); ++sphereIndex)
		{
	
			// We calculate A B and C
			float A{ Vector3::Dot(ray.direction,ray.direction) };
			float B{ Vector3::Dot(2 * (ray.direction)	,  ray.origin - m_SphereGeometries[sphereIndex].origin) };
			float C{ Vector3::Dot(ray.origin - m_SphereGeometries[sphereIndex].origin ,
				ray.origin - m_SphereGeometries[sphereIndex].origin) - (m_SphereGeometries[sphereIndex].radius * m_SphereGeometries[sphereIndex].radius) };
	
			// We calculate the DISCRIMINANT, this tells us where the ray is vs the sphere
			float discriminant{ (B * B) - (4 * A * C) };
	
			if (discriminant > 0)
			{
				float t = ((-B - sqrt(discriminant)) / (2 * A));

				if (t < ray.max)
				{
					if (t > ray.min)
					{
						return true;
					}
				}
			}
		}
	
		for (int planeIndex{}; planeIndex < m_PlaneGeometries.size(); ++planeIndex)
		{
			if (GeometryUtils::HitTest_Plane(m_PlaneGeometries[planeIndex], ray)) return true;
		}
	 	
	 	return false;
	 }

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma endregion

#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f,3.f,-9.f };
		m_Camera.fovAngle = 45.f;

		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);

		//Light 
		AddPointLight({ 0.f,5.f,-5.f }, 70, colors::White);
		AddPointLight({ 0.f,5.f,5.f }, 70, colors::White);
		
	}
#pragma endregion

#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		
		m_Camera.origin = { 0.f,3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f,.915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f,.915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f,.915f }, 1.f, 1.f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f,.75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f,.75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f,.75f }, .0f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f,.57f,.57f }, 1.f));

		// PLANE
		AddPlane(Vector3{0.f,0.f,10.f}, Vector3{0.f,0.f,-1.f}, matLambert_GrayBlue);	// back
		AddPlane(Vector3{0.f,0.f,0.f}, Vector3{0.f,1.f,0.f}, matLambert_GrayBlue);		// bottom
		AddPlane(Vector3{0.f,10.f,0.f}, Vector3{0.f,-1.f,0.f}, matLambert_GrayBlue);	// top
		AddPlane(Vector3{5.f,0.f,0.f}, Vector3{-1.f,0.f,0.f}, matLambert_GrayBlue);		// right
		AddPlane(Vector3{-5.f,0.f,0.f}, Vector3{1.f,0.f,0.f}, matLambert_GrayBlue);		// left

		//// Temporary Lambert-Phong Spheres & Materials
		//const auto matLambertPhong1 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 3.f));
		//const auto matLambertPhong2 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 15.f));
		//const auto matLambertPhong3 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 50.f));
		//
		//AddSphere(Vector3{ -1.75,1.f,0.f }, .75f, matLambertPhong1);
		//AddSphere(Vector3{ 0.f,1.f,0.f }, .75f, matLambertPhong2);
		//AddSphere(Vector3{ 1.75,1.f,0.f }, .75f, matLambertPhong3);

		// SPHERE
		 AddSphere(Vector3{-1.75,1.f,0.f}, .75f,matCT_GrayRoughMetal );
		 AddSphere(Vector3{ 0.f,1.f,0.f}, .75f,matCT_GrayMediumMetal );	
		 AddSphere(Vector3{1.75,1.f,0.f}, .75f,matCT_GraySmoothMetal );
		 AddSphere(Vector3{-1.75,3.f,0.f}, .75f,matCT_GrayRoughPlastic );
		 AddSphere(Vector3{0.f,3.f,0.f}, .75f, matCT_GrayMediumPlastic);
		 AddSphere(Vector3{1.75,3.f,0.f}, .75f, matCT_GraySmoothPlastic);

		// LIGHT
		AddPointLight(Vector3{ 0.f,5.f,5.f }, 50.f, ColorRGB{ 1.f,.61f,.45f });		// backlight
		AddPointLight(Vector3{ -2.5,5.f,-5.f }, 70.f, ColorRGB{ 1.f,.8f,.45f });	// front light L
		AddPointLight(Vector3{ 2.5f,2.5f,-5.f }, 50.f, ColorRGB{ .34f,.47f,.68f });	// front light R
		

		
		// //////////
		// // TEST SCENE
		// /////////	
		   
		// m_Camera.origin = { 0.f, 1.f, -5.f };
		// m_Camera.fovAngle = 45.f;
		   
		// //default: Material id0 >> SolidColor Material (RED)
		// const auto matLambert_Red = AddMaterial	(new Material_Lambert(colors::Red,1.f));
		// const auto matLambertPhong_Blue = AddMaterial(new Material_LambertPhong(colors::Blue, 1.f,1.f,60.f));
		// const auto matLambert_Yellow = AddMaterial(new Material_Lambert(colors::Yellow, 1.f));
		   
		// // Spheres
		// AddSphere({ -.75f, 1.f, 0.f }, 1.f, matLambert_Red);
		// AddSphere({ .75f, 1.f, 0.f }, 1.f, matLambertPhong_Blue);
		   
		// // Plane
		// AddPlane(Vector3{ 0.f,0.f,0.f }, Vector3{ 0.f,1.f,0.f }, matLambert_Yellow);
		   
		// // Light
		// AddPointLight({ 0.f,5.f,5.f }, 25.f, colors::White);
		// AddPointLight({ 0.f,2.5f,-5.f }, 25.f, colors::White);
		
	}
#pragma endregion
}
