#include "quasi-random.h"

float Halton::Sample(int sampleIndex, int base)
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

//XMVECTOR Halton::Sample2D(int sampleIndex, int base1, int base2)
//{
//
//}
//
//XMVECTOR Halton::SampleUnitCircle(int sampleIndex, int base)
//{
//
//}
//
//XMVECTOR Halton::SampleUnitSphere(int sampleIndex, int base1, int base2)
//{
//
//}
//
//XMVECTOR Halton::SampleDisk(float radius, int sampleIndex, int base1, int base2)
//{
//
//}