#include "stdafx.h"

#include "VQS.h"

VQS::VQS() :
	v{},
	q{},
	s{} {

}

VQS::VQS(Vec3 _v, Quat _q, Vec3 _s) :
	v{ _v },
	q{ _q },
	s{ _s } {

}

VQS::VQS(VQS const& _t) :
	v{ _t.v },
	q{ _t.q },
	s{ _t.s } {

}

VQS& VQS::operator= (VQS const& _t) {
	v = _t.v;
	q = _t.q;
	s = _t.s;
	return *this;
}

VQS VQS::operator+ (VQS const& _t) const {
	Vec3 pos = v + _t.v;
	Quat rot = q + _t.q;
	Vec3 scale = s + _t.s;
	return VQS{ pos, rot, scale };
}

void VQS::operator+= (VQS const& _t) {
	v += _t.v;
	q += _t.q;
	s += _t.s;
}

VQS VQS::operator- (VQS const& _t) const {
	Vec3 pos = v - _t.v;
	Quat rot = q - _t.q;
	Vec3 scale = s - _t.s;
	return VQS{ pos, rot, scale };
}

void VQS::operator-= (VQS const& _t) {
	v -= _t.v;
	q -= _t.q;
	s -= _t.s;
}

VQS VQS::operator* (Float32 const& _s) const {
	Vec3 pos = v * _s;
	Quat rot = q * _s;
	Vec3 scale = s * _s;
	return VQS{ pos, rot, scale };
}

void VQS::operator*= (Float32 const& _s) {
	v *= _s;
	q *= _s;
	s *= _s;
}

// non-class function
// first operator is scalar, so swap
VQS operator*(Float32 _s, VQS const& _t) {
	return _t * _s;
}

VQS VQS::operator* (VQS const& _t) const {
	Vec3 pos = Rotate(_t.v);
	Quat rot = q * _t.q;
	Vec3 scale = s * _t.s;
	return VQS{ pos, rot, scale };
}

VQS VQS::Multiply(VQS const& _t) const {
	Vec3 pos = Rotate(_t.v);
	Quat rot = q * _t.q;
	Vec3 scale = s * _t.s;
	return VQS{ pos, rot, scale };
}

void VQS::operator*=(VQS const& _t) {
	(*this) = Multiply(_t);
}

VQS VQS::operator/ (Float32 const& _s) const {
	Vec3 pos = v / _s;
	Quat rot = q / _s;
	Vec3 scale = s / _s;
	return VQS{ pos, rot, scale };
}

void VQS::operator/= (Float32 const& _s) {
	v /= _s;
	q /= _s;
	s /= _s;
}

VQS VQS::Inverse() const {
	Quat rot = glm::inverse(q);
	Vec3 pos = glm::rotate(rot, Rotate((-v) / s));
	Vec3 scale = 1.0f / s;
	return VQS{ pos, rot, scale };
}

Vec3 VQS::operator* (Vec3 const& _r) const {
	return (glm::rotate(q, (s * _r)) + v);
}

Vec3 VQS::Multiply(Vec3 const& _r) const {
	return (glm::rotate(q, (s * _r)) + v);
}

Vec3 VQS::Rotate(Vec3 const& _r) const {
	return (glm::rotate(q, (s * _r)) + v);
}
