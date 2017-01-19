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

union Vec2i {
	struct {
		S32 x, y;
	};
	struct {
		S32 w, h;
	};
	S32 p[2];
};
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

union Mat2f {
	struct {
		Vec2f col1;
		Vec2f col2;
	};
	F32 m[4];
};

union Mat4f {
	struct {
		Vec4f col1;
		Vec4f col2;
		Vec4f col3;
		Vec4f col4;
	};
	F32 m[16];
};

struct Transform {
	Vec2f pos;
	Vec2f scale;
	Mat2f rot;
};

// Scalar functions

inline B32 Equals(F32 a, F32 b, F32 tolerance = FLOAT_TOLERANCE) {
	F32 v = Abs(a - b);
	B32 result = v < tolerance;
	return (result);
}

inline B32 IsGreater(F32 a, F32 b, F32 tolerance = FLOAT_TOLERANCE) {
	F32 v = a - b;
	B32 result = v > tolerance;
	return (result);
}

inline F32 Lerp(F32 a, F32 t, F32 b) {
	F32 result = (1.0f - t) * a + t * b;
	return(result);
}

inline F32 Clamp(F32 value, F32 min, F32 max) {
	F32 result = value;
	if (result < min) {
		result = min;
	} else if (result > max) {
		result = max;
	}
	return(result);
}

inline F32 Clamp01(F32 value) {
	F32 result = Clamp(value, 0.0f, 1.0f);
	return(result);
}

