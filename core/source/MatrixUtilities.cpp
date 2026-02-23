#include "main.h"
#include "MatrixUtilities.h"

void fillRotation(mat4* mat, const mat3& src) {
	mat->m44[0][0] = src.a;
	mat->m44[1][0] = src.b;
	mat->m44[2][0] = src.c;
	mat->m44[0][1] = src.d;
	mat->m44[1][1] = src.e;
	mat->m44[2][1] = src.f;
	mat->m44[0][2] = src.g;
	mat->m44[1][2] = src.h;
	mat->m44[2][2] = src.i;
}

void fillRotationScaled(mat4* mat, const mat3& src, const vec3& scale) {
	mat->m44[0][0] = src.a * scale.x;
	mat->m44[1][0] = src.b * scale.x;
	mat->m44[2][0] = src.c * scale.x;
	mat->m44[0][1] = src.d * scale.y;
	mat->m44[1][1] = src.e * scale.y;
	mat->m44[2][1] = src.f * scale.y;
	mat->m44[0][2] = src.g * scale.z;
	mat->m44[1][2] = src.h * scale.z;
	mat->m44[2][2] = src.i * scale.z;
}

void buildOrthographicMatrix(mat4* res, float halfHeight, float aspect, float zNear, float zFar) {
	float halfWidth = halfHeight * aspect;
	float* m = res->m16;
	m[0] = 1.0f / halfWidth;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;
	m[4] = 0.0f;
	m[5] = 1.0f / halfHeight;
	m[6] = 0.0f;
	m[7] = 0.0f;
	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = -2.0f / (zFar - zNear);
	m[11] = -(zFar + zNear) / (zFar - zNear);
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}

void buildPerspectiveMatrix(mat4* res, float width, float height, float zNear, float zFar) {
	float* m = res->m16;
	m[0] = 2.0f * zNear / width;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;
	m[4] = 0.0f;
	m[5] = 2.0f * zNear / height;
	m[6] = 0.0f;
	m[7] = 0.0f;
	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = zFar / (zFar - zNear);
	m[11] = 1.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = zNear * zFar / (zNear - zFar);
	m[15] = 0.0f;
}

// o_fov is width of view plane at 1m distance - e.g. 2.0 for 90 degree fov
// o_fov = 2.0f * tan(0.5 * fov)   (where fov is horizontal fov in radians)
void buildProjectionMatrix(mat4* projMat, float o_fov, float zNear, float zFar, vec2 offset, int width,
                           int height) {
	float w = o_fov * zNear; // width at zNear view plane
	float mul = width ? (float) height / width : 1.0f;
	buildPerspectiveMatrix(projMat, w, w * mul, zNear, zFar);

	projMat->m44[2][0] = offset.x;
	projMat->m44[2][1] = offset.y;
}

void buildViewRotationMatrix(mat3* viewRotMat, const vec3& viewDir) {
	float
			a = viewDir.x,
			b = viewDir.y,
			c = viewDir.z,
			h = sqrtf(a * a + b * b);

	float st, ct, sp, cp;

	if (h < 0.001f) // assume looking forward along y axis
	{
		st = 1.0f;
		ct = 0.0f;
		sp = 0.0f;
		cp = c > 0.0f ? 1.0f : -1.0f;
	} else // can construct matrix correctly
	{
		st = b / h;
		ct = a / h;
		sp = h;
		cp = c;
	}

	viewRotMat->a = -st;
	viewRotMat->b = ct;
	viewRotMat->c = 0.0f;
	viewRotMat->d = -ct * cp;
	viewRotMat->e = -st * cp;
	viewRotMat->f = sp;
	viewRotMat->g = -ct * sp;
	viewRotMat->h = -st * sp;
	viewRotMat->i = -cp;
}

void buildViewMatrix(mat4* viewMat, const mat3& viewRotMat, const vec3& viewPos) {
	mat4 rotMat, transMat;

	rotMat.identity();
	transMat.identity();

	fillRotation(&rotMat, viewRotMat);

	transMat.m44[3][0] = -viewPos.x;
	transMat.m44[3][1] = -viewPos.y;
	transMat.m44[3][2] = -viewPos.z;

	*viewMat = transMat * rotMat; // set viewMat
}

void buildScaledWorldMatrix(mat4* worldMat, const mat3& rot, const vec3& pos, const vec3& scale) {
	fillRotationScaled(worldMat, rot, scale);

	worldMat->m44[0][3] = worldMat->m44[1][3] = worldMat->m44[2][3] = 0.0f;
	worldMat->m44[3][3] = 1.0f;

	worldMat->m44[3][0] = pos.x;
	worldMat->m44[3][1] = pos.y;
	worldMat->m44[3][2] = pos.z;
}