#include "stdafx.h"
#include "Line.h"

float v[] = {
     0.0f, -0.5f,  0.0f,   // arrow bot
     0.0f,  0.5f,  0.0f,   // arrow top
    -0.3f, 0.25f,  0.0f,   // arrow middle left
     0.0f,  0.5f,  0.0f,   // arrow top
     0.3f, 0.25f,  0.0f,   // arrow middle right
     0.0f,  0.5f,  0.0f    // arrow top
};

Arrow::Arrow() {
    vertices = { 
     0.0f, -0.5f,  0.0f,   // arrow bot
     0.0f,  0.5f,  0.0f,   // arrow top
    -0.3f, 0.25f,  0.0f,   // arrow middle left
     0.0f,  0.5f,  0.0f,   // arrow top
     0.3f, 0.25f,  0.0f,   // arrow middle right
     0.0f,  0.5f,  0.0f    // arrow top
    };

    unsigned int VAO;
    GLuint VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    vaoID = VAO;
}

void Arrow::DrawLine() const {
    glBindVertexArray(static_cast<GLuint>(vaoID));
    glDrawArrays(GL_LINES, 0, 3 * 2);
    glBindVertexArray(0);
}