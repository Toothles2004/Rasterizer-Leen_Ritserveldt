#pragma once
#include "Maths.h"
#include "vector"

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		ColorRGB color{colors::White};
		//Vector2 uv{}; //W2
		//Vector3 normal{}; //W4
		//Vector3 tangent{}; //W4
		//Vector3 viewDirection{}; //W4
	};

	struct Vertex_Out
	{
		Vector4 position{};
		ColorRGB color{ colors::White };
		//Vector2 uv{};
		//Vector3 normal{};
		//Vector3 tangent{};
		//Vector3 viewDirection{};
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	struct Mesh
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		std::vector<Vertex_Out> vertices_out{};
		Matrix worldMatrix{};
	};

	struct TriangleBoundingBox
	{
		Vector2 min{};
		Vector2 max{};

		bool IsPointInBoundingBox(const Vector2& pos) const
		{
			return ((pos.x >= min.x) && (pos.y >= min.y) && (pos.x <= max.x) && (pos.y <= max.y));
		}

		void CalculateBoundingBox(const Vector2& pos)
		{
			min.x = std::min(min.x, pos.x);
			min.y = std::min(min.y, pos.y);
			max.x = std::max(max.x, pos.x);
			max.y = std::max(max.y, pos.y);
			
		}
	};
}
