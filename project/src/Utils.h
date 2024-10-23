#pragma once
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			 hitRecord.didHit = false;
			 // if like seen in w01 slides, we replace the point in the definition of a sphere w the def of ray
			 // We get a quad function!!		ax^2+bx+c = 0
			 
			 // We calculate A B and C
			 float A{ Vector3::Dot(ray.direction,ray.direction)};
			 float B{ Vector3::Dot(	2 * (ray.direction)	,  (ray.origin - sphere.origin) )};
			 float C{ Vector3::Dot(	(ray.origin - sphere.origin) ,  ray.origin - sphere.origin) - (sphere.radius * sphere.radius)	};
			 
			 // We calculate the DISCRIMINANT, this tells us where the ray is vs the sphere
			 float discriminant{ (B*B) - (4 * A * C)};
			 
			 hitRecord.didHit = discriminant > 0;
			 // RETURNS BASED ON DISC
			 if (!ignoreHitRecord&&hitRecord.didHit)
			 {
			 	// hitRecord implement
			 	hitRecord.t = ((-B - sqrt(discriminant)) / (2 * A));
			 	if (hitRecord.t < ray.min)
			 	{
			 		hitRecord.t = ((-B + sqrt(discriminant)) / (2 * A));
			 	}	
			 
			 	hitRecord.origin = ray.origin + (hitRecord.t * ray.direction);
			 
			 	hitRecord.normal = hitRecord.origin - sphere.origin;
				hitRecord.normal.Normalize();
			 	hitRecord.materialIndex = sphere.materialIndex;
			 	
			 	// RETURN
				hitRecord.didHit = true;
			 }
			 return hitRecord.didHit;

		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			hitRecord.t = (dae::Vector3::Dot((plane.origin - ray.origin), plane.normal)) / (dae::Vector3::Dot(ray.direction, plane.normal));
			hitRecord.didHit = (hitRecord.t < ray.max && hitRecord.t > ray.min);
			 
			 //if (hitRecord.t < ray.min) hitRecord.t = FLT_MAX;
			 
			 if ((hitRecord.t >= ray.min) && (hitRecord.t <= ray.max))
			 {	
			 	hitRecord.origin = ray.origin + (hitRecord.t * ray.direction);
				hitRecord.normal = plane.normal;
			 	hitRecord.materialIndex = plane.materialIndex;
			 
			 	hitRecord.didHit = true;
			 	return hitRecord.didHit;
			 }
			 hitRecord.didHit=false;
			 return hitRecord.didHit;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			throw std::runtime_error("Not Implemented Yet");
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			throw std::runtime_error("Not Implemented Yet");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			if (light.type == LightType::Point)
			{
				return Vector3{ light.origin - origin };
			}
			else if (light.type == LightType::Directional)
			{
				// TODO !  ADD FUNCTIONALITY FOR DIRECTIONAL LIGHT
				Vector3 directionalLight = light.direction;
				directionalLight.Normalize();
				return directionalLight;
			}
			else return {};
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			if (light.type==LightType::Point)
			{
				Vector3 radiusVect{ light.origin - target };
				float radius{ radiusVect.Magnitude() };
				return (light.color * (light.intensity / (radius * radius)));
			}
			else
			{
				return light.color * light.intensity;
			}
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}