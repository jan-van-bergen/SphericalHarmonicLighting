#include "SphericalSamples.h"

#include <random>

#include "Util.h"

// Converts l, m representation into a 1 dimensional index
#define SH_INDEX(l, m) (l * (l+1) + m)

// Precalculated factorial table, starts at 0! and goes up to 33!
float factorial[34] = {
	1.0f,  
	1.0f,
	2.0f,
	6.0f,
	24.0f,
	120.0f,
	720.0f,
	5040.0f,
	40320.0f,
	362880.0f,
	3628800.0f,
	39916800.0f,
	479001600.0f,
	6227020800.0f,
	87178291200.0f,
	1307674368000.0f,
	20922789888000.0f,
	355687428096000.0f,
	6402373705728000.0f,
	121645100408832000.0f,
	2432902008176640000.0f,
	51090942171709440000.0f,
	1124000727777607680000.0f,
	25852016738884976640000.0f,
	620448401733239439360000.0f,
	15511210043330985984000000.0f,
	403291461126605635584000000.0f,
	10888869450418352160768000000.0f,
	304888344611713860501504000000.0f,
	8841761993739701954543616000000.0f,
	265252859812191058636308480000000.0f,
	8222838654177922817725562880000000.0f,
	263130836933693530167218012160000000.0f,
	8683317618811886495518194401280000000.0f
};

// Evaluates the Associated Legendre Polynomial P(l,m,x) at x
float P(int l, int m, float x) {	
	// Apply rule 2; P m m
	float pmm = 1.0f;
	if (m > 0) {
		float somx2 = sqrt((1.0f - x) * (1.0f + x));
		float fact = 1.0f;

		for (int i = 1; i <= m; i++) {
			pmm *= (-fact) * somx2;
			fact += 2.0f;
		}
	}

	// If l is equal to m then P m m is already the right answer
	if (l == m) return pmm;

	// Use rule 3 once
	float pmmp1 = x * (2.0f * m + 1.0f) * pmm;
	if (l == m + 1) return pmmp1;

	// Iterate rule 1 until the right answer is found
	float pll = 0.0f;
	for (int ll = m + 2; ll <= l; ll++) {
		pll = ((2.0f * ll - 1.0f) * x * pmmp1 - (ll + m - 1.0f) * pmm) / (ll - m);
		pmm = pmmp1;
		pmmp1 = pll;
	}

	return pll;
}

// Renormalisation constant for SH function
float K(int l, int m) {
	return sqrt(
		((2.0f * l + 1.0f) * factorial[l - m]) / 
		(4.0f * PI * factorial[l + m])
	);
}

// Returns a point sample of a Spherical Harmonic basis function
// l is the band, range [0..N]
// m in the range [-l..l]
// theta in the range [0..Pi]
// phi in the range [0..2*Pi]
float SH(int l, int m, float theta, float phi) {
	if (m == 0) {
		return K(l, 0) * P(l, m, cos(theta));
	} else if(m > 0) {
		return sqrt(2.0f) * K(l,  m) * cos( m * phi) * P(l,  m, cos(theta));
	} else {
		return sqrt(2.0f) * K(l, -m) * sin(-m * phi) * P(l, -m, cos(theta));
	}
} 

// Fills an N*N array with uniformly distributed SH samples across the unit sphere, using jittered stratification
void init_samples(SH_Sample samples[], int sqrt_n_samples, int n_bands) {
	const float inv_sqrt_n_samples = 1.0f / sqrt_n_samples;

	std::random_device random_device;
	std::mt19937 gen(random_device());
	std::uniform_real_distribution<float> U01(0.0f, 1.0f);

	for (int i = 0; i < sqrt_n_samples; i++) {
		for (int j = 0; j < sqrt_n_samples; j++) {
			// Generate unbiased distribution of spherical coords
			float x = ((float)i + U01(gen)) * inv_sqrt_n_samples;
			float y = ((float)j + U01(gen)) * inv_sqrt_n_samples;

			// Convert x and y to Spherical Coordinates
			float theta = 2.0f * acos(sqrt(1.0f - x));
			float phi   = 2.0f * PI * y;
			
			int index = i * sqrt_n_samples + j;

			// Store polar coords
			samples[index].theta = theta;
			samples[index].phi   = phi;

			// Convert spherical coords to unit vector
			samples[index].direction = glm::vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

			// Precompute all SH coefficients for this sample
			for (int l = 0; l < n_bands; l++) {
				for (int m = -l; m <= l; m++) {
					samples[index].coeffs[SH_INDEX(l, m)] = SH(l, m, theta, phi);
				}
			}
		}
	}
}

// Projects a given polar function into Spherical Harmonic coefficients.
// This is done using Monte Carlo integration, using the samples provided in the samples array
void project_polar_function(PolarFunction polar_function, int n_samples, const SH_Sample samples[], glm::vec3 result[]) {
	// For each sample
	for (int i = 0; i < n_samples; i++) {
		float theta = samples[i].theta;
		float phi   = samples[i].phi;

		// For each SH coefficient
		for (int j = 0; j < SH_COEFFICIENT_COUNT; j++) {
			result[j] += polar_function(theta, phi) * samples[i].coeffs[j];
		}
	}

	//  Weighted by the surface area of a 3D unit sphere, divided by the number of samples
	const float factor = 4.0f * PI / n_samples;
	for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
		result[i] *= factor;
	}
}

// Formula from "An Efficient Representation for Irradiance Environment Maps" by Ramamoorthi and Hanrahan
void calc_phong_lobe_coeffs(float result[SH_NUM_BANDS]) {
	assert(SH_NUM_BANDS > 0);

	result[0] = PI;

	if (SH_NUM_BANDS == 1) return;

	result[1] = 2.0f * PI / 3.0f;

	// Even bands have a formula
	for (int l = 2; l < SH_NUM_BANDS; l += 2) {
		result[l] = 2.0f * PI * 
			(pow(-1.0f, (l >> 1) - 1) / (float)((l + 2) * (l - 1))) *
			((factorial[l]) / ((1 << l) * factorial[l >> 1] * factorial[l >> 1]));
	}

	// Odd bands (> 1) are zero
	for (int l = 3; l < SH_NUM_BANDS; l += 2) {
		result[l] = 0.0f;
	}
}
