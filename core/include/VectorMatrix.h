#ifndef VECTOR_MATRIX_H
#define VECTOR_MATRIX_H

struct vec2 {
	union {
		struct {
			float x, y;
		};
		struct {
			float u, v;
		};
	};

	explicit constexpr vec2(float i = 0) noexcept : x(i), y(i) {}
	constexpr vec2(float x, float y) noexcept : x(x), y(y) {}

	bool operator==(const vec2& v) const noexcept { return x == v.x && y == v.y; }

	vec2 operator*(float s) const noexcept { return { x * s, y * s }; }
	vec2 operator/(float s) const noexcept {
		float r = 1.0f / s;
		return { x * r, y * r };
	}

	vec2 operator*=(float s) noexcept {
		x *= s;
		y *= s;
		return *this;
	}

	vec2 operator+(const vec2& v) const noexcept { return { x + v.x, y + v.y }; }
	vec2 operator-(const vec2& v) const noexcept { return { x - v.x, y - v.y }; }

	void set(float i) { x = y = i; }
	void set(float ix, float iy) {
		x = ix;
		y = iy;
	}

	float lengthSq() const noexcept { return x * x + y * y; }

	float length() const noexcept { return sqrtf(x * x + y * y); }
};

struct vec3 {
	union {
		struct {
			float x, y, z;
		};
		struct {
			float r, g, b;
		};
	};

	explicit constexpr vec3(float i = 0) noexcept : x(i), y(i), z(i) {}
	constexpr vec3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}

	bool operator==(const vec3& v) const noexcept {
		return x == v.x && y == v.y && z == v.z;
	}

	vec3 operator*(float s) const noexcept { return { x * s, y * s, z * s }; };
	vec3 operator/(float s) const noexcept {
		float r = 1.0f / s;
		return { x * r, y * r, z * r };
	}

	vec3 operator*=(float s) noexcept {
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}
	vec3 operator/=(float s) noexcept {
		float r = 1.0f / s;
		x *= r;
		y *= r;
		z *= r;
		return *this;
	}

	vec3 operator-() const noexcept { return { -x, -y, -z }; }

	vec3 operator+(const vec3& v) const noexcept { return { x + v.x, y + v.y, z + v.z }; }
	vec3 operator-(const vec3& v) const noexcept { return { x - v.x, y - v.y, z - v.z }; }

	vec3 operator+=(const vec3& v) noexcept {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
	vec3 operator-=(const vec3& v) noexcept {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	void set(float i) { x = y = z = i; }

	void set(float ix, float iy, float iz) {
		x = ix;
		y = iy;
		z = iz;
	}

	void unit() {
		float r = 1.0f / length();
		x *= r;
		y *= r;
		z *= r;
	}

	float lengthSq() const noexcept { return x * x + y * y + z * z; }

	float length() const noexcept { return sqrtf(x * x + y * y + z * z); }
};

float dot(const vec3& v1, const vec3& v2);
vec3 cross(const vec3& v1, const vec3& v2);

vec3 lerp(const vec3& a, const vec3& b, float t);

struct vec4 {
	float r, g, b, a;

	constexpr vec4() noexcept : r(0), g(0), b(0), a(0) {}

	constexpr vec4(float r, float g, float b, float a) noexcept : r(r), g(g), b(b), a(a) {}
};

vec4 lerp(const vec4& a, const vec4& b, float t);

struct mat3 {
	float
		a, b, c,
		d, e, f,
		g, h, i;

	static const mat3 I;

	constexpr mat3() noexcept : a(0), b(0), c(0), d(0), e(0), f(0), g(0), h(0), i(0) {}

	constexpr mat3(float a, float b, float c, float d, float e, float f, float g, float h, float i) noexcept :
	    a(a), b(b), c(c), d(d), e(e), f(f), g(g), h(h), i(i) {}

	mat3 operator*(const mat3& m) const noexcept;

	vec3 operator*(const vec3& v) const noexcept {
		return {
				a * v.x + b * v.y + c * v.z,
				d * v.x + e * v.y + f * v.z,
				g * v.x + h * v.y + i * v.z
		};
	};

	constexpr void identity() noexcept {
		a = e = i = 1.0f;
		b = c = d = f = g = h = 0.0f;
	}

	void R_VecAndAngle(const vec3& v, float t);

	void R_TwoVectors(const vec3& start, const vec3& end);

	void SetTransposeOf(const mat3* m) {
		a = m->a;
		b = m->d;
		c = m->g;
		d = m->b;
		e = m->e;
		f = m->h;
		g = m->c;
		h = m->f;
		i = m->i;
	}

	void SetInverseOf(const mat3* m);
};
inline constexpr mat3 mat3::I = mat3(
	1, 0, 0,
	0, 1, 0,
	0, 0, 1
);

struct mat4 {
	union {
		float m16[16];
		float m44[4][4];
	};

	mat4 operator*(const mat4& right) const;

	void identity();

	void SetOrthogonalInverseOf(const mat4* m);
};

#endif // VECTOR_MATRIX_H