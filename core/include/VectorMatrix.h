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

	vec2() : x(0), y(0) {}

	vec2(float i) : x(i), y(i) {}

	vec2(float ix, float iy) : x(ix), y(iy) {}

	bool operator==(const vec2& v) const { return x == v.x && y == v.y; };

	vec2 operator*(float s) const { return { x * s, y * s }; };

	vec2 operator/(float s) const {
		float r = 1.0f / s;
		return { x * r, y * r };
	};

	vec2 operator*=(float s) {
		x *= s;
		y *= s;
		return *this;
	};

	vec2 operator+(const vec2& v) const { return { x + v.x, y + v.y }; };

	vec2 operator-(const vec2& v) const { return { x - v.x, y - v.y }; };

	void set(float i) { x = y = i; }

	void set(float ix, float iy) {
		x = ix;
		y = iy;
	}

	float lengthSq() const { return x * x + y * y; }

	float length() const { return sqrtf(x * x + y * y); }
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

	vec3() : x(0), y(0), z(0) {}

	vec3(float i) : x(i), y(i), z(i) {}

	vec3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {}

	bool operator==(const vec3& v) const { return x == v.x && y == v.y && z == v.z; };

	vec3 operator*(float s) const { return { x * s, y * s, z * s }; };

	vec3 operator/(float s) const {
		float r = 1.0f / s;
		return { x * r, y * r, z * r };
	};

	vec3 operator*=(float s) {
		x *= s;
		y *= s;
		z *= s;
		return *this;
	};

	vec3 operator/=(float s) {
		float r = 1.0f / s;
		x *= r;
		y *= r;
		z *= r;
		return *this;
	};

	vec3 operator+(const vec3& v) const { return { x + v.x, y + v.y, z + v.z }; };

	vec3 operator-(const vec3& v) const { return { x - v.x, y - v.y, z - v.z }; };

	vec3 operator+=(const vec3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	};

	vec3 operator-=(const vec3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	};

	bool operator!=(const vec3& v) { return x != v.x || y != v.y || z != v.z; };

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

	float lengthSq() const { return x * x + y * y + z * z; }

	float length() const { return sqrtf(x * x + y * y + z * z); }
};

float dot(const vec3& v1, const vec3& v2);
vec3 cross(const vec3& v1, const vec3& v2);

vec3 lerp(const vec3& a, const vec3& b, float t);

struct vec4 {
	float r, g, b, a;

	vec4() : r(0), g(0), b(0), a(0) {}

	vec4(float ir, float ig, float ib, float ia) : r(ir), g(ig), b(ib), a(ia) {}
};

vec4 lerp(const vec4& a, const vec4& b, float t);

struct mat3 {
	float
		a, b, c,
		d, e, f,
		g, h, i;

	mat3() : a(0), b(0), c(0), d(0), e(0), f(0), g(0), h(0), i(0) {}

	mat3(float iA, float iB, float iC, float iD, float iE, float iF, float iG, float iH, float iI) :
	    a(iA), b(iB), c(iC), d(iD), e(iE), f(iF), g(iG), h(iH), i(iI) {}

	mat3 operator*(const mat3& m) const;

	vec3 operator*(const vec3& v) const {
		return {
				a * v.x + b * v.y + c * v.z,
				d * v.x + e * v.y + f * v.z,
				g * v.x + h * v.y + i * v.z
		};
	};

	void identity() {
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
const mat3 I_MAT3 = mat3(
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