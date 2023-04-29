#include "stdafx.h"
#include "BoxGeometry.h"


inline constexpr float PI = 3.14159f;
inline constexpr float rad = PI / 180.0f;

static void pushquad(Vector<Ivec3>& Tri, SizeT i, SizeT j, SizeT k, SizeT l)
{
    Tri.push_back(glm::ivec3(i, j, k));
    Tri.push_back(glm::ivec3(i, k, l));
}

// Batch up all the data defining a shape to be drawn (example: the
// teapot) as a Vertex Array object (VAO) and send it to the graphics
// card.  Return an OpenGL identifier for the created VAO.
static Uint32 VaoFromTris(const Vector<Vec4>& Pnt,
                        const Vector<Vec3>& Nrm,
                        const Vector<Vec2>& Tex,
                        const Vector<Vec3>& Tan,
                        const Vector<Ivec3>& Tri)
{
    Uint32 vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    GLuint Pbuff;
    glGenBuffers(1, &Pbuff);
    glBindBuffer(GL_ARRAY_BUFFER, Pbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * Pnt.size(),
        &Pnt[0][0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (Nrm.size() > 0) {
        GLuint Nbuff;
        glGenBuffers(1, &Nbuff);
        glBindBuffer(GL_ARRAY_BUFFER, Nbuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * Nrm.size(),
            &Nrm[0][0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (Tex.size() > 0) {
        GLuint Tbuff;
        glGenBuffers(1, &Tbuff);
        glBindBuffer(GL_ARRAY_BUFFER, Tbuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * Tex.size(),
            &Tex[0][0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (Tan.size() > 0) {
        GLuint Dbuff;
        glGenBuffers(1, &Dbuff);
        glBindBuffer(GL_ARRAY_BUFFER, Dbuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * Tan.size(),
            &Tan[0][0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint Ibuff;
    glGenBuffers(1, &Ibuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ibuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * Tri.size(),
        &Tri[0][0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    return vaoID;
}

void PhysDebugBox::ComputeSize()
{
    // Compute min/max
    minP = Vec3(Pnt[0]);
    maxP = Vec3(Pnt[0]);
        for (auto&& p : Pnt) {
            for (Uint32 c = 0; c < 3; c++) {
                minP[c] = std::min(minP[c], p[c]);
                maxP[c] = std::max(maxP[c], p[c]);
            }
        }

    center = (maxP + minP) / 2.0f;
    size = 0.0;
    for (Uint32 c = 0; c < 3; c++)
        size = std::max(size, (maxP[c] - minP[c]) / 2.0f);

    Float32 s = 1.0f / size;
    modelTr = glm::scale(Mat4(1), Vec3(s, s, s)) * glm::translate(Mat4(1), Vec3(-center[0], -center[1], -center[2]));
}

void PhysDebugBox::MakeVAO()
{
    vaoID = VaoFromTris(Pnt, Nrm, Tex, Tan, Tri);
    count = Tri.size();
}

void PhysDebugBox::DrawVAO() const
{
    glBindVertexArray(vaoID);
    glDrawElements(GL_TRIANGLES, 3 * static_cast<GLsizei>( count ), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////
   // Generates a box +-1 on all axes
PhysDebugBox::PhysDebugBox()
{
    glm::mat4 I(1.0f);

    // Six faces, each a rotation of a rectangle placed on the z axis.
    Face(I);
    Float32 r90 = PI / 2;
    Face(glm::rotate(I, r90,  Vec3(1.0f, 0.0f, 0.0f)));
    Face(glm::rotate(I, -r90, Vec3(1.0f, 0.0f, 0.0f)));
    Face(glm::rotate(I, r90,  Vec3(0.0f, 1.0f, 0.0f)));
    Face(glm::rotate(I, -r90, Vec3(0.0f, 1.0f, 0.0f)));
    Face(glm::rotate(I, PI,   Vec3(1.0f, 0.0f, 0.0f)));

    ComputeSize();
    MakeVAO();
}

void PhysDebugBox::Face(const Mat4& tr)
{
    SizeT n = Pnt.size();

    Float32 verts[8] = { 1.0f,1.0f, -1.0f,1.0f, -1.0f,-1.0f, 1.0f,-1.0f };
    Float32 texcd[8] = { 1.0f,1.0f,  0.0f,1.0f,  0.0f, 0.0f, 1.0f, 0.0f };

    // Four vertices to make a single face, with its own normal and
    // texture coordinates.
    for (Uint32 i = 0; i < 8; i += 2) {
        Pnt.push_back(tr * Vec4(verts[i], verts[i + 1], 1.0f, 1.0f));
        Nrm.push_back(Vec3(tr * Vec4(0.0f, 0.0f, 1.0f, 0.0f)));
        Tex.push_back(Vec2(texcd[i], texcd[i + 1]));
        Tan.push_back(Vec3(tr * Vec4(1.0f, 0.0f, 0.0f, 0.0f)));
    }

    pushquad(Tri, n, n + 1, n + 2, n + 3);
}