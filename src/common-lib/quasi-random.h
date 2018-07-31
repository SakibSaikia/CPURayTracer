#pragma once

#include "stdafx.h"

namespace Halton
{
	float Sample(int sampleIndex, int base);
	XMVECTOR Sample2D(int sampleIndex, int base1, int base2);
	XMVECTOR SampleUnitCircle(int sampleIndex, int base);
	XMVECTOR SampleUnitSphere(int sampleIndex, int base1, int base2);
	XMVECTOR SampleDisk(float radius, int sampleIndex, int base1, int base2);
};
