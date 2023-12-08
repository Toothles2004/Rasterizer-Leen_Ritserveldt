#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	struct Vertex_Out;
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;
	struct TriangleBoundingBox;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;

		void VertexTransformationFunction(Mesh& mesh) const;
		void VertexTransformationFunction(std::vector<Mesh>& meshes) const;

		ColorRGB PixelShading(const Vertex_Out& v);

		void ToggleDepthBuffer();
		void ToggleRotate();
		void ToggleNormalMapping();
		void CycleShadingMode();

	private:
		enum class ShadingMode
		{
			observedArea,
			diffuse,
			specular,
			combined,
			number
		};

		ShadingMode m_CurrentShadingMode{ShadingMode::combined };

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};

		Texture* m_pTextureDiffuse{};
		Texture* m_pTextureGloss{};
		Texture* m_pTextureNormal{};
		Texture* m_pTextureSpecular{};

		std::vector<Mesh> m_Meshes;

		bool m_DepthBufferOn;
		bool m_RotatingOn;
		bool m_NormalMappingOn;

		ColorRGB m_Ambient;
		float m_Shininess;
	};
}
