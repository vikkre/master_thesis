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
		Vector(const std::vector<T>& vec) : v{0} {
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
		Vector operator+(const S scalar) const {
			Vector ret(*this);
			for (size_t i = 0; i < s; ++i) {
				ret.v[i] += scalar;
			}
			return ret;
		}

		template<typename S>
		Vector operator-(const S scalar) const {
			Vector ret(*this);
			for (size_t i = 0; i < s; ++i) {
				ret.v[i] -= scalar;
			}
			return ret;
		}

		template<typename S>
		Vector operator*(const S scalar) const {
			Vector ret(*this);
			for (size_t i = 0; i < s; ++i) {
				ret.v[i] *= scalar;
			}
			return ret;
		}

		template<typename S>
		Vector operator/(const S scalar) const {
			Vector ret(*this);
			for (size_t i = 0; i < s; ++i) {
				ret.v[i] /= scalar;
			}
			return ret;
		}

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

		Vector operator*(const Vector& other) const {
			Vector ret(*this);
			for (size_t i = 0; i < s; ++i) {
				ret.v[i] *= other.v[i];
			}
			return ret;
		}

		Vector operator/(const Vector& other) const {
			Vector ret(*this);
			for (size_t i = 0; i < s; ++i) {
				ret.v[i] /= other.v[i];
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

		Vector& operator*=(const Vector& other) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] *= other.v[i];
			}
			return *this;
		}

		Vector& operator/=(const Vector& other) {
			for (size_t i = 0; i < s; ++i) {
				this->v[i] /= other.v[i];
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

		T magnitudeSquared() const {
			T sum = 0;
			for (size_t i = 0; i < s; ++i) {
				sum += this->v[i] * this->v[i];
			}
			return sum;
		}

		double magnitude() const {
			return std::sqrt((double) magnitudeSquared());
		}

		Vector& normalize() {
			double magnitude = this->magnitude();

			if (magnitude != 0) {
				double factor = 1.0 / magnitude;
				for (size_t i = 0; i < s; ++i) {
					this->v[i] *= factor;
				}
			}

			return *this;
		}

		T distanceSquared(const Vector& other) const {
			return (*this - other).magnitudeSquared();
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
	Vector<s, T> ret;
	for (size_t i = 0; i < s; ++i) {
		ret.v[i] = scalar + vector.v[i];
	}
	return ret;
}

template<size_t s, typename T, typename S>
Vector<s, T> operator-(const S scalar, const Vector<s, T>& vector) {
	Vector<s, T> ret;
	for (size_t i = 0; i < s; ++i) {
		ret.v[i] = scalar - vector.v[i];
	}
	return ret;
}

template<size_t s, typename T, typename S>
Vector<s, T> operator*(const S scalar, const Vector<s, T>& vector) {
	Vector<s, T> ret;
	for (size_t i = 0; i < s; ++i) {
		ret.v[i] = scalar * vector.v[i];
	}
	return ret;
}

template<size_t s, typename T, typename S>
Vector<s, T> operator/(const S scalar, const Vector<s, T>& vector) {
	Vector<s, T> ret;
	for (size_t i = 0; i < s; ++i) {
		ret.v[i] = scalar / vector.v[i];
	}
	return ret;
}

// Cross Product

template<typename T>
Vector<3, T> cross(const Vector<3, T>& a, const Vector<3, T>& b) {
	Vector<3, T> ret;

	ret[0] = a[1] * b[2] - a[2] * b[1];
	ret[1] = a[2] * b[0] - a[0] * b[2];
	ret[2] = a[0] * b[1] - a[1] * b[0];

	return ret;
}

// lerp

template<typename T>
T lerp(T v0, T v1, float t) {
	return (1.0f - t) * v0 + t * v1;
}

template<size_t s, typename T>
Vector<s, T> lerp(const Vector<s, T>& v0, const Vector<s, T>& v1, float t) {
	Vector<s, T> res;

	for (size_t i = 0; i < s; ++i) {
		res[i] = lerp(v0[i], v1[i], t);
	}

	return res;
}

// reflect, refract

template<size_t s, typename T>
Vector<s, T> reflect(const Vector<s, T>& I, const Vector<s, T>& N) {
	return I - ((2.0 * N.dot(I)) * N);
}

template<size_t s, typename T>
Vector<s, T> refract(const Vector<s, T>& I, const Vector<s, T>& N, T eta) {
	// float NdI = N.dot(I);
	float k = 1.0 - eta * eta * (1.0 - N.dot(I) * N.dot(I));
	if (k < 0.0) return Vector<s, T>();
	else return eta * I - (eta * N.dot(I) + sqrt(k)) * N;
}

template<size_t s, typename T>
Vector<s, T> customRefract(const Vector<s, T>& I, Vector<s, T> N, T eta) {
	if (N.dot(I) > 0.0) {
		N = -1.0 * N;
	} else {
		eta = 1.0 / eta;
	}

	float angle = sin(acos(N.dot(I))) * eta;
	if (-1.0 < angle && angle < 1.0)
		return refract(I, N, eta);
	else
		return reflect(I, N);
}

// Triangle Area

template<typename T>
float getTriangleArea(const Vector<3, T>& v1, const Vector<3, T>& v2, const Vector<3, T>& v3) {
	Vector<3, T> ab = v2 - v1;
	Vector<3, T> ac = v3 - v1;
	Vector<3, T> cp = cross(ab, ac);
	return 0.5f * cp.magnitude();
}

// Comverts

template<typename T>
Vector<4, T> expandVector(const Vector<3, T>& vec, T v) {
	return Vector<4, T>({vec[0], vec[1], vec[2], v});
}

template<typename T>
Vector<3, T> cutVector(const Vector<4, T>& vec) {
	return Vector<3, T>({vec[0], vec[1], vec[2]});
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
