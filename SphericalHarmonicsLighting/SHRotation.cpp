#include "SHRotation.h"

#include "SphericalSamples.h"

#define R_INDEX(m, n) = (3*(m + 1) + n + 1)

struct Matrix {
public:
	inline Matrix(int l) : l(l), size(2*l + 1) {
		data = new float[size * size];
	}

	inline ~Matrix() {
		if (data) {
			delete[] data;
		}
	}

	inline void set(int row, int col, float value) {
		row += l;
		col += l;

		assert(row >= 0 && row < size);
		assert(col >= 0 && col < size);

		data[row * size + col] = value;
	}

	inline float operator()(int row, int col) const {
		row += l;
		col += l;

		if (row >= 0 && row < size && col >= 0 && col < size) {
			return data[row * size + col];
		}

		return 0.0f;
	}

	inline void operator=(Matrix& other) {
		l    = other.l;
		size = other.size;

		if (data) {
			delete[] data;
		}

		data = other.data;
		other.data = NULL;
	}

private:
	int l;
	int size;
	float* data;
};

inline float delta(int i, int j) {
	return i == j ? 1.0f : 0.0f;
}

inline float u(int l, int m, int n) {
	if (abs(n) < l) {
		return sqrt(
			(float)((l + m) * (l - m)) /
			(float)((l + n) * (l - n))
		);
	} else {
		return sqrt(
			(float)((l + m) * (l - m)) /
			(float)((2*l) * (2*l - 1))
		);
	}
}

inline float v(int l, int m, int n) {
	if (abs(n) < l) {
		return 0.5f * sqrt(
			(float)((1.0f + delta(m, 0)) * (l + abs(m) - 1) * (l + abs(m))) /
			(float)((l + n) * (l - n))) *
				(1.0f - 2.0f * delta(m, 0));
	} else {
		return 0.5f * sqrt(
			(float)((1.0f + delta(m, 0)) * (l + abs(m) - 1) * (l + abs(m))) /
			(float)((2*l) * (2*l - 1))) *
				(1.0f - 2.0f * delta(m, 0));
	}
}

// @NOTE: conflicting formulas papers "SH Ligting: the gritty details" (Green) and
// "Rotation Matrices for Real Spherical Harmonics. Direct Determination by Recursion" (Ivanic)
inline float w(int l, int m, int n) {
	if (abs(n) < l) {
		return -0.5f * sqrt(
			(float)((l - abs(m) - 1) * (l - abs(m))) /
			(float)((l + n) * (l - n))) *
				(1.0f - delta(m, 0));
	} else {
		return -0.5f * sqrt(
			(float)((l + abs(m) - 1) * (l + abs(m))) /
			(float)((2*l) * (2*l - 1))) *
				(1.0f - delta(m, 0));
	}
}

inline float P(const Matrix& R, const Matrix& prev_M, int l, int i, int a, int b) {
	if (abs(b) < l) {
		return R(i,  0) * prev_M(a, b);
	} else if (b == l) {
		return R(i,  1) * prev_M(a,  l - 1) -
			   R(i, -1) * prev_M(a, -l + 1);
	} else if (b == -l) {
		return R(i,  1) * prev_M(a, -l + 1) +
			   R(i, -1) * prev_M(a,  l - 1);
	}
}

inline float U(const Matrix& R, const Matrix& prev_M, int l, int m, int n) {
	return P(R, prev_M, l, 0, m, n);
}

inline float V(const Matrix& R, const Matrix& prev_M, int l, int m, int n) {
	if (m == 0) {
		return P(R, prev_M, l, 1, 1, n) + P(R, prev_M, l, -1, -1, n);
	} else if (m > 0) {
		return P(R, prev_M, l,  1,  m - 1, n) * sqrt(1.0f + delta(m,  1)) -
			   P(R, prev_M, l, -1, -m + 1, n) *     (1.0f - delta(m,  1));
	} else if (m < 0) {
		return P(R, prev_M, l,  1,  m + 1, n) *     (1.0f - delta(m, -1)) +
			   P(R, prev_M, l, -1, -m - 1, n) * sqrt(1.0f - delta(m, -1));
	}
}

inline float W(const Matrix& R, const Matrix& prev_M, int l, int m, int n) {
	if (m == 0) {
		return 0.0f;
	} else if (m > 0) {
		return P(R, prev_M, l, 1, m + 1, n) + P(R, prev_M, l, -1, -m - 1, n);
	} else if (m < 0) {
		return P(R, prev_M, l, 1, m - 1, n) - P(R, prev_M, l, -1, -m + 1, n);
	}
}

void rotate(glm::quat& rotation, const float coeffs_in[], float coeffs_out[]) {
	// Convert the Quaternion into Matrix form
	glm::mat3 rotation_matrix = glm::mat3_cast(rotation);

	// Permute the Rotation Matrix and put it into a 3x3 Matrix that we can acces using indices -1, 0, 1
	Matrix R(1);
	R.set(-1, -1, rotation_matrix[1][1]); R.set(-1, 0, rotation_matrix[2][1]); R.set(-1, 1, rotation_matrix[0][1]);
	R.set( 0, -1, rotation_matrix[1][2]); R.set( 0, 0, rotation_matrix[2][2]); R.set( 0, 1, rotation_matrix[0][2]);
	R.set( 1, -1, rotation_matrix[1][0]); R.set( 1, 0, rotation_matrix[2][0]); R.set( 1, 1, rotation_matrix[0][0]);

	// First harmonic remains unchanged
	coeffs_out[0] = coeffs_in[0];

	Matrix prev_M(0);
	prev_M.set(0, 0, 1.0f);

	// Iterate over bands
	for (int l = 1; l < SH_NUM_BANDS; l++) {
		Matrix M(l);

		// Create a 2l+1 x 2l+1 Rotation Matrix to rotate the current band
		for (int m = -l; m <= l; m++) {
			for (int n = -l; n <= l; n++) {
				M.set(m , n,
					u(l, m, n) * U(R, prev_M, l, m, n) +
					v(l, m, n) * V(R, prev_M, l, m, n) +
					w(l, m, n) * W(R, prev_M, l, m, n)
				);
			}
		}

		// Matrix multiply
		for (int i = 0; i < 2*l + 1; i++) {
			float sum = 0.0f;

			for (int j = 0; j < 2*l + 1; j++) {
				sum += M(i - l, j - l) * coeffs_in[l*l + j];
			}

			coeffs_out[l*l + i] = sum;
		}

		prev_M = M;
	}
}