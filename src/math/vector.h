#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>

template <size_t s, typename T>
class Vector {
	public:
		// Vector itself
		T v[s];

		// Constructors

		Vector() : v{0} {}
		Vector(const T* vec) : v{0} {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] = vec[i];
			}
		}
		Vector(const std::vector<T> vec) : v{0} {
			for (size_t i = 0; i < s && i < vec.size(); ++i) {
				this->v[i] = vec[i];
			}
		}

		Vector(const Vector& other) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] = other.v[i];
			}
		}

		Vector& operator=(const Vector& other) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] = other.v[i];
			}
			return *this;
		}

		// Access Operators

		T& operator[](const size_t i) {
			return v[i];
		}

		const T& operator[](const size_t i) const {
			return v[i];
		}

		size_t size() const {
			return s;
		}

		// Math Operators with Scalar

		template<typename S>
		Vector& operator+=(const S scalar) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] += scalar;
			}
			return *this;
		}

		template<typename S>
		Vector& operator-=(const S scalar) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] -= scalar;
			}
			return *this;
		}

		template<typename S>
		Vector& operator*=(const S scalar) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] *= scalar;
			}
			return *this;
		}

		template<typename S>
		Vector& operator/=(const S scalar) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] /= scalar;
			}
			return *this;
		}

		// Math Operators with Vector

		Vector operator+(const Vector& other) const {
			Vector ret(*this);
			for (size_t i = 0; i < s; ++i) {
				ret.v[i] += other.v[i];
			}
			return ret;
		}

		Vector operator-(const Vector& other) const {
			Vector ret(*this);
			for (size_t i = 0; i < s; ++i) {
				ret.v[i] -= other.v[i];
			}
			return ret;
		}

		Vector& operator+=(const Vector& other) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] += other.v[i];
			}
			return *this;
		}

		Vector& operator-=(const Vector& other) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] -= other.v[i];
			}
			return *this;
		}

		// Compare operatores

		bool operator==(const Vector& other) const {
			bool equal = true;

			for (size_t i = 0; i < s; ++i) {
				equal = equal && (this->v[i] == other.v[i]);
			}

			return equal;
		}

		// additional math operators

		double magnitude() const {
			T sum = 0;
			for (size_t i = 0; i < s; ++i) {
				sum += this->v[i] * this->v[i];
			}
			return std::sqrt((double) sum);
		}

		Vector& normalize() {
			double magnitude = this->magnitude();

			if (magnitude != 0) {
				*this *= (1.0d / magnitude);
			}

			return *this;
		}

		double distance(const Vector& other) const {
			return (*this - other).magnitude();
		}

		T dot(const Vector& other) const {
			T sum = 0;
			for (size_t i = 0; i < s; ++i) {
				sum += this->v[i] * other.v[i];
			}
			return sum;
		}

		double angle(const Vector& other) const {
			return acos( std::clamp(double(this->dot(other)) / (this->magnitude() * other.magnitude()), -1.0, 1.0) );
		}
};

// Math Operators with Scalar

template<size_t s, typename T, typename S>
Vector<s, T> operator+(const S scalar, const Vector<s, T>& vector) {
	Vector<s, T> ret(vector);
	for (size_t i = 0; i < s; ++i) {
		ret.v[i] += scalar;
	}
	return ret;
}

template<size_t s, typename T, typename S>
Vector<s, T> operator-(const S scalar, const Vector<s, T>& vector) {
	Vector<s, T> ret(vector);
	for (size_t i = 0; i < s; ++i) {
		ret.v[i] -= scalar;
	}
	return ret;
}

template<size_t s, typename T, typename S>
Vector<s, T> operator*(const S scalar, const Vector<s, T>& vector) {
	Vector<s, T> ret(vector);
	for (size_t i = 0; i < s; ++i) {
		ret.v[i] *= scalar;
	}
	return ret;
}

template<size_t s, typename T, typename S>
Vector<s, T> operator/(const S scalar, const Vector<s, T>& vector) {
	Vector<s, T> ret(vector);
	for (size_t i = 0; i < s; ++i) {
		ret.v[i] /= scalar;
	}
	return ret;
}

// Cross Product

template<typename T, typename S>
Vector<3, T> cross(const Vector<3, T>& a, const Vector<3, S>& b) {
	Vector<3, T> ret;

	ret[0] = a[1] * b[2] - a[2] * b[1];
	ret[1] = a[2] * b[0] - a[0] * b[2];
	ret[2] = a[0] * b[1] - a[1] * b[0];

	return ret;
}

// Compares

template<size_t s, typename T, typename S>
bool operator<(const Vector<s, T>& a, const Vector<s, S>& b) {
	for (size_t i = 0; i < s; ++i) {
		if (a[i] != b[i]) return a[i] < b[i];
	}
	return false;
}

namespace std {
	template <size_t s, typename T>
	struct hash<Vector<s, T>> {
		std::size_t operator()(const Vector<s, T>& v) const {
			size_t h = ~size_t(0);

			for (size_t i = 0; i < s; ++i) {
				h ^= hash<T>()(v[i]) << i;
			}

			return h;
		}
	};
}

// ostream

template<size_t s, typename T>
std::ostream& operator<<(std::ostream& out, const Vector<s, T>& v) {
	out << "v" << s << "(";

	for (size_t i = 0; i < s; ++i) {
		if (i > 0) out << ";";
		out << v[i];
	}

	out << ")";

	return out;
}

// Typedefs

template <typename T>
using Vector2 = Vector<2, T>;
typedef Vector2<int32_t> Vector2i;
typedef Vector2<uint32_t> Vector2u;
typedef Vector2<float> Vector2f;
typedef Vector2<double> Vector2d;

template <typename T>
using Vector3 = Vector<3, T>;
typedef Vector3<int32_t> Vector3i;
typedef Vector3<uint32_t> Vector3u;
typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;

template <typename T>
using Vector4 = Vector<4, T>;
typedef Vector4<int32_t> Vector4i;
typedef Vector4<uint32_t> Vector4u;
typedef Vector4<float> Vector4f;
typedef Vector4<double> Vector4d;

#endif
