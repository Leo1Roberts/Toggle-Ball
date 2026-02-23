#ifndef MATRIX_UTILITIES_H
#define MATRIX_UTILITIES_H

void buildOrthographicMatrix(mat4* res, float halfHeight, float aspect, float zNear, float zFar);

void buildPerspectiveMatrix(mat4* res, float width, float height, float zNear, float zFar);

void
buildProjectionMatrix(mat4* projMat, float o_fov, float zNear, float zFar, const vec2& offset, int width,
                      int height);

void buildViewRotationMatrix(mat3* viewRotMat, const vec3& viewDir);

void buildViewMatrix(mat4* viewMat, const mat3& viewRotMat, const vec3& viewPos);

void buildScaledWorldMatrix(mat4* worldMat, const mat3& rot, const vec3& pos, const vec3& scale);

#endif // MATRIX_UTILITIES_H