inline F32 Clamp01MapToRange(F32 t, F32 min, F32 max) {
	F32 result = 0.0f;
	F32 range = max - min;
	if (range != 0.0f) {
		result = Clamp01((t - min) / range);
	}
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

inline B32 Equals(const Vec2f &a, const Vec2f &b, F32 tolerance = FLOAT_TOLERANCE) {
	B32 result = Equals(a.x, b.x, tolerance) &&
		Equals(a.y, b.y, tolerance);
	return (result);
}

inline B32 Equals(const Vec4f &a, const Vec4f &b, F32 tolerance = FLOAT_TOLERANCE) {
	B32 result = Equals(a.x, b.x, tolerance) &&
		Equals(a.y, b.y, tolerance) &&
		Equals(a.z, b.z, tolerance) &&
		Equals(a.w, b.w, tolerance);
	return (result);
}

inline F32 Dot(const Vec2f &a, const Vec2f &b) {
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

inline Vec2f Vec2MultM(const Vec2f &a, const Mat2f &mat) {
	Vec2f result;
	result.x = mat.col1.x * a.x + mat.col2.x * a.y;
	result.y = mat.col1.y * a.x + mat.col2.y * a.y;
	return(result);
}

inline Vec2f Vec2MultT(const Vec2f &a, const Transform &t) {
	Vec2f result = Vec2MultM(a, t.rot) + t.pos;
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

inline Vec2f Hadamard(const Vec2f &a, const Vec2f &b) {
	Vec2f result = { a.x * b.x, a.y * b.y };
	return(result);
}

inline F32 LengthSquared(const Vec2f &v) {
	F32 result = Dot(v, v);
	return(result);
}

inline F32 Length(const Vec2f &v) {
	F32 result = SquareRoot(LengthSquared(v));
	return(result);
}

inline Vec2f Normalize(const Vec2f &v) {
	F32 l = 1.0f / Length(v);
	Vec2f result = v * l;
	return (result);
}

inline Vec2f Cross(const Vec2f &a, F32 s) {
	return V2(s * a.y, -s * a.x);
}

inline Vec2f Cross(F32 s, const Vec2f &a) {
	return V2(-s * a.y, s * a.x);
}

inline F32 Cross(const Vec2f &a, const Vec2f &b) {
	return a.x * b.y - a.y * b.x;
}

inline Vec2f V2Min(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2(Min(a.x, b.x), Min(a.y, b.y));
	return(result);
}

inline Vec2f V2Max(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2(Max(a.x, b.x), Max(a.y, b.y));
	return(result);
}

inline B32 IsUnitVector(const Vec2f &v) {
	B32 result = !((v.x < -1.0f || v.x > 1.0f) || (v.y < -1.0f || v.y > 1.0f));
	return(result);
}

// Matrix functions

inline Mat2f MakeIdentity2() {
	Mat2f result;
	result.col1 = V2(1.0f, 0.0f);
	result.col2 = V2(0.0f, 1.0f);
	return (result);
}

inline Mat2f MakeRotation2(F32 angle) {
	F32 s = Sin(angle);
	F32 c = Cos(angle);
	Mat2f result;
	result.col1 = V2(c, s);
	result.col2 = V2(-s, c);
	return(result);
}

inline Mat2f MakeRotation2(Vec2f axis) {
	Mat2f result;
	result.col1 = axis;
	result.col2 = Cross(1.0f, axis);
	return(result);
}

inline Mat2f Transpose2(const Mat2f &m) {
	Mat2f result;
	result.col1 = V2(m.col1.x, m.col2.x);
	result.col2 = V2(m.col1.y, m.col2.y);
	return(result);
}

inline Mat2f Mat2Mult(const Mat2f &a, const Mat2f &b) {
	Mat2f result;
	result.col1 = Vec2MultM(b.col1, a);
	result.col2 = Vec2MultM(b.col2, a);
	return(result);
}

inline F32 ToAngle(const Vec2f &v) {
	F32 result = ATan2(v.y, v.x);
	return(result);
}

inline F32 ToAngle(const Mat2f &mat) {
	F32 result = ToAngle(mat.col1);
	return(result);
}

inline Mat4f MakeIdentity4() {
	Mat4f result;
	result.col1 = V4(1.0f, 0.0f, 0.0f, 0.0f);
	result.col2 = V4(0.0f, 1.0f, 0.0f, 0.0f);
	result.col3 = V4(0.0f, 0.0f, 1.0f, 0.0f);
	result.col4 = V4(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

inline Mat4f MakeRotation4(const Mat2f &mat2) {
	Mat4f result = MakeIdentity4();
	result.col1.xy = mat2.col1;
	result.col2.xy = mat2.col2;
	return (result);
}

inline Mat4f MakeTranslation4(const Vec2f &p) {
	Mat4f result = MakeIdentity4();
	result.col4.x = p.x;
	result.col4.y = p.y;
	result.col4.z = 0.0f;
	return (result);
}

inline Mat4f MakeScale4(const Vec3f &s) {
	Mat4f result = MakeIdentity4();
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = s.z;
	return (result);
}

inline Mat4f operator *(const Mat4f &a, const Mat4f &b) {
	// http://stackoverflow.com/questions/18499971/efficient-4x4-matrix-multiplication-c-vs-assembly
	Mat4f result = MakeIdentity4();
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

inline Transform DefaultTransform() {
	Transform result;
	result.pos = V2();
	result.rot = MakeIdentity2();
	result.scale = V2(1.0f, 1.0f);
	return(result);
}

inline Transform MakeTranslation2(const Vec2f &translation) {
	Transform result;
	result.pos = translation;
	result.rot = MakeIdentity2();
	result.scale = V2(1, 1);
	return(result);
}

inline Transform MakeTransform2(const Vec2f &pos, F32 rotation = 0.0f, F32 scale = 1.0f) {
	Transform result;
	result.pos = pos;
	result.rot = MakeRotation2(rotation);
	result.scale = V2(scale, scale);
	return(result);
}
inline Transform MakeTransform2(const Vec2f &pos, Vec2f axis, Vec2f scale = V2(1.0f, 1.0f)) {
	Transform result;
	result.pos = pos;
	result.rot = MakeRotation2(axis);
	result.scale = scale;
	return(result);
}

inline Transform MultTransform2(const Transform &a, const Transform &b) {
	Transform result;
	result.rot = Mat2Mult(a.rot, b.rot);
	result.pos = a.pos + b.pos;
	result.scale = Hadamard(a.scale, b.scale);
	return (result);
}