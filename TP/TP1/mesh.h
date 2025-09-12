#ifndef MESH_H
#define MESH_H

#include <QOpenGLBuffer>
#include <vector>

class Mesh
{
public:
    Mesh();
    ~Mesh();
    void draw();
    void setupCube();
    void cleanup();

private:
    QOpenGLBuffer vbo; // vertexBuffer
    QOpenGLBuffer ibo; // indexBuffer
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

#endif // MESH_H