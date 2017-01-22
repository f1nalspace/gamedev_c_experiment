#pragma once

#include "engine_types.h"
#include "engine_intrinsics.h"

constant F32 PI32 = 3.14159265359f;
constant F32 TAU32 = 6.28318530717958647692f;
constant F32 DEG2RAD32 = PI32 / 180.0f;
constant F32 RAD2DEG32 = 180.0f / PI32;
constant F32 FLOAT_TOLERANCE = 0.00001f;

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Sign(v) ((v) < 0 ?  -1 : 1)

union Vec2i {
	struct {
		S32 x, y;
	};
	struct {
		S32 w, h;
	};
	S32 p[2];
};
StaticAlignmentAssert(Vec2i);

union Vec2f {
	struct {
		F32 x, y;
	};
	struct {
		F32 u, v;
	};
	struct {
		F32 w, h;
	};
	F32 p[2];
};
StaticAlignmentAssert(Vec2f);

union Vec3f {
	struct {
		F32 x, y, z;
	};
	struct {
		F32 u, v, w;
	};
	struct {
		F32 r, g, b;
	};
	struct {
		Vec2f xy;
		F32 ignored0;
	};
	struct {
		F32 ignored1;
		Vec2f yz;
	};
	struct {
		Vec2f uv;
		F32 ignored2;
	};
	struct {
		F32 ignored3;
		Vec2f vw;
	};
	F32 p[3];
};
StaticAlignmentAssert(Vec3f);

union Vec4f {
	struct {
		union {
			Vec3f xyz;
			struct {
				F32 x, y, z;
			};
		};
		F32 w;
	};
	struct {
		union {
			Vec3f rgb;
			struct {
				F32 r, g, b;
			};
		};
		F32 a;
	};
	struct {
		Vec3f xyz;
		F32 w;
	};
	struct {
		Vec2f xy;
		F32 ignored0;
		F32 ignored1;
	};
	struct {
		F32 ignored2;
		Vec2f yz;
		F32 ignored3;
	};
	struct {
		F32 ignored4;
		F32 ignored5;
		Vec2f zw;
	};
	F32 p[4];
};
StaticAlignmentAssert(Vec4f);

union Mat2f {
	struct {
		Vec2f col1;
		Vec2f col2;
	};
	F32 m[4];
};
StaticAlignmentAssert(Mat2f);

union Mat4f {
	struct {
		Vec4f col1;
		Vec4f col2;
		Vec4f col3;
		Vec4f col4;
	};
	F32 m[16];
};
StaticAlignmentAssert(Mat4f);

struct Transform {
	Vec2f pos;
	Vec2f scale;
	Mat2f rot;
};
StaticAlignmentAssert(Transform);

union AABB {
	struct {
		Vec2f min, max;
	};
	Vec2f e[2];
};
StaticAlignmentAssert(AABB);

// Scalar functions

inline B32 ScalarEquals(F32 a, F32 b, F32 tolerance = FLOAT_TOLERANCE) {
	F32 v = Abs(a - b);
	B32 result = v < tolerance;
	return (result);
}

inline B32 ScalarIsGreater(F32 a, F32 b, F32 tolerance = FLOAT_TOLERANCE) {
	F32 v = a - b;
	B32 result = v > tolerance;
	return (result);
}

inline F32 ScalarLerp(F32 a, F32 t, F32 b) {
	F32 result = (1.0f - t) * a + t * b;
	return(result);
}

inline F32 ScalarClamp(F32 value, F32 min, F32 max) {
	F32 result = value;
	if (result < min) {
		result = min;
	} else if (result > max) {
		result = max;
	}
	return(result);
}

inline F32 ScalarClamp01(F32 value) {
	F32 result = ScalarClamp(value, 0.0f, 1.0f);
	return(result);
}

inline F32 DegreeToRadians32(F32 degree) {
	F32 result = degree * DEG2RAD32;
	return(result);
}

inline F32 RadiansToDegree32(F32 radians) {
	F32 result = radians * RAD2DEG32;
	return(result);
}

// Vector functions

inline Vec2i V2i(S32 x, S32 y) {
	Vec2i result;
	result.x = x;
	result.y = y;
	return(result);
}
inline Vec2i V2i() {
	return V2i(0, 0);
}

inline Vec2f V2(F32 x, F32 y) {
	Vec2f result;
	result.x = x;
	result.y = y;
	return(result);
}
inline Vec2f V2(F32 xy) {
	return V2(xy, xy);
}
inline Vec2f V2() {
	return V2(0, 0);
}

inline Vec3f V3(F32 x, F32 y, F32 z) {
	Vec3f result;
	result.x = x;
	result.y = y;
	result.z = z;
	return(result);
}
inline Vec3f V3(F32 xyz) {
	return V3(xyz, xyz, xyz);
}
inline Vec3f V3() {
	return V3(0, 0, 0);
}

