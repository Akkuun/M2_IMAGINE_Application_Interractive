#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <algorithm>
#include "Vec3.h"

#include <GL/glut.h>

// -------------------------------------------
// Basic Mesh class
// -------------------------------------------

struct MeshVertex
{
    inline MeshVertex() {}
    inline MeshVertex(const Vec3 &_p, const Vec3 &_n) : p(_p), pInit(_p), n(_n) {}
    inline MeshVertex(const MeshVertex &v) : p(v.p), pInit(v.pInit), n(v.n) {}
    inline virtual ~MeshVertex() {}
    inline MeshVertex &operator=(const MeshVertex &v)
    {
        p = v.p;
        pInit = v.pInit;
        n = v.n;
        return (*this);
    }
    double &operator[](unsigned int c)
    {
        return p[c];
    }
    double operator[](unsigned int c) const
    {
        return p[c];
    }

    // membres :
    Vec3 p;     // une position
    Vec3 pInit; // une position
    Vec3 n;     // une normale
};

struct MeshTriangle
{
    inline MeshTriangle()
    {
        v[0] = v[1] = v[2] = 0;
    }
    inline MeshTriangle(const MeshTriangle &t)
    {
        v[0] = t.v[0];
        v[1] = t.v[1];
        v[2] = t.v[2];
    }
    inline MeshTriangle(unsigned int v0, unsigned int v1, unsigned int v2)
    {
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
    }
    inline virtual ~MeshTriangle() {}
    inline MeshTriangle &operator=(const MeshTriangle &t)
    {
        v[0] = t.v[0];
        v[1] = t.v[1];
        v[2] = t.v[2];
        return (*this);
    }
    unsigned int &operator[](unsigned int c)
    {
        return v[c];
    }
    unsigned int operator[](unsigned int c) const
    {
        return v[c];
    }

    // membres :
    unsigned int v[3];
};

class Mesh
{
public:
    std::vector<MeshVertex> V;
    std::vector<MeshTriangle> T;

    void loadOFF(const std::string &filename);
    void recomputeNormals();
    void centerAndScaleToUnit();
    void scaleUnit();

    void draw() const
    {
        // This code is deprecated. We will how to use vertex arrays and vertex buffer objects instead.
        glBegin(GL_TRIANGLES);
        for (unsigned int i = 0; i < T.size(); i++)
            for (unsigned int j = 0; j < 3; j++)
            {
                const MeshVertex &v = V[T[i].v[j]];
                glNormal3f(v.n[0], v.n[1], v.n[2]);
                glVertex3f(v.p[0], v.p[1], v.p[2]);
            }
        glEnd();
    }

    void drawWithColors(const std::vector<Vec3> &vertexColors) const
    {
        glBegin(GL_TRIANGLES);
        for (unsigned int i = 0; i < T.size(); i++)
            for (unsigned int j = 0; j < 3; j++)
            {
                const MeshVertex &v = V[T[i].v[j]];
                if (T[i].v[j] < vertexColors.size())
                {
                    const Vec3 &color = vertexColors[T[i].v[j]];
                    glColor3f(color[0], color[1], color[2]);
                }
                glNormal3f(v.n[0], v.n[1], v.n[2]);
                glVertex3f(v.p[0], v.p[1], v.p[2]);
            }
        glEnd();
    }

    std::vector<unsigned int> getAdjacentVertices(unsigned int vertexIndex) const
    {
        std::vector<unsigned int> adjacentVertices;
        for (unsigned int i = 0; i < T.size(); i++)
        {
            for (unsigned int j = 0; j < 3; j++)
            {
                if (T[i].v[j] == vertexIndex)
                {
                    // Add the other two vertices of this triangle
                    for (unsigned int k = 0; k < 3; k++)
                    {
                        if (k != j)
                        {
                            unsigned int neighbor = T[i].v[k];
                            // Check if this neighbor is not already in the list
                            if (std::find(adjacentVertices.begin(), adjacentVertices.end(), neighbor) == adjacentVertices.end())
                            {
                                adjacentVertices.push_back(neighbor);
                            }
                        }
                    }
                    break;
                }
            }
        }
        return adjacentVertices;
    }
};

#endif
