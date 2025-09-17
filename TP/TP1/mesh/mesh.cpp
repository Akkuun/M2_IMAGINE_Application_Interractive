#include "mesh.h"
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <fstream>
#include <cmath>
#include <iostream>
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

bool Mesh::isEmpty() const
{
    return vertices.empty() || indices.empty();
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

void Mesh::loadOFF(const QString &fileName)
{
    // on vide les données précédentes
    vertices.clear();
    indices.clear();

    // on vide les buffers OpenGL
    if (vbo.isCreated())
    {
        vbo.destroy();
    }
    if (ibo.isCreated())
    {
        ibo.destroy();
    }

    // Ouvrir le fichier OFF
    std::ifstream file(fileName.toStdString());
    if (!file.is_open())
    {
        qWarning("Could not open the OFF file.");
        return;
    }

    std::string line;
    std::getline(file, line);
    if (line != "OFF")
    {
        qWarning("Not a valid OFF file.");
        return;
    }

    size_t numVertices, numFaces, numEdges;
    file >> numVertices >> numFaces >> numEdges;
    // Lire les sommets
    for (size_t i = 0; i < numVertices; ++i)
    {
        float x, y, z;
        file >> x >> y >> z;
        // Pour l'instant, on initialise les normales à zéro. Elles seront calculées plus tard
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(0.0f); // nx
        vertices.push_back(0.0f); // ny
        vertices.push_back(0.0f); // nz
    }

    // Lire les faces
    for (size_t i = 0; i < numFaces; ++i)
    {
        size_t n, v0, v1, v2;
        file >> n >> v0 >> v1 >> v2;
        if (n != 3)
        {
            qWarning("Only triangular faces are supported.");
            return;
        }
        indices.push_back(static_cast<unsigned int>(v0));
        indices.push_back(static_cast<unsigned int>(v1));
        indices.push_back(static_cast<unsigned int>(v2));

        // Calculer la normale de la face
        // Les vertices sont stockés avec 6 floats par vertex : x,y,z,nx,ny,nz
        float ax = vertices[6 * v1] - vertices[6 * v0];
        float ay = vertices[6 * v1 + 1] - vertices[6 * v0 + 1];
        float az = vertices[6 * v1 + 2] - vertices[6 * v0 + 2];
        float bx = vertices[6 * v2] - vertices[6 * v0];
        float by = vertices[6 * v2 + 1] - vertices[6 * v0 + 1];
        float bz = vertices[6 * v2 + 2] - vertices[6 * v0 + 2];

        float nx = ay * bz - az * by;
        float ny = az * bx - ax * bz;
        float nz = ax * by - ay * bx;

        // Ajouter la normale de la face aux normales des sommets
        vertices[6 * v0 + 3] += nx;
        vertices[6 * v0 + 4] += ny;
        vertices[6 * v0 + 5] += nz;
        vertices[6 * v1 + 3] += nx;
        vertices[6 * v1 + 4] += ny;
        vertices[6 * v1 + 5] += nz;
        vertices[6 * v2 + 3] += nx;
        vertices[6 * v2 + 4] += ny;
        vertices[6 * v2 + 5] += nz;
    }
    file.close();

    // Normaliser les normales des sommets
    for (size_t i = 0; i < numVertices; ++i)
    {
        float nx = vertices[6 * i + 3];
        float ny = vertices[6 * i + 4];
        float nz = vertices[6 * i + 5];
        float length = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (length > 0.0f)
        {
            vertices[6 * i + 3] /= length;
            vertices[6 * i + 4] /= length;
            vertices[6 * i + 5] /= length;
        }
    }

    std::cout << "Loaded OFF file: " << fileName.toStdString() << " with "
              << numVertices << " vertices and " << numFaces << " faces." << std::endl;

    // Mettre à jour les buffers OpenGL
    if (!vbo.isCreated())
    {
        vbo.create();
    }
    if (!ibo.isCreated())
    {
        ibo.create();
    }

    vbo.bind();
    vbo.allocate(vertices.data(), vertices.size() * sizeof(float));
    vbo.release();

    // Bind and fill the IBO with index data
    ibo.bind();
    ibo.allocate(indices.data(), indices.size() * sizeof(unsigned int));
    ibo.release();
}
