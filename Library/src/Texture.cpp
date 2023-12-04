#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
		m_Red = new Uint8();
		m_Green = new Uint8();
		m_Blue = new Uint8();
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)

		return new Texture(IMG_Load(path.c_str()));
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO

		const int rangeU{ static_cast<int>(uv.x * m_pSurface->w)};
		const int rangeV{ static_cast<int>(uv.y * m_pSurface->h)};

		const Uint32 pixel{ m_pSurfacePixels[(rangeV*m_pSurface->w) + rangeU] };

		//Sample the correct texel for the given uv
		SDL_GetRGB(pixel, m_pSurface->format, m_Red, m_Green, m_Blue);

		return {static_cast<float>(*m_Red) / 255.f, static_cast<float>(*m_Green) / 255.f, static_cast<float>(*m_Blue) / 255.f};
	}
}