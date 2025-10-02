#ifndef SphereSelectionTool_H
#define SphereSelectionTool_H
#include "Vec3.h"
#include <GL/glut.h>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declaration
struct Mesh;

struct AStarNode
{
	int vertexIndex; // Index du vertex dans le mesh
	float gCost;	 // Distance réelle depuis le départ
	float hCost;	 // Heuristique (distance euclidienne)
	float fCost() const { return gCost + hCost; }
	int parent; // Index du parent dans le chemin

	// fonction de comparaison pour la priority_queue (comparaison heuristique)
	bool operator<(const AStarNode &other) const
	{
		return fCost() > other.fCost(); // Pour priority_queue (min-heap)
	}
};

struct SphereSelectionTool
{
	float radius;
	Vec3 center;
	bool isActive;
	bool isAdding;

	SphereSelectionTool() : radius(0.6f), center(0.0, 0.0, 0.0), isActive(false), isAdding(false) {}

	void initSphere(const Vec3 &pCenter, const float &pRadius)
	{
		center = pCenter;
		radius = pRadius;
	}

	// Nouvelle méthode pour mettre à jour seulement la position
	void updateSphere(const Vec3 &newCenter)
	{
		center = newCenter;
	}

	void updateSphere(float newRadius)
	{
		radius = newRadius;
	}

	bool contains(const Vec3 &p)
	{
		// si point distance du poids est inférieur au rayon, il est dans la sphère (cf raytracing rapport)
		Vec3 diff = p - center;
		float distance = sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
		return distance <= radius;
	}

	std::unordered_map<int, float> computeGeodesicDistances(
		const Mesh &mesh,
		int startVertexIndex,
		float maxDistance = -1.0f,					 // -1 signifie pas de limite
		const Vec3 &referenceNormal = Vec3(0, 0, 0), // Normale de référence N pour filtrage
		bool useNormalHeuristic = false)			 // Utiliser l'heuristique basée sur la normale
	{
		std::unordered_map<int, float> distances; // vertexIndex -> distance
		std::priority_queue<AStarNode> openSet;	  // noeuds à explorer
		std::unordered_set<int> closedSet;		  // noeuds déjà explorés

		// on instancie le noeud de départ

		AStarNode startNode;
		startNode.vertexIndex = startVertexIndex;
		startNode.gCost = 0.0f;
		startNode.hCost = 0.0f;
		startNode.parent = -1; // pas de parent pour le noeud de départ

		openSet.push(startNode);			// on l'ajoute à la liste des noeuds à explorer
		distances[startVertexIndex] = 0.0f; // distance entre start et start = 0

		while (!openSet.empty())
		{									   // tant qu'on a pas tout explorer
			AStarNode current = openSet.top(); // noeud actuel
			openSet.pop();					   // on le retire de la liste des noeuds à explorer

			if (closedSet.find(current.vertexIndex) != closedSet.end())
				continue; // si on l'a déjà exploré, on passe au suivant

			// On marque le noeud actuel comme exploré
			closedSet.insert(current.vertexIndex);

			// Arrêter si on dépasse la distance maximale (seulement si maxDistance > 0)
			if (maxDistance > 0 && current.gCost > maxDistance)
				continue;

			for (int voisinIndex : getNeighbors(mesh, current.vertexIndex)) // on récupere tout les voisins du node actuel
			{
				if (closedSet.find(voisinIndex) != closedSet.end())
					continue; // Skip if already processed

				Vec3 currentPos = mesh.V[current.vertexIndex].p; // position actuel du node a comparer
				Vec3 neighborPos = mesh.V[voisinIndex].p;		 // position du voisin retourne par getNeighbors

				float coutArete = (neighborPos - currentPos).length(); // cout de l'arête entre le noeud actuel et son voisin
				float newGCost = current.gCost + coutArete;			   // cout total pour aller au voisin en passant par le noeud actuel

				// Skip if this path is longer than max distance
				if (maxDistance > 0 && newGCost > maxDistance)
					continue;

				// Si on a trouvé un chemin plus court > on ajoute en node le voisin le plus proche dans
				// distance
				if (distances.find(voisinIndex) == distances.end() ||
					newGCost < distances[voisinIndex])
				{

					distances[voisinIndex] = newGCost;

					AStarNode neighbor;
					neighbor.vertexIndex = voisinIndex;
					neighbor.gCost = newGCost;

					// Choisir l'heuristique selon le mode
					if (useNormalHeuristic && referenceNormal.length() > 0.1f)
					{
						// Heuristique basée sur la différence de normale
						Vec3 neighborNormal = mesh.V[voisinIndex].n;
						float dotProduct = referenceNormal[0] * neighborNormal[0] +
										   referenceNormal[1] * neighborNormal[1] +
										   referenceNormal[2] * neighborNormal[2];
						float normalDifference = 1.0f - dotProduct; // Plus c'est proche de 0, plus les normales sont similaires
						neighbor.hCost = normalDifference * 10.0f;	// Multiplier pour donner plus de poids
					}
					else
					{
						// Heuristique euclidienne classique
						neighbor.hCost = (neighborPos - center).length();
					}

					neighbor.parent = current.vertexIndex;

					openSet.push(neighbor);
				}
			}
		}

		return distances;
	}

	void draw()
	{
		if (!isActive)
			return;

		glPushMatrix();
		glTranslatef(center[0], center[1], center[2]);

		// Wireframe sphere with transparency
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (isAdding)
			glColor4f(0.0f, 1.0f, 0.0f, 0.3f); // on ajoute
		else
			glColor4f(1.0f, 0.0f, 0.0f, 0.3f); // on retire

		glutWireSphere(radius, 20, 20);

		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}
	// fonction pour obtenir les voisins d'un vertex
	std::vector<int> getNeighbors(const Mesh &mesh, int vertexIndex)
	{
		std::vector<int> neighbors;

		for (const auto &triangle : mesh.T)
		{
			for (int i = 0; i < 3; i++) // pour chaque vertex du triangle
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

	// Trouver le vertex V le plus proche de P et retourner sa normale N
	std::pair<int, Vec3> findClosestVertexWithNormal(const Mesh &mesh, const Vec3 &point)
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

		// Retourner l'index du vertex V et sa normale N
		return std::make_pair(closestIndex, mesh.V[closestIndex].n);
	}

	// Converti les coordonnées 2D de la souris en coordonnées 3D dans l'espace du monde via gluUnProject
	Vec3 screenTo3D(int mouseX, int mouseY)
	{
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ;
		GLdouble posX, posY, posZ;

		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glGetIntegerv(GL_VIEWPORT, viewport);

		winX = (float)mouseX;
		winY = (float)viewport[3] - (float)mouseY; // Flip Y coordinate
		glReadPixels(mouseX, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

		gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		return Vec3(posX, posY, posZ);
	}

	// Scroll radius modif
	void updateRadius(float deltaRadius)
	{
		radius += deltaRadius;
		if (radius < 0.1f)
			radius = 0.1f; // valeur min
		if (radius > 5.0f)
			radius = 5.0f; // valeur max
	}
};
#endif