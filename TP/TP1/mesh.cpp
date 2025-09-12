#include "mesh.h"
#include <QOpenGLFunctions>
#include <QOpenGLContext>
// Constructor
Mesh::Mesh() : vbo(QOpenGLBuffer::VertexBuffer), ibo(QOpenGLBuffer::IndexBuffer)
{
    // Les buffers seront créés dans setupCube() quand le contexte OpenGL sera disponible
}
// Destructor
Mesh::~Mesh()
{
    cleanup();
}

void Mesh::cleanup()
{
    // Vérifier qu'un contexte OpenGL est disponible avant de détruire les buffers
    if (QOpenGLContext::currentContext())
    {
        if (vbo.isCreated())
        {
            vbo.destroy();
        }
        if (ibo.isCreated())
        {
            ibo.destroy();
        }
    }
}

void Mesh::draw()
{
    // Vérifier que les buffers sont créés et contiennent des données
    if (!vbo.isCreated() || !ibo.isCreated() || indices.empty())
    {
        return;
    }

    vbo.bind();
    ibo.bind();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    // Position attribute (location = 0)
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    // Normal attribute (location = 1)
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    f->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    f->glDisableVertexAttribArray(0);
    f->glDisableVertexAttribArray(1);
    ibo.release();
    vbo.release();
}
// setupCube initializes the vertex and index data for a cube
void Mesh::setupCube()
{
    // Créer les buffers maintenant que le contexte OpenGL est disponible
    if (!vbo.isCreated())
    {
        vbo.create();
    }
    if (!ibo.isCreated())
    {
        ibo.create();
    }

    // Define the vertices for a cube (8 unique vertices with averaged normals)
    vertices = {
        // positions          // normals (moyennées pour chaque vertex)
        -0.15f, -0.15f, -0.15f, -0.1577f, -0.1577f, -0.1577f, // 0: back-bottom-left
        0.15f, -0.15f, -0.15f, 0.1577f, -0.1577f, -0.1577f,   // 1: back-bottom-right
        0.15f, 0.15f, -0.15f, 0.1577f, 0.1577f, -0.1577f,     // 2: back-top-right
        -0.15f, 0.15f, -0.15f, -0.1577f, 0.1577f, -0.1577f,   // 3: back-top-left
        -0.15f, -0.15f, 0.15f, -0.1577f, -0.1577f, 0.1577f,   // 4: front-bottom-left
        0.15f, -0.15f, 0.15f, 0.1577f, -0.1577f, 0.1577f,     // 5: front-bottom-right
        0.15f, 0.15f, 0.15f, 0.1577f, 0.1577f, 0.1577f,       // 6: front-top-right
        -0.15f, 0.15f, 0.15f, -0.1577f, 0.1577f, 0.1577f      // 7: front-top-left
    };

    indices = {
        // Back face (z = -0.5) - regardant vers -Z
        0, 2, 1, 0, 3, 2,
        // Front face (z = 0.5) - regardant vers +Z
        4, 5, 6, 4, 6, 7,
        // Left face (x = -0.5) - regardant vers -X
        0, 4, 7, 0, 7, 3,
        // Right face (x = 0.5) - regardant vers +X
        1, 2, 6, 1, 6, 5,
        // Bottom face (y = -0.5) - regardant vers -Y
        0, 1, 5, 0, 5, 4,
        // Top face (y = 0.5) - regardant vers +Y
        3, 7, 6, 3, 6, 2};

    // Bind and fill the VBO with vertex data
    vbo.bind();
    vbo.allocate(vertices.data(), vertices.size() * sizeof(float));
    vbo.release();

    // Bind and fill the IBO with index data
    ibo.bind();
    ibo.allocate(indices.data(), indices.size() * sizeof(unsigned int));
    ibo.release();
}