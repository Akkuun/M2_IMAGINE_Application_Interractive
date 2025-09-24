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
	bool isAdding;
	bool isActive;

	SphereSelectionTool() : radius(0.6f), center(0.0, 0.0, 0.0), isAdding(false), isActive(false) {}

	void initSphere(const Vec3 &pCenter, const float &pRadius)
	{
		// init sphere with Vec3 center and radius
		center = pCenter;
		radius = pRadius;
	}

	void updateSphere(float pRadius)
	{
		// update radius
		radius = pRadius;
	}

	void updateSphere(const Vec3 &pCenter)
	{
		// update center position
		center = pCenter;
	}

	bool contains(const Vec3 &p)
	{
		// Project both the point and center to screen coordinates, then check 2D distance
		// Get transformation matrices
		float modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		float projection[16];
		glGetFloatv(GL_PROJECTION_MATRIX, projection);

		// Project the vertex p to screen coordinates
		float px = modelview[0] * p[0] + modelview[4] * p[1] + modelview[8] * p[2] + modelview[12];
		float py = modelview[1] * p[0] + modelview[5] * p[1] + modelview[9] * p[2] + modelview[13];
		float pz = modelview[2] * p[0] + modelview[6] * p[1] + modelview[10] * p[2] + modelview[14];
		float pw = modelview[3] * p[0] + modelview[7] * p[1] + modelview[11] * p[2] + modelview[15];
		px /= pw;
		py /= pw;
		pz /= pw;
		pw = 1.f;

		float pxx = projection[0] * px + projection[4] * py + projection[8] * pz + projection[12] * pw;
		float pyy = projection[1] * px + projection[5] * py + projection[9] * pz + projection[13] * pw;
		float pww = projection[3] * px + projection[7] * py + projection[11] * pz + projection[15] * pw;
		pxx /= pww;
		pyy /= pww;

		pxx = (pxx + 1.f) / 2.f;
		pyy = (pyy + 1.f) / 2.f;

		// Project the center to screen coordinates
		float cx = modelview[0] * center[0] + modelview[4] * center[1] + modelview[8] * center[2] + modelview[12];
		float cy = modelview[1] * center[0] + modelview[5] * center[1] + modelview[9] * center[2] + modelview[13];
		float cz = modelview[2] * center[0] + modelview[6] * center[1] + modelview[10] * center[2] + modelview[14];
		float cw = modelview[3] * center[0] + modelview[7] * center[1] + modelview[11] * center[2] + modelview[15];
		cx /= cw;
		cy /= cw;
		cz /= cw;
		cw = 1.f;

		float cxx = projection[0] * cx + projection[4] * cy + projection[8] * cz + projection[12] * cw;
		float cyy = projection[1] * cx + projection[5] * cy + projection[9] * cz + projection[13] * cw;
		float cww = projection[3] * cx + projection[7] * cy + projection[11] * cz + projection[15] * cw;
		cxx /= cww;
		cyy /= cww;

		cxx = (cxx + 1.f) / 2.f;
		cyy = (cyy + 1.f) / 2.f;

		// Calculate 2D distance in screen space
		float dx = pxx - cxx;
		float dy = pyy - cyy;
		float screenDistance = sqrt(dx * dx + dy * dy);
		float screenRadius = radius * 2.0f; // Same scaling as in draw()

		return screenDistance <= screenRadius;
	}

	void draw()
	{
		if (!isActive)
			return;

		// Get transformation matrices to project 3D center to screen coordinates
		float modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		float projection[16];
		glGetFloatv(GL_PROJECTION_MATRIX, projection);

		// Project 3D center to screen coordinates (same way as setTagForVerticesInRectangle)
		Vec3 const &p = center;
		float x = modelview[0] * p[0] + modelview[4] * p[1] + modelview[8] * p[2] + modelview[12];
		float y = modelview[1] * p[0] + modelview[5] * p[1] + modelview[9] * p[2] + modelview[13];
		float z = modelview[2] * p[0] + modelview[6] * p[1] + modelview[10] * p[2] + modelview[14];
		float w = modelview[3] * p[0] + modelview[7] * p[1] + modelview[11] * p[2] + modelview[15];
		x /= w;
		y /= w;
		z /= w;
		w = 1.f;

		float xx = projection[0] * x + projection[4] * y + projection[8] * z + projection[12] * w;
		float yy = projection[1] * x + projection[5] * y + projection[9] * z + projection[13] * w;
		float ww = projection[3] * x + projection[7] * y + projection[11] * z + projection[15] * w;
		xx /= ww;
		yy /= ww;

		// Convert to normalized device coordinates [0,1] (same as in setTagForVerticesInRectangle)
		float centerX = (xx + 1.f) / 2.f;
		float centerY = (yy + 1.f) / 2.f;

		// Convert 3D radius to screen space radius (approximate)
		float screenRadius = radius * 2.0f; // Make it bigger and more visible

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.f, 1.f, 0.f, 1.f, -1.f, 1.f);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// Draw filled circle
		glColor4f(1, 1, 1, 0.4f); // transparent white in the middle

		int segments = 32; // Number of segments for circle smoothness
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(centerX, centerY); // Center point
		for (int i = 0; i <= segments; i++)
		{
			float angle = 2.0f * M_PI * i / segments;
			float x = centerX + screenRadius * cos(angle);
			float y = centerY + screenRadius * sin(angle);
			glVertex2f(x, y);
		}
		glEnd();

		// Draw circle outline
		glLineWidth(2.0);

		if (isAdding)
			glColor4f(0.1, 0.1, 1.f, 0.5f); // adding -> blue
		else
			glColor4f(1.0, 0.1, 0.1, 0.5f); // removing -> red

		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < segments; i++)
		{
			float angle = 2.0f * M_PI * i / segments;
			float x = centerX + screenRadius * cos(angle);
			float y = centerY + screenRadius * sin(angle);
			glVertex2f(x, y);
		}
		glEnd();

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}
};
#endif