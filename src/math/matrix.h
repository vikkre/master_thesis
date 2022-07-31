#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>
#include <type_traits>

#include "vector.h"

template <size_t s, typename T>
class Matrix {
	public:
		// Class Definitions

		class Row {
			public:
				Matrix& mat;
				const size_t i;

				Row(Matrix& mat, const size_t i): mat(mat), i(i) {}
				T& operator[](const size_t j) {
					return mat.m[i][j];
				}
		};

		class Const_Row {
			public:
				const Matrix& mat;
				const size_t i;

				Const_Row(const Matrix& mat, const size_t i): mat(mat), i(i) {}
				const T& operator[](const size_t j) const {
					return mat.m[i][j];
				}
		};

		// Matrix itself
		T m[s][s];

		// Constructors

		Matrix(): m{0} {
			for (size_t i = 0; i < s; ++i) {
				m[i][i] = 1.0f;
			}
		}

		Matrix(const Matrix& other) {
			for (size_t i = 0; i < s; ++i) {
				for (size_t j = 0; j < s; ++j) {
					this->m[i][j] = other.m[i][j];
				}
			}
		}

		Matrix& operator=(const Matrix& other) {
			for (size_t i = 0; i < s; ++i) {
				for (size_t j = 0; j < s; ++j) {
					this->m[i][j] = other.m[i][j];
				}
			}
			return *this;
		}

		// Access Operators

		Row operator[](const size_t i) {
			return Row(*this, i);
		}

		Const_Row operator[](const size_t i) const {
			return Const_Row(*this, i);
		}

		size_t size() const {
			return s;
		}

		// Math Operators

		Matrix operator+(const Matrix& other) const {
			Matrix ret(*this);
			for (size_t i = 0; i < s; ++i) {
				for (size_t j = 0; j < s; ++j) {
					ret.m[i][j] += other.m[i][j];
				}
			}
			return ret;
		}

		Matrix operator-(const Matrix& other) const {
			Matrix ret(*this);
			for (size_t i = 0; i < s; ++i) {
				for (size_t j = 0; j < s; ++j) {
					ret.m[i][j] -= other.m[i][j];
				}
			}
			return ret;
		}

		Matrix& operator+=(const Matrix& other) {
			for (size_t i = 0; i < s; ++i) {
				for (size_t j = 0; j < s; ++j) {
					this->m[i][j] += other.m[i][j];
				}
			}
			return *this;
		}

		Matrix& operator-=(const Matrix& other) {
			for (size_t i = 0; i < s; ++i) {
				for (size_t j = 0; j < s; ++j) {
					this->m[i][j] -= other.m[i][j];
				}
			}
			return *this;
		}

		template<typename S>
		Matrix& operator*=(const S scalar) {
			for (size_t i = 0; i < s; ++i) {
				for (size_t j = 0; j < s; ++j) {
					this->m[i][j] *= scalar;
				}
			}
			return *this;
		}

		template<size_t size = s>
		typename std::enable_if<(size > 2), Matrix<size-1, T>>::type
		subMatrix(size_t ri, size_t rj) const {
			Matrix<size-1, T> sub_mat;

			size_t i_sub = 1;
			for (size_t i = 1; i < size; ++i) {
				if ((i - 1) == ri) i_sub = 0;

				size_t j_sub = 1;
				for (size_t j = 1; j < size; ++j) {
					if ((j - 1) == rj) j_sub = 0;
					sub_mat[i-1][j-1] = m[i - i_sub][j - j_sub];
				}
			}

			return sub_mat;
		}

		Matrix& transpose() {
			for (size_t i = 1; i < s; ++i) {
				for (size_t j = 0; j < i; ++j) {
					T tmp = m[i][j];
					m[i][j] = m[j][i];
					m[j][i] = tmp;
				}
			}
			return *this;
		}