inline Vec4f V4(F32 x, F32 y, F32 z, F32 w = 1.0f) {
	Vec4f result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return(result);
}
inline Vec4f V4() {
	return V4(0, 0, 0, 1);
}

inline B32 Vec2Equals(const Vec2f &a, const Vec2f &b, F32 tolerance = FLOAT_TOLERANCE) {
	B32 result = ScalarEquals(a.x, b.x, tolerance) &&
		ScalarEquals(a.y, b.y, tolerance);
	return (result);
}

inline B32 Vec4Equals(const Vec4f &a, const Vec4f &b, F32 tolerance = FLOAT_TOLERANCE) {
	B32 result = ScalarEquals(a.x, b.x, tolerance) &&
		ScalarEquals(a.y, b.y, tolerance) &&
		ScalarEquals(a.z, b.z, tolerance) &&
		ScalarEquals(a.w, b.w, tolerance);
	return (result);
}

inline F32 Vec2Dot(const Vec2f &a, const Vec2f &b) {
	F32 result = a.x * b.x + a.y * b.y;
	return(result);
}

inline Vec2f operator*(F32 a, const Vec2f &b) {
	Vec2f result;
	result.x = a * b.x;
	result.y = a * b.y;
	return(result);
}
inline Vec2f operator*(const Vec2f &a, F32 b) {
	Vec2f result = b * a;
	return(result);
}
inline Vec2f& operator*=(Vec2f &a, F32 value) {
	a = value * a;
	return(a);
}

inline Vec2f operator-(const Vec2f &a) {
	Vec2f result;
	result.x = -a.x;
	result.y = -a.y;
	return(result);
}
inline Vec2f operator+(const Vec2f &a, const Vec2f &b) {
	Vec2f result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return(result);
}
inline Vec2f& operator+=(Vec2f &a, const Vec2f &b) {
	a = b + a;
	return(a);
}
inline Vec2f operator-(const Vec2f &a, const Vec2f &b) {
	Vec2f result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return(result);
}
inline Vec2f& operator-=(Vec2f &a, const Vec2f &b) {
	a = a - b;
	return(a);
}

inline Vec2f Vec2MultMat2(const Vec2f &a, const Mat2f &mat) {
	Vec2f result;
	result.x = mat.col1.x * a.x + mat.col2.x * a.y;
	result.y = mat.col1.y * a.x + mat.col2.y * a.y;
	return(result);
}

inline Vec2f Vec2MultTransform(const Vec2f &a, const Transform &t) {
	Vec2f result = Vec2MultMat2(a, t.rot) + t.pos;
	return(result);
}

// Vec3f

inline Vec3f operator*(F32 a, const Vec3f &b) {
	Vec3f result;
	result.x = a * b.x;
	result.y = a * b.y;
	result.z = a * b.z;
	return(result);
}

inline Vec3f operator*(const Vec3f &a, F32 b) {
	Vec3f result = b * a;
	return(result);
}
inline Vec3f& operator*=(Vec3f &a, F32 value) {
	a = value * a;
	return(a);
}

inline Vec2f Vec2Hadamard(const Vec2f &a, const Vec2f &b) {
	Vec2f result = { a.x * b.x, a.y * b.y };
	return(result);
}

inline F32 Vec2LengthSquared(const Vec2f &v) {
	F32 result = Vec2Dot(v, v);
	return(result);
}

inline F32 Vec2Length(const Vec2f &v) {
	F32 result = SquareRoot(Vec2LengthSquared(v));
	return(result);
}

inline Vec2f Vec2Normalize(const Vec2f &v) {
	F32 l = 1.0f / Vec2Length(v);
	Vec2f result = v * l;
	return (result);
}

inline Vec2f Vec2Cross(const Vec2f &a, F32 s) {
	return V2(s * a.y, -s * a.x);
}

inline Vec2f Vec2Cross(F32 s, const Vec2f &a) {
	return V2(-s * a.y, s * a.x);
}

inline F32 Vec2Cross(const Vec2f &a, const Vec2f &b) {
	return a.x * b.y - a.y * b.x;
}

inline Vec2f Vec2Min(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2(Min(a.x, b.x), Min(a.y, b.y));
	return(result);
}

inline Vec2f Vec2Max(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2(Max(a.x, b.x), Max(a.y, b.y));
	return(result);
}

// Matrix functions

inline Mat2f Mat2Identity() {
	Mat2f result;
	result.col1 = V2(1.0f, 0.0f);
	result.col2 = V2(0.0f, 1.0f);
	return (result);
}

