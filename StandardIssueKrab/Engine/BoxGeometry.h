#pragma once

struct PhysDebugBox
{
public:
    // The OpenGL identifier of this VAO
    Uint32 vaoID;

    // Data arrays
    Vector<Vec4> Pnt;
    Vector<Vec3> Nrm;
    Vector<Vec2> Tex;
    Vector<Vec3> Tan;

    // Geometry defined by indices into data arrays
    Vector<Ivec3> Tri;
    SizeT count;

    Vec3 minP, maxP;
    Vec3 center;
    Float32 size;
    Mat4 modelTr;

    PhysDebugBox();
    ~PhysDebugBox() = default;
    void DrawVAO() const;

private:
    void ComputeSize();
    void MakeVAO();
    void Face(const Mat4& tr);
};