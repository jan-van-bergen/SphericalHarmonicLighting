#include "SHRotation.h"

#include "SphericalSamples.h"

// Square Matrix of size 2l + 1 for a given l
// Can be indexed using indices in the range [-l, l]
struct Matrix {
public:
	void set_order(int l) {
		this->l    = l;
		this->size = 2*l + 1;
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
		
		assert(row >= 0 && row < size);
		assert(col >= 0 && col < size);

		return data[row * size + col];
	}

private:
	int l;
	int size;
	float data[4 * SH_NUM_BANDS*SH_NUM_BANDS + 4*SH_NUM_BANDS + 1];
};

// Kronecker Delta
float delta(int i, int j) {
	return i == j ? 1.0f : 0.0f;
}

float u(int l, int m, int n) {
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

float v(int l, int m, int n) {
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

float w(int l, int m, int n) {
	if (abs(n) < l) {
		return -0.5f * sqrt(
			(float)((l - abs(m) - 1) * (l - abs(m))) /
			(float)((l + n) * (l - n))) *
				(1.0f - delta(m, 0));
	} else {
		return -0.5f * sqrt(
			(float)((l - abs(m) - 1) * (l - abs(m))) /
			(float)((2*l) * (2*l - 1))) *
				(1.0f - delta(m, 0));
	}
}

float P(const Matrix& R, const Matrix& prev_M, int l, int i, int a, int b) {
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

float U(const Matrix& R, const Matrix& prev_M, int l, int m, int n) {
	return P(R, prev_M, l, 0, m, n);
}

float V(const Matrix& R, const Matrix& prev_M, int l, int m, int n) {
	if (m == 0) {
		return P(R, prev_M, l, 1, 1, n) + P(R, prev_M, l, -1, -1, n);
	} else if (m > 0) {
		return P(R, prev_M, l,  1,  m - 1, n) * sqrt(1.0f + delta(m,  1)) -
			   P(R, prev_M, l, -1, -m + 1, n) *     (1.0f - delta(m,  1));
	} else if (m < 0) {
		// @NOTE: Both Green's and Ivanic' papers are wrong in this case!
		// Even in Ivanic' Errata this is still wrong
		return P(R, prev_M, l,  1,  m + 1, n) *     (1.0f - delta(m, -1)) +
			   P(R, prev_M, l, -1, -m - 1, n) * sqrt(1.0f + delta(m, -1));
	}
}

float W(const Matrix& R, const Matrix& prev_M, int l, int m, int n) {
	assert(m != 0);

	if (m > 0) {
		return P(R, prev_M, l, 1, m + 1, n) + P(R, prev_M, l, -1, -m - 1, n);
	} else {
		return P(R, prev_M, l, 1, m - 1, n) - P(R, prev_M, l, -1, -m + 1, n);
	}
}

void rotate(const glm::quat& rotation, const glm::vec3 coeffs_in[], glm::vec3 coeffs_out[]) {
	// Make sure the input and ouput arrays are not the same memory location
	assert(coeffs_in != coeffs_out);

	// Convert the Quaternion into Matrix form
	glm::mat3 rotation_matrix = glm::mat3_cast(rotation);

	// Permute the Rotation Matrix and put it into a 3x3 Matrix that we can acces using indices -1, 0, 1
	Matrix R;
	R.set_order(1);
	R.set(-1, -1, rotation_matrix[1][1]); R.set(-1, 0, rotation_matrix[2][1]); R.set(-1, 1, rotation_matrix[0][1]);
	R.set( 0, -1, rotation_matrix[1][2]); R.set( 0, 0, rotation_matrix[2][2]); R.set( 0, 1, rotation_matrix[0][2]);
	R.set( 1, -1, rotation_matrix[1][0]); R.set( 1, 0, rotation_matrix[2][0]); R.set( 1, 1, rotation_matrix[0][0]);

	// First harmonic remains unchanged
	coeffs_out[0] = coeffs_in[0];

	Matrix matrices[2];
	
	// Initialize first matrix as a 1x1 matrix containing 1
	matrices[0].set_order(0);
	matrices[0].set(0, 0, 1.0f);

	int current_index  = 1;
	int previous_index = 0;

	// Iterate over bands
	for (int l = 1; l < SH_NUM_BANDS; l++) {
		current_index = l & 1; // Modulo 2 by performing a bitwise AND with 1
		matrices[current_index].set_order(l);

		// Create a 2l+1 x 2l+1 Rotation Matrix to rotate the current band
		for (int m = -l; m <= l; m++) {
			for (int n = -l; n <= l; n++) {
				float u_ = u(l, m, n);
				float v_ = v(l, m, n);
				float w_ = w(l, m, n);

				float M_mn = 0.0f;
				// Only calulcate U,V,W if u,v,w are non-zero
				// Not only is this an optimization, U,V,W will index out of
				// bounds when they are called for any l,m,n that cause u,v,w to be 0
				if (u_) M_mn += u_ * U(R, matrices[previous_index], l, m, n);
				if (v_) M_mn += v_ * V(R, matrices[previous_index], l, m, n);
				if (w_) M_mn += w_ * W(R, matrices[previous_index], l, m, n);

				matrices[current_index].set(m , n, M_mn);
			}
		}

		// Matrix multiply
		for (int i = 0; i < 2*l + 1; i++) {
			glm::vec3 sum(0.0f, 0.0f, 0.0f);

			for (int j = 0; j < 2*l + 1; j++) {
				sum += matrices[current_index](i - l, j - l) * coeffs_in[l*l + j];
			}

			coeffs_out[l*l + i] = sum;
		}

		previous_index = current_index;
	}
}