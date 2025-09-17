#include "Mesh.h"
#include <iostream>
#include <fstream>
#include <cmath>

void Mesh::loadOFF(const std::string &filename)
{
    std::ifstream in(filename.c_str());
    if (!in)
        exit(EXIT_FAILURE);
    std::string offString;
    unsigned int sizeV, sizeT, tmp;
    in >> offString >> sizeV >> sizeT >> tmp;
    V.resize(sizeV);
    T.resize(sizeT);
    for (unsigned int i = 0; i < sizeV; i++)
        in >> V[i].p;
    int s;
    for (unsigned int i = 0; i < sizeT; i++)
    {
        in >> s;
        for (unsigned int j = 0; j < 3; j++)
            in >> T[i].v[j];
    }
    in.close();
    recomputeNormals();
}

void Mesh::recomputeNormals()
{
    for (unsigned int i = 0; i < V.size(); i++)
        V[i].n = Vec3(0.0, 0.0, 0.0);
    for (unsigned int i = 0; i < T.size(); i++)
    {
        Vec3 e01 = V[T[i].v[1]].p - V[T[i].v[0]].p;
        Vec3 e02 = V[T[i].v[2]].p - V[T[i].v[0]].p;
        Vec3 n = Vec3::cross(e01, e02);
        n.normalize();
        for (unsigned int j = 0; j < 3; j++)
            V[T[i].v[j]].n += n;
    }
    for (unsigned int i = 0; i < V.size(); i++)
        V[i].n.normalize();
}

void Mesh::compute_skinning_weights(Skeleton &skeleton)
{
    // you should compute weights for each vertex w.r.t. the skeleton bones
    // so each vertex will have B weights (B = number of bones)
    // these weights shoud be stored in vertex.w:

    unsigned int B = skeleton.bones.size();
    int n = 10; // parametre pour l'influence de la distance sur le poids
    for (unsigned int i = 0; i < V.size(); ++i)
    {
        // l'idée s'est de parcourir tout les os, et calculer la distance entre le vertex et l'os,
        V[i].w.resize(B, 0.0); // poids de l'os initialiser a 0
        Vec3 poid = V[i].p;
        double sommePoids = 0.0; // variable qui contient les somme des poids pour la normalisation
        // on parcourt tout les os
        for (unsigned int j = 0; j < B; ++j)
        {
            Vec3 articulation1 = skeleton.articulations[skeleton.bones[j].joints[0]].p; // os j
            Vec3 articulation2 = skeleton.articulations[skeleton.bones[j].joints[1]].p; // os k

            Vec3 distanceArticulation1 = poid - articulation1;                 // vecteur entre le vertex et l'articulation 1
            Vec3 articulation1ToArticulation2 = articulation2 - articulation1; // vecteur entre les deux articulations pour avoir le segment articulation 1 et 2

            float t = Vec3::dot(distanceArticulation1, articulation1ToArticulation2) / articulation1ToArticulation2.squareLength();
            t = std::max(0.f, std::min(1.f, t));
            Vec3 proj = articulation1 + t * articulation1ToArticulation2; // projection orthogonale du vertex sur le segment
            float distance = (poid - proj).length();
            // calcul du poids
            V[i].w[j] = pow((1.0 / (distance)), n); // on ajoute un petit epsilon pour eviter la division par 0
            sommePoids += V[i].w[j];
            // poids i, j  = (1/ distanceij)^n
        }
        // Normalisation des poids pour rester entre 0 et 1
        for (unsigned int j = 0; j < B; ++j)
            V[i].w[j] /= sommePoids;
    }
}

void Mesh::draw(int displayedBone) const
{

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glBegin(GL_TRIANGLES);
    for (unsigned int i = 0; i < T.size(); i++)
        for (unsigned int j = 0; j < 3; j++)
        {
            const MeshVertex &v = V[T[i].v[j]];
            if (displayedBone >= 0 && displayedBone < v.w.size())
            {
                Vec3 rgb = HSVtoRGB(v.w[displayedBone], 0.8, 0.8);
                glColor3f(rgb[0], rgb[1], rgb[2]);
            }
            else
                glColor3f(0.6, 0.6, 0.6);
            glNormal3f(v.n[0], v.n[1], v.n[2]);
            glVertex3f(v.p[0], v.p[1], v.p[2]);
        }

    glEnd();
}

