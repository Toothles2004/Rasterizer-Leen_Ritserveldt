//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <execution>
#include <iostream>

#include "Maths.h"
#include "Texture.h"
#include "Utils.h"

#define PARALLEL_EXECUTION

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_DepthBufferOn(false),
	m_RotatingOn(true),
	m_NormalMappingOn(true),
	m_Ambient({ 0.03f, 0.03f, 0.03f }),
	m_Shininess(25.f)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(static_cast<float>(m_Width) / m_Height, 45.f, { .0f,.5f, -64.f });

	m_pTextureDiffuse = Texture::LoadFromFile("Resources/vehicle_diffuse.png");
	m_pTextureGloss = Texture::LoadFromFile("Resources/vehicle_gloss.png");
	m_pTextureNormal = Texture::LoadFromFile("Resources/vehicle_normal.png");
	m_pTextureSpecular = Texture::LoadFromFile("Resources/vehicle_specular.png");

	m_Meshes.push_back(Mesh{});
	m_Meshes[0].primitiveTopology = PrimitiveTopology::TriangleList;
	Utils::ParseOBJ("Resources/vehicle.obj", m_Meshes[0].vertices, m_Meshes[0].indices);
	m_Meshes[0].worldMatrix =
	{
		Matrix{}
	};
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
	if(m_RotatingOn)
	{
		m_Meshes[0].worldMatrix = Matrix::CreateRotationY(pTimer->GetTotal() / 2) * Matrix::CreateTranslation(0, 0, -40.f);
	}
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, Uint8{ 100 }, Uint8{ 100 }, Uint8{ 100 }));
	SDL_LockSurface(m_pBackBuffer);

	for(int index{}; index < m_Width * m_Height; ++index)
	{
		m_pDepthBufferPixels[index] = INFINITY;
	}

	VertexTransformationFunction(m_Meshes);

	//RENDER LOGIC
	for (auto& mesh : m_Meshes)
	{
		//Check all triangles
		for (int triangleIndex{}; triangleIndex < static_cast<int>(mesh.indices.size() - 2); ++triangleIndex)
		{
			uint32_t vertex0{ mesh.indices[triangleIndex] };
			uint32_t vertex1{ mesh.indices[triangleIndex + 1] };
			uint32_t vertex2{ mesh.indices[triangleIndex + 2] };

			//Do triangleStrip
			if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip)
			{
				//Check degenerate triangles
				if ((mesh.indices[vertex0] == mesh.indices[vertex1]) && (mesh.indices[vertex1] == mesh.indices[vertex2]))
				{
					continue;
				}

				//If triangleIndex is odd, swap 2nd and 3rd element
				if (((triangleIndex % 2) == 1))
				{
					std::swap(vertex1, vertex2);
				}
			}
			//Do triangleList
			else if(mesh.primitiveTopology == PrimitiveTopology::TriangleList)
			{
				triangleIndex += 2;
			}

			if
				(
					(mesh.vertices_out[vertex0].position.z < 0 || 1 < mesh.vertices_out[vertex0].position.z) ||
					(mesh.vertices_out[vertex1].position.z < 0 || 1 < mesh.vertices_out[vertex1].position.z) ||
					(mesh.vertices_out[vertex2].position.z < 0 || 1 < mesh.vertices_out[vertex2].position.z)
				)
			{
				continue;
			}
			
			//Create triangle vectors
			const Vector2 v0{ mesh.vertices_out[vertex0].position.x, mesh.vertices_out[vertex0].position.y };
			const Vector2 v1{ mesh.vertices_out[vertex1].position.x, mesh.vertices_out[vertex1].position.y };
			const Vector2 v2{ mesh.vertices_out[vertex2].position.x, mesh.vertices_out[vertex2].position.y };

			const float totalTriangleArea{ Vector2::Cross(v1 - v0, v2 - v0) / 2 };

			//Create bounding box
			const float minX =
				std::max(std::min(std::min(v0.x, v1.x), v2.x), 0.f);
			const float minY =
				std::max(std::min(std::min(v0.y, v1.y), v2.y), 0.f);

			const float maxX =
				std::min(std::max(std::max(v0.x, v1.x), v2.x), static_cast<float> (m_Width));
			const float maxY =
				std::min(std::max(std::max(v0.y, v1.y), v2.y), static_cast<float> (m_Height));

			//Do pixel loop
			for (int px{ static_cast<int>(minX) }; px < maxX; ++px)
			{
				for (int py{ static_cast<int>(minY) }; py < maxY; ++py)
				{
					Vector2 pixel{ px + 0.5f, py + 0.5f };

					float pixelDepth{};
					/*ColorRGB pixelColor{};*/
					Vector2 pixelUV{};

					//If pixel not in triangle skip to next pixel
					const float weightV0{ (Vector2::Cross(v2 - v1, pixel - v1) / 2.f) / totalTriangleArea };
					if (weightV0 < 0)
					{
						continue;
					}
					const float weightV1{ (Vector2::Cross(v0 - v2, pixel - v2) / 2.f) / totalTriangleArea };
					if (weightV1 < 0)
					{
						continue;
					}
					const float weightV2{ (Vector2::Cross(v1 - v0, pixel - v0) / 2.f) / totalTriangleArea };
					if (weightV2 < 0)
					{
						continue;
					}

					/*pixelColor =
						mesh.vertices[vertex0].color * weightV0 +
						mesh.vertices[vertex1].color * weightV1 +
						mesh.vertices[vertex2].color * weightV2;*/

					//Calculate the pixel depth
					pixelDepth = 1.f / 
						(
							(weightV0 * mesh.vertices_out[vertex0].position.w) + 
							(weightV1 * mesh.vertices_out[vertex1].position.w) +
							(weightV2 * mesh.vertices_out[vertex2].position.w)
						);
					const float interpolatedZ = 1.f /
						(
							((mesh.vertices_out[vertex0].position.z) * weightV0) +
							((mesh.vertices_out[vertex1].position.z) * weightV1) +
							((mesh.vertices_out[vertex2].position.z) * weightV2)
							);

					//If the z point is not closer in this triangle check the next triangle
					if (!(pixelDepth <= m_pDepthBufferPixels[px + (py * m_Width)]))
					{
						continue;
					}

					//Set z buffer to closer point
					m_pDepthBufferPixels[px + (py * m_Width)] = pixelDepth;

					//Calculate the pixel UV
					pixelUV =
						(
							((mesh.vertices_out[vertex0].uv * weightV0) * mesh.vertices_out[vertex0].position.w) +
							((mesh.vertices_out[vertex1].uv * weightV1) * mesh.vertices_out[vertex1].position.w) +
							((mesh.vertices_out[vertex2].uv * weightV2) * mesh.vertices_out[vertex2].position.w)
						) * pixelDepth;

					ColorRGB finalColor{};

					//show buffer depth with 0-1  greyscale if m_DepthBufferOn is on, otherwise show normal texture
					if(m_DepthBufferOn)
					{
						pixelDepth = Lerpf(1.f, 0.995f, pixelDepth);
						finalColor = ColorRGB(pixelDepth, pixelDepth, pixelDepth);
					}
					else
					{
						//Shading function
						Vertex_Out currentVertex{};
						currentVertex.uv = pixelUV;
						currentVertex.position =
						{
							pixel.x,
							pixel.y,
							interpolatedZ,
							pixelDepth
						};

						currentVertex.normal =
							(
								((mesh.vertices_out[vertex0].normal * weightV0) * mesh.vertices_out[vertex0].position.w) +
								((mesh.vertices_out[vertex1].normal * weightV1) * mesh.vertices_out[vertex1].position.w) +
								((mesh.vertices_out[vertex2].normal * weightV2) * mesh.vertices_out[vertex2].position.w)
							);
						currentVertex.normal.Normalize();

						currentVertex.tangent =
							(
								((mesh.vertices_out[vertex0].tangent * weightV0) * mesh.vertices_out[vertex0].position.w) +
								((mesh.vertices_out[vertex1].tangent * weightV1) * mesh.vertices_out[vertex1].position.w) +
								((mesh.vertices_out[vertex2].tangent * weightV2) * mesh.vertices_out[vertex2].position.w)
							);
						currentVertex.tangent.Normalize();

						currentVertex.viewDirection = 
							(
								((mesh.vertices_out[vertex0].viewDirection * weightV0) * mesh.vertices_out[vertex0].position.w) +
								((mesh.vertices_out[vertex1].viewDirection * weightV1) * mesh.vertices_out[vertex1].position.w) +
								((mesh.vertices_out[vertex2].viewDirection * weightV2) * mesh.vertices_out[vertex2].position.w)
							);

						finalColor = PixelShading(currentVertex);
					}

					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(Mesh& mesh) const
{
	//Todo > W1 Projection Stage
	mesh.vertices_out.clear();
	const Matrix worldViewProjectionMatrix{ mesh.worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };

	for(const auto& vertex : mesh.vertices)
	{
		//World -> view space
		Vertex_Out vertexOut{};
		vertexOut.position = worldViewProjectionMatrix.TransformPoint(vertex.position.x, vertex.position.y, vertex.position.z, 0);
		vertexOut.color = vertex.color;
		vertexOut.uv = vertex.uv;
		vertexOut.normal = mesh.worldMatrix.TransformVector(vertex.normal);
		vertexOut.tangent = mesh.worldMatrix.TransformVector(vertex.tangent);
		vertexOut.viewDirection = mesh.worldMatrix.TransformPoint(vertex.position) - m_Camera.origin;

		//pespective divide = View space -> clipping space (NDC)
		vertexOut.position.w = 1.f / vertexOut.position.w;
		vertexOut.position.x *= vertexOut.position.w;
		vertexOut.position.y *= vertexOut.position.w;
		vertexOut.position.z *= vertexOut.position.w;

		//NDC -> screen space
		vertexOut.position.x = { ((vertexOut.position.x + 1) * 0.5f) * static_cast<float>(m_Width) };
		vertexOut.position.y = { ((1 - vertexOut.position.y) * 0.5f) * static_cast<float>(m_Height) };

		mesh.vertices_out.push_back(vertexOut);
	}
}

void Renderer::VertexTransformationFunction(std::vector<Mesh>& meshes) const
{
	for(auto& mesh : meshes)
	{
		VertexTransformationFunction(mesh);
	}
}

ColorRGB Renderer::PixelShading(const Vertex_Out& v)
{
	const Vector3 lightDirection = { .577f, -.577f, .577f };
	const Vector3 binormal = Vector3::Cross(v.normal, v.tangent);
	const float lightIntensity{ 7.f };

	ColorRGB currentFinalColor{};

	
	ColorRGB glossSample = m_pTextureGloss->Sample(v.uv);
	ColorRGB specularSample = m_pTextureSpecular->Sample(v.uv);
	ColorRGB diffuseSample = m_pTextureDiffuse->Sample(v.uv);

	diffuseSample = (diffuseSample * lightIntensity) / PI;

	Matrix tangentSpaceAxis = Matrix{ v.tangent, binormal, v.normal, Vector3::Zero };
	Vector3 normals{};

	if(m_NormalMappingOn)
	{
		ColorRGB normalSample = m_pTextureNormal->Sample(v.uv);
		normalSample /= 255.f;
		normalSample *= 2.f;
		normalSample -= ColorRGB(1.f, 1.f, 1.f);

		normals = tangentSpaceAxis.TransformVector(Vector3(normalSample.r, normalSample.g, normalSample.b));
	}
	else
	{
		normals = v.normal;
	}

	const float observedArea{ Vector3::Dot(normals, lightDirection) };

	glossSample *= m_Shininess;
	specularSample = specularSample * powf(std::max(Vector3::Dot(lightDirection - (2.f * std::max(Vector3::Dot(normals, lightDirection), 0.f) * normals), v.viewDirection), 0.f), glossSample.r);
	specularSample.MaxToOne();

	
	switch (m_CurrentShadingMode)
	{
	case dae::Renderer::ShadingMode::observedArea:
		if (observedArea > 0.f)
		{
			currentFinalColor = ColorRGB(observedArea, observedArea, observedArea);
		}
		break;
	case dae::Renderer::ShadingMode::diffuse:
		currentFinalColor = diffuseSample + m_Ambient;
		break;
	case dae::Renderer::ShadingMode::specular:
		currentFinalColor = specularSample;
		break;
	case dae::Renderer::ShadingMode::combined:
		if (observedArea > 0.f)
		{
			currentFinalColor = observedArea * (diffuseSample + m_Ambient + specularSample);
		}
		break;
	default:
		break;
	}

	return currentFinalColor;
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void Renderer::ToggleDepthBuffer()
{
	m_DepthBufferOn = !m_DepthBufferOn;
}

void Renderer::ToggleRotate()
{
	m_RotatingOn = !m_RotatingOn;
}

void Renderer::ToggleNormalMapping()
{
	m_NormalMappingOn = !m_NormalMappingOn;
}

void Renderer::CycleShadingMode()
{
	m_CurrentShadingMode = static_cast<ShadingMode>((static_cast<int>(m_CurrentShadingMode) + 1) % static_cast<int>(ShadingMode::number));
}
