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

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	/*Vertex v0{ {0.f, 0.5f, 1.f} };
	Vertex v1{ {0.5f, -0.5f, 1.f} };
	Vertex v2{ {-0.5f, -0.5f, 1.f} };*/

	const std::vector<Vertex> triangle{ {{0.f, 2.f, 0.f}}, {{1.f, 0.f, 0.f}}, {{-1.f, 0.f, 0.f}} };

	std::vector<Vertex> transformTriangle{};

	VertexTransformationFunction(triangle, transformTriangle);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			ColorRGB pixelColor = colors::Black;
			Vector2 pixel{ px + 0.5f, py + 0.5f };
			bool hitAll{ true };

			for (int index{}; index < static_cast<int>(transformTriangle.size()); ++index)
			{
				Vector2 vertexVector
				{
					transformTriangle[(index + 1) % 3].position.x - transformTriangle[index].position.x,
					transformTriangle[(index + 1) % 3].position.y - transformTriangle[index].position.y
				};

				Vector2 pixelVector
				{
					pixel.x - transformTriangle[index].position.x,
					pixel.y - transformTriangle[index].position.y
				};

				if (Vector2::Cross(vertexVector, pixelVector) <= 0)
				{
					hitAll = false;
				}
			}

			if (hitAll)
			{
				pixelColor = colors::White;
			}

			ColorRGB finalColor{ pixelColor };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
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

		//pespective divide = View space -> clipping space (NDC)
		tempVertex.x /= tempVertex.z;
		tempVertex.y /= tempVertex.z;

		//NDC -> screen space
		const Vertex screenSpaceVertex
		{
			{((tempVertex.x + 1) / 2) * m_Width, ((1 - tempVertex.y) / 2) * m_Height, 0}
		};

		vertices_out.push_back(screenSpaceVertex);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