void Mesh::drawTransformedMesh(SkeletonTransformation &transfo) const
{

    std::vector<Vec3> new_positions(V.size());
    std::vector<Vec3> new_normals(V.size());

    for (unsigned int i = 0; i < V.size(); i++) // on calcul pour chaque os
    {
        new_positions[i] = Vec3(0, 0, 0);
        new_normals[i] = Vec3(0, 0, 0);

        for (unsigned int j = 0; j < transfo.bone_transformations.size(); j++) // pour chaque os
        {
            // pi = somme sur j ( poids(i,j) * ( Rj * pi + tj ) )
            Vec3 poid = V[i].p;                                               // position du vertex i
            Vec3 normal = V[i].n;                                             // normale du vertex i
            Mat3 R = transfo.bone_transformations[j].world_space_rotation;    // matrice de rotation de l'os j
            Vec3 t = transfo.bone_transformations[j].world_space_translation; // translation de l'os j
            Vec3 p_transformed = R * poid + t;                                // formule de transformation
            // pour la normale, on fait l'inverse transpose de la matrice de rotation
            Vec3 n_transformed = R * normal; // car R est une matrice de rotation, son inverse est son transpose
            n_transformed.normalize();

            new_positions[i] += V[i].w[j] * p_transformed;
            new_normals[i] += V[i].w[j] * n_transformed;
        }
    }

    //---------------------------------------------------//
    //---------------------------------------------------//
    //---------------------------------------------------//

    glEnable(GL_LIGHTING);
    glBegin(GL_TRIANGLES);
    for (unsigned int i = 0; i < T.size(); i++)
        for (unsigned int j = 0; j < 3; j++)
        {
            const MeshVertex &v = V[T[i].v[j]];
            Vec3 p = new_positions[T[i].v[j]];
            Vec3 n = new_normals[T[i].v[j]];
            glColor3f(0.6, 0.6, 0.6);
            glNormal3f(n[0], n[1], n[2]);
            glVertex3f(p[0], p[1], p[2]);
        }
    glEnd();
}

/*! \brief Convert HSV to RGB color space

  Converts a given set of HSV values `h', `s', `v' into RGB
  coordinates. The output RGB values are in the range [0, 1], and
  the input HSV values are in the ranges h = [0, 360], and s, v =
  [0, 1], respectively.

  \param fH Hue component, used as input, range: [0, 1]
  \param fS Hue component, used as input, range: [0, 1]
  \param fV Hue component, used as input, range: [0, 1]

  \param fR Red component, used as output, range: [0, 1]
  \param fG Green component, used as output, range: [0, 1]
  \param fB Blue component, used as output, range: [0, 1]

*/
Vec3 Mesh::HSVtoRGB(float fH, float fS, float fV) const
{

    fH = (1. - fH) * 0.65 * 360.;

    float fR, fG, fB;
    float fC = fV * fS; // Chroma
    float fHPrime = fmod(fH / 60.0, 6);
    float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    float fM = fV - fC;

    if (0 <= fHPrime && fHPrime < 1)
    {
        fR = fC;
        fG = fX;
        fB = 0;
    }
    else if (1 <= fHPrime && fHPrime < 2)
    {
        fR = fX;
        fG = fC;
        fB = 0;
    }
    else if (2 <= fHPrime && fHPrime < 3)
    {
        fR = 0;
        fG = fC;
        fB = fX;
    }
    else if (3 <= fHPrime && fHPrime < 4)
    {
        fR = 0;
        fG = fX;
        fB = fC;
    }
    else if (4 <= fHPrime && fHPrime < 5)
    {
        fR = fX;
        fG = 0;
        fB = fC;
    }
    else if (5 <= fHPrime && fHPrime < 6)
    {
        fR = fC;
        fG = 0;
        fB = fX;
    }
    else
    {
        fR = 0;
        fG = 0;
        fB = 0;
    }

    fR += fM;
    fG += fM;
    fB += fM;
    return Vec3(fR, fG, fB);
}
