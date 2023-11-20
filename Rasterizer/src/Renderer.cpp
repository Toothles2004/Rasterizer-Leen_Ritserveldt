//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f, -10.f });
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
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

	/*Vertex v0{ {0.f, 0.5f, 1.f} };
	Vertex v1{ {0.5f, -0.5f, 1.f} };
	Vertex v2{ {-0.5f, -0.5f, 1.f} };*/

	//const std::vector<Vertex> triangle{ {{0.f, 2.f, 0.f}}, {{1.f, 0.f, 0.f}}, {{-1.f, 0.f, 0.f}} };
	const std::vector<Vertex> triangle
	{
		//Triangle 0
		{{0.f, 2.f, 0.f}, {1, 0, 0}},
		{{1.5f, -1.f, 0.f}, {1, 0, 0}},
		{{-1.5f, -1.f, 0.f}, {1, 0, 0}},

		//Triangle 1
		{{0.f, 4.f, 2.f}, {1, 0, 0}},
		{{3.0f, -2.f, 2.f}, {0, 1, 0}},
		{{-3.f, -2.f, 2.f}, {0, 0, 1}}
	};

	std::vector<Vertex> transformTriangle{};

	VertexTransformationFunction(triangle, transformTriangle);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			for(int triangleIndex{}; triangleIndex < static_cast<int>(transformTriangle.size()); triangleIndex += 3)
			{
				float pixelDepth{};
				ColorRGB pixelColor{};
				Vector2 pixel{ px + 0.5f, py + 0.5f };
				bool hitAll{ true };

				const Vector2 vector1
				{
					transformTriangle[triangleIndex+1].position.x - transformTriangle[triangleIndex].position.x,
					transformTriangle[triangleIndex+1].position.y - transformTriangle[triangleIndex].position.y

				};
				const Vector2 vector2
				{
					transformTriangle[triangleIndex+2].position.x - transformTriangle[triangleIndex].position.x,
					transformTriangle[triangleIndex+2].position.y - transformTriangle[triangleIndex].position.y

				};
				const float totalTriangleArea{ Vector2::Cross(vector1, vector2) / 2 };

				for (int trianglePointIndex{}; trianglePointIndex < 3; ++trianglePointIndex)
				{
					const int indexP0{ triangleIndex + ((trianglePointIndex + 1) % 3) };
					const int indexP1{ triangleIndex + ((trianglePointIndex + 2) % 3) };
					const int indexP2{ triangleIndex + ((trianglePointIndex + 3) % 3) };

					Vector2 vertexVector
					{
						transformTriangle[indexP1].position.x - transformTriangle[indexP0].position.x,
						transformTriangle[indexP1].position.y - transformTriangle[indexP0].position.y
					};

					Vector2 pixelVector
					{
						pixel.x - transformTriangle[indexP0].position.x,
						pixel.y - transformTriangle[indexP0].position.y
					};

					float weight{ Vector2::Cross(vertexVector, pixelVector) / 2 };
					weight /= totalTriangleArea;

					if (weight <= 0)
					{
						hitAll = false;
					}

					pixelColor += transformTriangle[indexP2].color * weight;
					pixelDepth += transformTriangle[indexP2].position.z * weight;
				}

				if (!hitAll)
				{
					continue;
				}
				if(!(pixelDepth < m_pDepthBufferPixels[px+(py*m_Width)]))
				{
					continue;
				}

				m_pDepthBufferPixels[px + (py * m_Width)] = pixelDepth;

				ColorRGB finalColor{ pixelColor };

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage

	for(const auto& vertex : vertices_in)
	{
		//World -> view space
		Vector3 tempVertex = m_Camera.viewMatrix.TransformPoint(vertex.position) ;
		const float aspectRatio{ (static_cast<float>(m_Width) / static_cast<float>(m_Height)) };

		tempVertex.x /= aspectRatio  * m_Camera.fov;
		tempVertex.y /= m_Camera.fov;

		//pespective divide = View space -> clipping space (NDC)
		tempVertex.x /= tempVertex.z;
		tempVertex.y /= tempVertex.z;

		//NDC -> screen space
		Vertex screenSpaceVertex{ vertex };
		screenSpaceVertex.position =  { ((tempVertex.x + 1) / 2) * m_Width, ((1 - tempVertex.y) / 2) * m_Height, tempVertex.z } ;
		vertices_out.push_back(screenSpaceVertex);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
