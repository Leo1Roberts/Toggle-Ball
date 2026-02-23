#include "main.h"

// vec3

inline float dot(const vec3& v1, const vec3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline vec3 cross(const vec3& v1, const vec3& v2) {
	return {
			v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x
	};
}

vec3 lerp(const vec3& a, const vec3& b, float t) {
	return {
			lerp(a.x, b.x, t),
			lerp(a.y, b.y, t),
			lerp(a.z, b.z, t)
	};
}

// vec4

vec4 lerp(const vec4& a, const vec4& b, float t) {
	return {
	lerp(a.r, b.r, t),
	lerp(a.g, b.g, t),
	lerp(a.b, b.b, t),
	lerp(a.a, b.a, t)
	};
}

// mat3

mat3 mat3::operator*(const mat3& m) const {
	mat3 res;
	res.a = a * m.a + b * m.d + c * m.g;
	res.b = a * m.b + b * m.e + c * m.h;
	res.c = a * m.c + b * m.f + c * m.i;
	res.d = d * m.a + e * m.d + f * m.g;
	res.e = d * m.b + e * m.e + f * m.h;
	res.f = d * m.c + e * m.f + f * m.i;
	res.g = g * m.a + h * m.d + i * m.g;
	res.h = g * m.b + h * m.e + i * m.h;
	res.i = g * m.c + h * m.f + i * m.i;
	return res;
}

void mat3::R_VecAndAngle(const vec3& v, float t) {
	float ct = cosf(t), st = sinf(t);

	float
			vx2 = v.x * v.x,
			vy2 = v.y * v.y,
			vz2 = v.z * v.z,
			vxvy = v.x * v.y,
			vxvz = v.x * v.z,
			vyvz = v.y * v.z,
			stvx = st * v.x,
			stvy = st * v.y,
			stvz = st * v.z;

	a = vx2 + ct * (1.0f - vx2);
	d = vxvy - ct * vxvy + stvz;
	g = vxvz - ct * vxvz - stvy;

	b = vxvy - ct * vxvy - stvz;
	e = vy2 + ct * (1.0f - vy2);
	h = vyvz - ct * vyvz + stvx;

	c = vxvz - ct * vxvz + stvy;
	f = vyvz - ct * vyvz - stvx;
	i = vz2 + ct * (1.0f - vz2);
}

void mat3::R_TwoVectors(const vec3& start, const vec3& end) {
	vec3 v = cross(start, end);
	float divisor = 1 / (start.length() * end.length());
	float st = v.length() * divisor;
	float ct = dot(start, end) * divisor;

	float
			vx2 = v.x * v.x,
			vy2 = v.y * v.y,
			vz2 = v.z * v.z,
			vxvy = v.x * v.y,
			vxvz = v.x * v.z,
			vyvz = v.y * v.z,
			stvx = st * v.x,
			stvy = st * v.y,
			stvz = st * v.z;

	a = vx2 + ct * (1.0f - vx2);
	d = vxvy - ct * vxvy + stvz;
	g = vxvz - ct * vxvz - stvy;

	b = vxvy - ct * vxvy - stvz;
	e = vy2 + ct * (1.0f - vy2);
	h = vyvz - ct * vyvz + stvx;

	c = vxvz - ct * vxvz + stvy;
	f = vyvz - ct * vyvz - stvx;
	i = vz2 + ct * (1.0f - vz2);
}

void mat3::SetInverseOf(const mat3* m) {
	float
			det = m->a * (m->e * m->i - m->h * m->f) + m->d * (m->h * m->c - m->b * m->i) + m->g * (m->b * m->f - m->e * m->c),
			rec = 1.0f / det;

	a = (m->e * m->i - m->h * m->f) * rec;
	d = (m->g * m->f - m->d * m->i) * rec;
	g = (m->d * m->h - m->g * m->e) * rec;
	b = (m->h * m->c - m->b * m->i) * rec;
	e = (m->a * m->i - m->g * m->c) * rec;
	h = (m->g * m->b - m->a * m->h) * rec;
	c = (m->b * m->f - m->e * m->c) * rec;
	f = (m->d * m->c - m->a * m->f) * rec;
	i = (m->a * m->e - m->d * m->b) * rec;
}

// mat4

typedef float mat44[4][4];

mat4 mat4::operator*(const mat4& right) const {
	mat4 res;
	const mat44& m = right.m44;
	const float* v = m16;
	for (int i = 0; i < 4; i++) {
		res.m44[i][0] = v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0] + v[3] * m[3][0];
		res.m44[i][1] = v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1] + v[3] * m[3][1];
		res.m44[i][2] = v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2] + v[3] * m[3][2];
		res.m44[i][3] = v[0] * m[0][3] + v[1] * m[1][3] + v[2] * m[2][3] + v[3] * m[3][3];
		v += 4;
	}

	return res;
}

void mat4::identity() {
	m44[0][0] = 1;
	m44[0][1] = 0;
	m44[0][2] = 0;
	m44[0][3] = 0;
	m44[1][0] = 0;
	m44[1][1] = 1;
	m44[1][2] = 0;
	m44[1][3] = 0;
	m44[2][0] = 0;
	m44[2][1] = 0;
	m44[2][2] = 1;
	m44[2][3] = 0;
	m44[3][0] = 0;
	m44[3][1] = 0;
	m44[3][2] = 0;
	m44[3][3] = 1;
}

void mat4::SetOrthogonalInverseOf(const mat4* m) {
	// transpose the top left 3x3

	m44[0][0] = m->m44[0][0];
	m44[0][1] = m->m44[1][0];
	m44[0][2] = m->m44[2][0];
	m44[0][3] = 0.0f;

	m44[1][0] = m->m44[0][1];
	m44[1][1] = m->m44[1][1];
	m44[1][2] = m->m44[2][1];
	m44[1][3] = 0.0f;

	m44[2][0] = m->m44[0][2];
	m44[2][1] = m->m44[1][2];
	m44[2][2] = m->m44[2][2];
	m44[2][3] = 0.0f;

	// translation is the negative of old translation multiplied by the new 3x3

	vec3* v = (vec3*) m->m44[3]; // source v

	m44[3][0] = -v->x * m44[0][0] - v->y * m44[1][0] - v->z * m44[2][0];
	m44[3][1] = -v->x * m44[0][1] - v->y * m44[1][1] - v->z * m44[2][1];
	m44[3][2] = -v->x * m44[0][2] - v->y * m44[1][2] - v->z * m44[2][2];
	m44[3][3] = 1.0f;
}
