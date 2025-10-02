#include <queue>
#include <unordered_map>
#include <limits>
#include <cmath>
#include "Mesh.h"

struct AStarNode
{
    int vertexIndex;
    float gCost; // Distance réelle depuis le départ
    float hCost; // Heuristique (distance euclidienne)
    float fCost() const { return gCost + hCost; }
    int parent;

    bool operator<(const AStarNode &other) const
    {
        return fCost() > other.fCost(); // Pour priority_queue (min-heap)
    }
};

struct SphereSelectionTool
{
    // ...existing members...

    // Nouvelle méthode pour calculer les distances géodésiques
    std::unordered_map<int, float> computeGeodesicDistances(
        const Mesh &mesh,
        int startVertexIndex,
        float maxDistance)
    {
        std::unordered_map<int, float> distances;
        std::priority_queue<AStarNode> openSet;
        std::unordered_set<int> closedSet;

        // Initialiser le noeud de départ
        AStarNode startNode;
        startNode.vertexIndex = startVertexIndex;
        startNode.gCost = 0.0f;
        startNode.hCost = 0.0f;
        startNode.parent = -1;

        openSet.push(startNode);
        distances[startVertexIndex] = 0.0f;

        while (!openSet.empty())
        {
            AStarNode current = openSet.top();
            openSet.pop();

            if (closedSet.find(current.vertexIndex) != closedSet.end())
                continue;

            closedSet.insert(current.vertexIndex);

            // Si on dépasse la distance max, on s'arrête
            if (current.gCost > maxDistance)
                continue;

            // Explorer les voisins
            for (int neighborIndex : getNeighbors(mesh, current.vertexIndex))
            {
                if (closedSet.find(neighborIndex) != closedSet.end())
                    continue;

                Vec3 currentPos = mesh.V[current.vertexIndex].p;
                Vec3 neighborPos = mesh.V[neighborIndex].p;

                float edgeCost = (neighborPos - currentPos).length();
                float newGCost = current.gCost + edgeCost;

                // Si on a trouvé un chemin plus court
                if (distances.find(neighborIndex) == distances.end() ||
                    newGCost < distances[neighborIndex])
                {

                    distances[neighborIndex] = newGCost;

                    AStarNode neighbor;
                    neighbor.vertexIndex = neighborIndex;
                    neighbor.gCost = newGCost;
                    neighbor.hCost = (neighborPos - center).length(); // Heuristique
                    neighbor.parent = current.vertexIndex;

                    openSet.push(neighbor);
                }
            }
        }

        return distances;
    }

    // Méthode pour obtenir les voisins d'un vertex
    std::vector<int> getNeighbors(const Mesh &mesh, int vertexIndex)
    {
        std::vector<int> neighbors;

        // Parcourir toutes les faces pour trouver les arêtes connectées
        for (const auto &triangle : mesh.T)
        {
            for (int i = 0; i < 3; i++)
            {
                if (triangle.v[i] == vertexIndex)
                {
                    // Ajouter les deux autres vertices du triangle
                    neighbors.push_back(triangle.v[(i + 1) % 3]);
                    neighbors.push_back(triangle.v[(i + 2) % 3]);
                }
            }
        }

        // Supprimer les doublons
        std::sort(neighbors.begin(), neighbors.end());
        neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());

        return neighbors;
    }

    // Nouvelle méthode de sélection géodésique
    bool containsGeodesic(const Mesh &mesh, const Vec3 &p, int clickedVertexIndex)
    {
        // Trouver l'index du vertex le plus proche de p
        int vertexIndex = findClosestVertex(mesh, p);

        // Calculer les distances géodésiques depuis le point cliqué
        auto distances = computeGeodesicDistances(mesh, clickedVertexIndex, radius);

        // Vérifier si ce vertex est dans la distance géodésique
        return distances.find(vertexIndex) != distances.end() &&
               distances[vertexIndex] <= radius;
    }

    // Trouver le vertex le plus proche d'un point
    int findClosestVertex(const Mesh &mesh, const Vec3 &point)
    {
        int closestIndex = 0;
        float minDistance = (mesh.V[0].p - point).length();

        for (int i = 1; i < mesh.V.size(); i++)
        {
            float distance = (mesh.V[i].p - point).length();
            if (distance < minDistance)
            {
                minDistance = distance;
                closestIndex = i;
            }
        }

        return closestIndex;
    }

    // ...existing methods...
};