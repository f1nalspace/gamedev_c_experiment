#include "win32_render_opengl.h"

#include <Windows.h>
#include <gl\gl.h>

external void Win32RenderOpenGLInit() {
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

external void Win32RenderOpenGL(RenderState *renderState) {
	glViewport(renderState->viewportOffset.x, renderState->viewportOffset.y, renderState->viewportSize.x, renderState->viewportSize.y);

	F32 worldHalfWidth = renderState->areaSize.w * 0.5f;
	F32 worldHalfHeight = renderState->areaSize.h * 0.5f;

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-worldHalfWidth, worldHalfWidth, -worldHalfHeight, worldHalfHeight, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for (U32 commandIndex = 0; commandIndex < renderState->commandCount; ++commandIndex) {
		RenderCommand *command = renderState->commands + commandIndex;

		// NOTE(final): This is not fast by any means
		Mat4f scaleMat = Mat4ScaleFromVec3(V3(command->transform.scale.x, command->transform.scale.y, 0.0f));
		Mat4f translationMat = Mat4TranslationFromVec2(command->transform.pos);
		Mat4f rotationMat = Mat4RotationFromMat2(command->transform.rot);
		Mat4f modelviewMat = scaleMat * translationMat * rotationMat;

		switch (command->type) {
			case RenderCommandType::RenderCommandType_Clear:
			{
				glClearColor(command->color.r, command->color.g, command->color.b, command->color.a);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}; break;
			case RenderCommandType::RenderCommandType_Lines:
			{
				RenderCommandLines *lines = &command->lines;
				glLoadMatrixf(&modelviewMat.m[0]);
				glColor4fv(&command->color.p[0]);
				glLineWidth(lines->lineWidth);
				glBegin(lines->isChained ? GL_LINE_LOOP : GL_LINES);
				for (U32 vertexIndex = 0; vertexIndex < lines->vertexCount; ++vertexIndex) {
					Vec2f *v = lines->verts + vertexIndex;
					glVertex2fv(&v->p[0]);
				}
				glEnd();
				glLineWidth(1.0f);
			}; break;
		}
	}
}