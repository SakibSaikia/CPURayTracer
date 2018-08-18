#include "quasi-random.h"

float Random::HaltonSample(uint64_t sampleIndex, uint32_t base)
{
	float result = 0.f;
	float f = 1.f;

	while (sampleIndex > 0)
	{
		f = f / base;
		result += f * (sampleIndex % base);
		sampleIndex = sampleIndex / base;
	}

	return result;
}

XMFLOAT2 Random::HaltonSample2D(uint64_t sampleIndex, uint32_t base1, uint32_t base2)
{
	return XMFLOAT2(
		Random::HaltonSample(sampleIndex, base1),
		Random::HaltonSample(sampleIndex, base2)
	);
}

XMFLOAT2 Random::HaltonSampleRing(uint64_t sampleIndex, uint32_t base)
{
	float theta = 2.f * XM_PI * Random::HaltonSample(sampleIndex, base);
	return XMFLOAT2(
		std::cos(theta),
		std::sin(theta)
	);

}

XMFLOAT3 Random::HaltonSampleHemisphere(uint64_t sampleIndex, uint32_t base1, uint32_t base2)
{
	const float u1 = Random::HaltonSample(sampleIndex, base1);
	const float u2 = Random::HaltonSample(sampleIndex, base2);

	const float r = std::sqrt(1.f - u1*u1);
	const float phi = 2 * XM_PI * Random::HaltonSample(sampleIndex, base2);

	return XMFLOAT3(
		r * std::cos(phi),
		r * std::sin(phi),
		u1
	);

}

XMFLOAT2 Random::HaltonSampleDisk(uint64_t sampleIndex, uint32_t base1, uint32_t base2)
{
	float theta = 2.f * XM_PI * Random::HaltonSample(sampleIndex, base1);
	float r = Random::HaltonSample(sampleIndex, base2);

	return XMFLOAT2(
		r * std::cos(theta),
		r * std::sin(theta)
	);
}

// Xorshift PRNG 
// https://en.wikipedia.org/wiki/Xorshift
uint64_t Random::Xorshift()
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	const auto timeNow = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<uint64_t, std::nano> duration = (timeNow - startTime);
	uint64_t x = duration.count();
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;

	return x;
}