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

	Vertex v0{ {0.f, 0.5f, 1.f} };
	Vertex v1{ {0.5f, -0.5f, 1.f} };
	Vertex v2{ {-0.5f, -0.5f, 1.f} };

	std::vector<Vector2> screenSpaceVertex
	{
		{((v0.position.x + 1) / 2) * m_Width, ((1 - v0.position.y) / 2) * m_Height},
		{((v1.position.x + 1) / 2) * m_Width, ((1 - v1.position.y) / 2) * m_Height},
		{((v2.position.x + 1) / 2) * m_Width, ((1 - v2.position.y) / 2) * m_Height},
	};

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float pixelColor = 0;
			Vector2 pixel{ px + 0.5f, py + 0.5f };
			bool hitAll{ true };
			for(int index{}; index < static_cast<int>(screenSpaceVertex.size()); ++index)
			{
				Vector2 vertexVector{ screenSpaceVertex[(index + 1) % 3] - screenSpaceVertex[index] };
				Vector2 pixelVector{ pixel - screenSpaceVertex[index] };
				if(Vector2::Cross(vertexVector, pixelVector) <= 0)
				{
					hitAll = false;
				}
			}

			if(hitAll)
			{
				pixelColor = 255;
			}

			ColorRGB finalColor{ pixelColor, pixelColor, pixelColor };

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
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
