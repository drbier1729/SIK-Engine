#pragma once

class VQS {
public:
	Vec3 v;
	Quat q;
	Vec3 s;

public:
	VQS();
	VQS(Vec3 _v, Quat _q, Vec3 _s);
	VQS(VQS const& _t);
	VQS& operator= (VQS const& _t);
	~VQS() noexcept = default;

	// operations
	// add & subtract
	VQS operator+ (VQS const& _t) const;
	void operator+= (VQS const& _t);
	VQS operator- (VQS const& _t) const;
	void operator-= (VQS const& _t);
	// product
	// with scalar
	VQS operator* (Float32 const& _s) const;
	void operator*= (Float32 const& _s);
	// with VQS
	VQS operator* (VQS const& _t) const;
	VQS Multiply(VQS const& _t) const;
	void operator*= (VQS const& _t);
	// division
	// with scalar
	VQS operator/ (Float32 const& _s) const;
	void operator/= (Float32 const& _s);
	// inverse
	VQS Inverse() const;

	// rotation
	Vec3 operator* (Vec3 const& _r) const;
	Vec3 Multiply(Vec3 const& _r) const;
	Vec3 Rotate(Vec3 const& _r) const;
};

// first operand is scalar
VQS operator* (Float32 _s, VQS const& _t);