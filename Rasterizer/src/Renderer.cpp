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

	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");
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

	//const std::vector<Vertex> triangle
	//{
	//	//Triangle 0
	//	{{0.f, 2.f, 0.f}, {1, 0, 0}},
	//	{{1.5f, -1.f, 0.f}, {1, 0, 0}},
	//	{{-1.5f, -1.f, 0.f}, {1, 0, 0}},

	//	//Triangle 1
	//	{{0.f, 4.f, 2.f}, {1, 0, 0}},
	//	{{3.0f, -2.f, 2.f}, {0, 1, 0}},
	//	{{-3.f, -2.f, 2.f}, {0, 0, 1}}
	//};

	//const std::vector<Vertex> triangle
	//{
	//	{{meshesWorld[3].position}, {1, 1, 1}}, //Triangle 0 (3,0,4)
	//	{{meshesWorld[0].position}, {1, 1, 1}}, //Triangle 1 (0, 4, 1)
	//	{{meshesWorld[4].position}, {1, 1, 1}}, //Triangle 2 (4, 1, 5)
	//	{{meshesWorld[1].position}, {1, 1, 1}}, //Triangle 3 (1, 5, 2)
	//	{{meshesWorld[5].position}, {1, 1, 1}},
	//	{{meshesWorld[2].position}, {1, 1, 1}},
	//	{{meshesWorld[2].position}, {1, 1, 1}},

	//	{{meshesWorld[6].position}, {1, 1, 1}},
	//	{{meshesWorld[6].position}, {1, 1, 1}}, //Triangle 4 (6, 3, 7)
	//	{{meshesWorld[3].position}, {1, 1, 1}}, //Triangle 5 (3, 7, 4)
	//	{{meshesWorld[7].position}, {1, 1, 1}}, //Triangle 6 (7, 4, 8)
	//	{{meshesWorld[4].position}, {1, 1, 1}}, //Triangle 7 (4, 8, 5)
	//	{{meshesWorld[8].position}, {1, 1, 1}},
	//	{{meshesWorld[5].position}, {1, 1, 1}},
	//};

	//TriangleStrip
	const std::vector<Mesh> meshesWorldStrip
	{
		//Quad
		Mesh
		{
			{
			Vertex{{-3.f, 3.f, -2.f}, {}, {0, 0}},
			Vertex{{0.f, 3.f, -2.f}, {}, {0.5f, 0}},
			Vertex{{3.f, 3.f, -2.f}, {}, {1, 0}},
			Vertex{{-3.f, 0.f, -2.f}, {}, {0, 0.5f}},
			Vertex{{0.f, 0.f, -2.f}, {}, {0.5f, 0.5f}},
			Vertex{{3.f, 0.f, -2.f}, {}, {1, 0.5f}},
			Vertex{{-3.f, -3.f, -2.f}, {}, {0, 1}},
			Vertex{{0.f, -3.f, -2.f}, {}, {0.5, 1}},
			Vertex{{3.f, -3.f, -2.f}, {}, {1, 1}}
			},

			{
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},

			PrimitiveTopology::TriangleStrip
		}
	};

	//TriangleList
	const std::vector<Mesh> meshesWorldList
	{
		//Quad
		Mesh
		{
			{
			Vertex{{-3.f, 3.f, -2.f}, {}, { 0, 0}},
			Vertex{{0.f, 3.f, -2.f}, {}, {0.5f, 0}},
			Vertex{{3.f, 3.f, -2.f}, {}, {1, 0}},
			Vertex{{-3.f, 0.f, -2.f}, {}, {0, 0.5f}},
			Vertex{{0.f, 0.f, -2.f}, {}, {0.5f, 0.5f}},
			Vertex{{3.f, 0.f, -2.f}, {}, {1, 0.5f}},
			Vertex{{-3.f, -3.f, -2.f}, {}, {0, 1}},
			Vertex{{0.f, -3.f, -2.f}, {}, {0.5, 1}},
			Vertex{{3.f, -3.f, -2.f}, {}, {1, 1}}
			},

			{
				3, 0, 1,	1, 4, 3,	4, 1, 2,
				2, 5, 4,	6, 3, 4,	4, 7, 6,
				7, 4, 5,	5, 8, 7
			},

			PrimitiveTopology::TriangleList
		}
	};

	std::vector<Mesh> transformMesh{ };
	VertexTransformationFunction(meshesWorldStrip, transformMesh);

	//RENDER LOGIC
	for (auto& mesh : transformMesh)
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
			
			//Create triangle vectors
			const Vector2 v0{ mesh.vertices[vertex0].position.x, mesh.vertices[vertex0].position.y };
			const Vector2 v1{ mesh.vertices[vertex1].position.x, mesh.vertices[vertex1].position.y };
			const Vector2 v2{ mesh.vertices[vertex2].position.x, mesh.vertices[vertex2].position.y };

			const float totalTriangleArea{ Vector2::Cross(v1 - v0, v2 - v0) / 2 };

			//Create bounding box
			TriangleBoundingBox boundingBox{};
			boundingBox.CalculateBoundingBox(v0);
			boundingBox.CalculateBoundingBox(v1);
			boundingBox.CalculateBoundingBox(v2);

			//Do pixel loop
			for (int px{}; px < m_Width; ++px)
			{
				for (int py{}; py < m_Height; ++py)
				{
					
					Vector2 pixel{ px + 0.5f, py + 0.5f };

					//Ignore and continue if pixel is not in the bounding box
					if (!(boundingBox.IsPointInBoundingBox(pixel)))
					{
						continue;
					}

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

					pixelDepth = 1.f / 
						(
							(weightV0 / mesh.vertices[vertex0].position.z) + 
							(weightV1 / mesh.vertices[vertex1].position.z) + 
							(weightV2 / mesh.vertices[vertex2].position.z)
						);

					//If the z point is not closer in this triangle check the next triangle
					if (!(pixelDepth <= m_pDepthBufferPixels[px + (py * m_Width)]))
					{
						continue;
					}

					//Set z buffer to closer point
					m_pDepthBufferPixels[px + (py * m_Width)] = pixelDepth;

					pixelUV =
						(
							((mesh.vertices[vertex0].uv * weightV0) / mesh.vertices[vertex0].position.z) +
							((mesh.vertices[vertex1].uv * weightV1) / mesh.vertices[vertex1].position.z) +
							((mesh.vertices[vertex2].uv * weightV2) / mesh.vertices[vertex2].position.z)
						) * pixelDepth;

					pixelUV.x = std::clamp(pixelUV.x, 0.f, 1.f);
					pixelUV.y = std::clamp(pixelUV.y, 0.f, 1.f);

					ColorRGB finalColor{ m_pTexture->Sample(pixelUV) };

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

void Renderer::VertexTransformationFunction(const std::vector<Mesh>& mesh_in, std::vector<Mesh>& mesh_out) const
{
	mesh_out = mesh_in;
	for(int index{}; index < mesh_in.size(); ++index)
	{
		mesh_out[index].vertices = {};
		VertexTransformationFunction(mesh_in[index].vertices, mesh_out[index].vertices);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
