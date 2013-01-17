#ifndef GFXOPENGL_H
#define GFXOPENGL_H

#define PI		(3.14159265359f)
#define DEG2RAD(a)	(PI/180*(a))
#define RAD2DEG(a)	(180/PI*(a))

enum BoardPiece_tag {NONE, X_PLAYER, O_PLAYER};
typedef enum BoardPiece_tag BoardPiece;

struct GfxOpenGL_tag
{
	/* Used to make sure that Render() is not called before Init() */
	bool ready;

	unsigned m_windowWidth;
	unsigned m_windowHeight;
	GLfloat lineWidth;
	GLUquadricObj* circle; /* For drawing the O's */

	BoardPiece board[9];
	BoardPiece player; /* Who's up? */
	bool nextClickRestart;
};
typedef struct GfxOpenGL_tag GfxOpenGL;

float ConvScrCoordsX(GfxOpenGL *gfx, unsigned coord);
float ConvScrCoordsY(GfxOpenGL *gfx, unsigned coord);
bool gfxInit(GfxOpenGL *gfx);
void gfxSetupProjection(GfxOpenGL *gfx, unsigned width, unsigned height);
bool gfxShutdown(GfxOpenGL *gfx);
void gfxRender(GfxOpenGL *gfx);
void gfxMousePick(GfxOpenGL *gfx, unsigned xScr, unsigned yScr);

#endif /* not GFXOPENGL_H */