		template<size_t size = s>
		typename std::enable_if<(size == 2), Matrix>::type
		cofactorMatrix() {
			Matrix ret;

			T a11 = m[0][0];
			T a12 = m[0][1];
			T a21 = m[1][0];
			T a22 = m[1][1];

			ret[0][0] =  a22;
			ret[0][1] = -a12;
			ret[1][0] = -a21;
			ret[1][1] =  a11;

			return ret;
		}

		template<size_t size = s>
		typename std::enable_if<(size > 2), Matrix>::type
		cofactorMatrix() const {
			Matrix cof;

			for (size_t i = 0; i < s; ++i) {
				for (size_t j = 0; j < s; ++j) {
					cof[i][j] = subMatrix(j, i).det();
					if ((i + j) % 2 == 1) cof[i][j] *= -1;
				}
			}

			return cof;
		}

		template<size_t size = s>
		typename std::enable_if<(size == 2), T>::type
		det() const {
			return m[0][0] * m[1][1] - m[0][1] * m[1][0];
		}

		template<size_t size = s>
		typename std::enable_if<(size > 2), T>::type
		det() const {
			T det = 0;

			for (size_t i = 0; i < s; ++i) {
				Matrix<s-1, T> sub_mat = subMatrix(0, i);

				if (i%2 == 0) det += m[0][i] * sub_mat.det();
				else          det -= m[0][i] * sub_mat.det();
			}

			return det;
		}

		Matrix inverseMatrix() {
			return (T(1.0) / this->det()) * this->cofactorMatrix();
		}

		Matrix operator*(const Matrix& other) const {
			Matrix ret;
			for (size_t i = 0; i < s; ++i) {
				ret.m[i][i] = 0.0f;
				for (size_t j = 0; j < s; ++j) {
					for (size_t n = 0; n < s; ++n) {
						ret.m[i][j] += this->m[i][n] * other.m[n][j];
					}
				}
			}
			return ret;
		}

		Matrix& operator*=(const Matrix& other) {
			Matrix ret;
			for (size_t i = 0; i < s; ++i) {
				ret.m[i][i] = 0.0f;
				for (size_t j = 0; j < s; ++j) {
					for (size_t n = 0; n < s; ++n) {
						ret.m[i][j] += this->m[i][n] * other.m[n][j];
					}
				}
			}
			*this = ret;
			return *this;
		}

		// Vector matrix muliplication

		Vector<s, T> operator*(const Vector<s, T>& vector) const {
			Vector<s, T> ret;
			for (size_t i = 0; i < s; ++i) {
				for (size_t j = 0; j < s; ++j) {
					ret[i] += this->m[i][j] * vector[j];
				}
			}
			return ret;
		}
};

// Special functions

template<size_t s, typename T, typename S>
Matrix<s, T> operator*(const S scalar, const Matrix<s, T>& matrix) {
	Matrix<s, T> ret(matrix);
	for (size_t i = 0; i < s; ++i) {
		for (size_t j = 0; j < s; ++j) {
			ret.m[i][j] *= scalar;
		}
	}
	return ret;
}

// Typedefs

template <typename T>
using Matrix2 = Matrix<2, T>;
typedef Matrix2<int> Matrix2i;
typedef Matrix2<float> Matrix2f;
typedef Matrix2<double> Matrix2d;

template <typename T>
using Matrix3 = Matrix<3, T>;
typedef Matrix3<int> Matrix3i;
typedef Matrix3<float> Matrix3f;
typedef Matrix3<double> Matrix3d;

template <typename T>
using Matrix4 = Matrix<4, T>;
typedef Matrix4<int> Matrix4i;
typedef Matrix4<float> Matrix4f;
typedef Matrix4<double> Matrix4d;

// Predefined Matrices

Matrix4f getScaleMatrix(const Vector3f& scale);
Matrix4f getTranslationMatrix(const Vector3f& position);
Matrix4f getLookAtMatrix(const Vector3f& position, const Vector3f& lookAt, const Vector3f& up);
Matrix4f getPerspectiveMatrix(const float aspect, const float fieldOfView, const float nearZ, const float farZ, bool invertY=false);

#endif