inline Mat2f Mat2RotationFromAngle(F32 angle) {
	F32 s = Sin(angle);
	F32 c = Cos(angle);
	Mat2f result;
	result.col1 = V2(c, s);
	result.col2 = V2(-s, c);
	return(result);
}

inline Mat2f Mat2RotationFromAxis(Vec2f axis) {
	Mat2f result;
	result.col1 = axis;
	result.col2 = Vec2Cross(1.0f, axis);
	return(result);
}

inline Mat2f Mat2Transpose(const Mat2f &m) {
	Mat2f result;
	result.col1 = V2(m.col1.x, m.col2.x);
	result.col2 = V2(m.col1.y, m.col2.y);
	return(result);
}

inline Mat2f Mat2Mult(const Mat2f &a, const Mat2f &b) {
	Mat2f result;
	result.col1 = Vec2MultMat2(b.col1, a);
	result.col2 = Vec2MultMat2(b.col2, a);
	return(result);
}

inline F32 AxisToAngle(const Vec2f &axis) {
	F32 result = ATan2(axis.y, axis.x);
	return(result);
}

inline F32 Mat2ToAngle(const Mat2f &mat) {
	F32 result = AxisToAngle(mat.col1);
	return(result);
}

inline Mat4f Mat4Identity() {
	Mat4f result;
	result.col1 = V4(1.0f, 0.0f, 0.0f, 0.0f);
	result.col2 = V4(0.0f, 1.0f, 0.0f, 0.0f);
	result.col3 = V4(0.0f, 0.0f, 1.0f, 0.0f);
	result.col4 = V4(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

inline Mat4f Mat4RotationFromMat2(const Mat2f &mat2) {
	Mat4f result = Mat4Identity();
	result.col1.xy = mat2.col1;
	result.col2.xy = mat2.col2;
	return (result);
}

inline Mat4f Mat4TranslationFromVec2(const Vec2f &p) {
	Mat4f result = Mat4Identity();
	result.col4.x = p.x;
	result.col4.y = p.y;
	result.col4.z = 0.0f;
	return (result);
}

inline Mat4f Mat4ScaleFromVec3(const Vec3f &s) {
	Mat4f result = Mat4Identity();
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = s.z;
	return (result);
}

inline Mat4f operator *(const Mat4f &a, const Mat4f &b) {
	// http://stackoverflow.com/questions/18499971/efficient-4x4-matrix-multiplication-c-vs-assembly
	Mat4f result = Mat4Identity();
	for (U32 i = 0; i < 16; i += 4) {
		for (U32 j = 0; j < 4; ++j) {
			result.m[i + j] =
				(b.m[i + 0] * a.m[j + 0])
				+ (b.m[i + 1] * a.m[j + 4])
				+ (b.m[i + 2] * a.m[j + 8])
				+ (b.m[i + 3] * a.m[j + 12]);
		}
	}
	return(result);
}

// Transform functions

inline Transform TransformIdentity() {
	Transform result;
	result.pos = V2();
	result.rot = Mat2Identity();
	result.scale = V2(1.0f, 1.0f);
	return(result);
}

inline Transform TransformMakeTranslation(const Vec2f &translation) {
	Transform result;
	result.pos = translation;
	result.rot = Mat2Identity();
	result.scale = V2(1, 1);
	return(result);
}

inline Transform TransformMake(const Vec2f &pos, F32 rotation = 0.0f, F32 scale = 1.0f) {
	Transform result;
	result.pos = pos;
	result.rot = Mat2RotationFromAngle(rotation);
	result.scale = V2(scale, scale);
	return(result);
}
inline Transform TransformMake(const Vec2f &pos, Vec2f axis, Vec2f scale = V2(1.0f, 1.0f)) {
	Transform result;
	result.pos = pos;
	result.rot = Mat2RotationFromAxis(axis);
	result.scale = scale;
	return(result);
}

inline Transform TransformMult(const Transform &a, const Transform &b) {
	Transform result;
	result.rot = Mat2Mult(a.rot, b.rot);
	result.pos = a.pos + b.pos;
	result.scale = Vec2Hadamard(a.scale, b.scale);
	return (result);
}

// AABB functions

inline AABB AABBFromMinMax(const Vec2f &min, const Vec2f &max) {
	AABB result;
	result.min = min;
	result.max = max;
	return(result);
}

inline AABB AABBFromCenterExt(const Vec2f &center, const Vec2f &extend) {
	AABB result;
	result.min = center - extend;
	result.max = center + extend;
	return(result);
}

inline AABB AABBCombine(const AABB &a, const AABB &b) {
	AABB result;
	result.min = Vec2Min(a.min, b.min);
	result.max = Vec2Max(a.max, b.max);
	return(result);
}