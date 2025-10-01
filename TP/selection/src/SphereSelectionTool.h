#ifndef SphereSelectionTool_H
#define SphereSelectionTool_H
#include "Vec3.h"
#include <GL/glut.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

	// Nouvelle méthode pour mettre à jour seulement le rayon
	void updateSphere(float newRadius)
	{
		radius = newRadius;
	}

	bool contains(const Vec3 &p)
	{
		// Calculate distance between point p and sphere center
		Vec3 diff = p - center;
		float distance = sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
		return distance <= radius;
	}

	void draw()
	{
		if (!isActive)
			return;

		glPushMatrix();
		glTranslatef(center[0], center[1], center[2]);

		// Draw wireframe sphere to visualize selection area
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (isAdding)
			glColor4f(0.0f, 1.0f, 0.0f, 0.3f); // Green for adding
		else
			glColor4f(1.0f, 0.0f, 0.0f, 0.3f); // Red for removing

		glutWireSphere(radius, 20, 20);

		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

	// Convert screen coordinates to 3D world coordinates
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

	// Update radius (for mouse wheel interaction)
	void updateRadius(float deltaRadius)
	{
		radius += deltaRadius;
		if (radius < 0.1f)
			radius = 0.1f; // Minimum radius
		if (radius > 5.0f)
			radius = 5.0f; // Maximum radius
	}
};
#endif