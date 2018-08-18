#pragma once

#include "stdafx.h"

namespace Random
{
	float HaltonSample(uint64_t sampleIndex, uint32_t base);
	XMFLOAT2 HaltonSample2D(uint64_t sampleIndex, uint32_t base1, uint32_t base2);
	XMFLOAT2 HaltonSampleRing(uint64_t sampleIndex, uint32_t base);
	XMFLOAT2 HaltonSampleDisk(uint64_t sampleIndex, uint32_t base1, uint32_t base2);
	XMFLOAT3 HaltonSampleHemisphere(uint64_t sampleIndex, uint32_t base1, uint32_t base2);

	uint64_t Xorshift();
};
