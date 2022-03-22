#pragma once


struct Color8
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

struct Color32
{
	float R;
	float G;
	float B;
};

namespace Colors
{
	constexpr Color8 White8 = { 0xff, 0xff, 0xff };
	constexpr Color8 Black8 = { 0x00, 0x00, 0x00 };
	constexpr Color8 Red8 = { 0xff, 0x00, 0x00 };
	constexpr Color8 Green8 = { 0x00, 0xff, 0x00 };
	constexpr Color8 Blue8 = { 0x00, 0x00, 0xff };
	constexpr Color8 Orange8 = { 0xb0, 0x40, 0x10 };

	constexpr Color32 Black32 = { 0.f, 0.f, 0.f };
	constexpr Color32 White32 = { 1.f, 1.f, 1.f };
	constexpr Color32 Grey32 = { 0.5f, 0.5f, 0.5f };
	constexpr Color32 Orange32 = { 0.7f, 0.2f, 0.05f };
	constexpr Color32 Pink32 = { 0.7f, 0.2f, 0.5f };
	constexpr Color32 Green32 = { 0.f, 1.f, 0.f };
}