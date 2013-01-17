#ifdef _WINDOWS
#include <windows.h>
#endif

#include <gl/gl.h>
#include <gl/glu.h>
#include "bool.h"

#include "GfxOpenGL.h"

/* Private declarations */
bool gfxIsEndingMove(GfxOpenGL *gfx);

/* Screen coordinate conversions */
float ConvScrCoordsX(GfxOpenGL *gfx, unsigned coord)
{ return (3 * (float)coord - 1.5f * gfx->m_windowWidth) /
		gfx->m_windowHeight; }
float ConvScrCoordsY(GfxOpenGL *gfx, unsigned coord)
{ return -3 * (float)coord / gfx->m_windowHeight + 1.5f; }

bool gfxInit(GfxOpenGL *gfx)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glDepthMask(GL_FALSE);
	gfx->circle = gluNewQuadric();
	gluQuadricNormals(gfx->circle, GLU_NONE);
	memset(gfx->board, 0, sizeof(gfx->board));
	gfx->player = X_PLAYER;
	gfx->nextClickRestart = false;
	gfx->ready = true;
	return true;
}

void gfxSetupProjection(GfxOpenGL *gfx, unsigned width, unsigned height)
{
	if (height == 0) /* Don't want a divide by zero */
	{
		height = 1;					
	}

	glClear(GL_COLOR_BUFFER_BIT);

	/* Set the viewport to the new dimensions.  */
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	{ /* Calculate the aspect ratio of the window.  */
		GLdouble sideSweep = (GLdouble)width / height * 1.5;
		gluOrtho2D(-sideSweep, sideSweep, -1.5, 1.5);
	}

	/* Reset the modelview matrix.  */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gfx->m_windowWidth = width;
	gfx->m_windowHeight = height;

	/* Calculate the line width based off of the window size.  */
	gfx->lineWidth = (GLfloat)gfx->m_windowHeight / 32.0f;
	glLineWidth(gfx->lineWidth);
}

bool gfxShutdown(GfxOpenGL *gfx)
{
	gluDeleteQuadric(gfx->circle);
	return true;
}

void gfxRender(GfxOpenGL *gfx)
{
	float boardPositions[3][3][2] =
		{ { {-1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f} },
		  { {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f} },
		  { {-1.0f, -1.0f}, {0.0f, -1.0f}, {1.0f, -1.0f} } };
	unsigned i;

	if (!gfx->ready)
		return;
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2f(-0.5f, 1.5f);
	glVertex2f(-0.5f, -1.5f);
	glVertex2f(0.5f, 1.5f);
	glVertex2f(0.5f, -1.5f);
	glVertex2f(1.5f, 0.5f);
	glVertex2f(-1.5f, 0.5f);
	glVertex2f(1.5f, -0.5f);
	glVertex2f(-1.5f, -0.5f);
	glEnd();

	for (i = 0; i < 3; i++)
	{
		unsigned j;
		for (j = 0; j < 3; j++)
		{
			if (gfx->board[i*3+j] == X_PLAYER)
			{
				glPushMatrix();
				glTranslatef(boardPositions[i][j][0],
							 boardPositions[i][j][1], 0.0f);
				glColor3f(1.0f, 0.0f, 0.0f);
				glBegin(GL_QUADS);
				/* +, +: 0.392520630368f */
				/* +, -: 0.326229369632f */
				/* -, +: -0.326229369632f */
				/* -, -: -0.392520630368f */
				glVertex2f(0.392520630368f, 0.326229369632f);
				glVertex2f(0.326229369632f, 0.392520630368f);
				glVertex2f(-0.392520630368f, -0.326229369632f);
				glVertex2f(-0.326229369632f, -0.392520630368f);

				glVertex2f(-0.326229369632f, 0.392520630368f);
				glVertex2f(-0.392520630368f, 0.326229369632f);
				glVertex2f(0.326229369632f, -0.392520630368f);
				glVertex2f(0.392520630368f, -0.326229369632f);
				glEnd();
				glPopMatrix();
			}
			if (gfx->board[i*3+j] == O_PLAYER)
			{
				glPushMatrix();
				glTranslatef(boardPositions[i][j][0],
							 boardPositions[i][j][1], 0.0f);
				glColor3f(0.0f, 0.625f, 0.0f);
				gluDisk(gfx->circle, 0.3125, 0.40625, 32, 1);
				glPopMatrix();
			}
		}
	}
}

void gfxMousePick(GfxOpenGL *gfx, unsigned xScr, unsigned yScr)
{
	float x = ConvScrCoordsX(gfx, xScr);
	float y = ConvScrCoordsY(gfx, yScr);
	unsigned row, column;

	if (gfx->nextClickRestart == true)
	{
		gfx->nextClickRestart = false;
		memset(gfx->board, 0, sizeof(gfx->board));
		gfx->player = X_PLAYER;
		return;
	}

	if (y > 1.5f)
		return;
	else if (y > 0.5f)
		row = 0;
	else if (y > -0.5f)
		row = 1;
	else if (y > -1.5f)
		row = 2;
	else
		return;

	if (x < -1.5f)
		return;
	else if (x < -0.5f)
		column = 0;
	else if (x < 0.5f)
		column = 1;
	else if (x < 1.5f)
		column = 2;
	else
		return;

	if (gfx->board[row*3+column] == NONE)
	{
		gfx->board[row*3+column] = gfx->player;
		if (gfx->player == X_PLAYER)
			gfx->player = O_PLAYER;
		else if (gfx->player == O_PLAYER)
			gfx->player = X_PLAYER;
	}
	else
		return;
	if (gfxIsEndingMove(gfx))
		gfx->nextClickRestart = true;
}

bool gfxIsEndingMove(GfxOpenGL *gfx)
{
	unsigned winningMoves[8][3] =
		{ {0, 4, 8}, {2, 4, 6}, /* Wins through center with corners */
		  {1, 4, 7}, {3, 4, 5}, /* Wins through center with edges */
		  /* Wins on outer rim */
		  {2, 1, 0}, {0, 3, 6}, {6, 7, 8}, {8, 5, 2} };
	unsigned i;

	/* Check for wins.  */
	for (i = 0; i < 8; i++)
	{
		BoardPiece winner = NONE;
		unsigned j;
		if (gfx->board[winningMoves[i][0]] == NONE)
			continue;
		winner = gfx->board[winningMoves[i][0]];
		for (j = 1; j < 3; j++)
		{
			if (gfx->board[winningMoves[i][j]] != winner)
			{
				winner = NONE;
				break;
			}
		}
		if (winner != NONE)
			return true;
	}

	/* Check for a tie.  */
	{
		bool boardFilled = true;
		for (i = 0; i < 3; i++)
		{
			unsigned j;
			for (j = 0; j < 3; j++)
			{
				if (gfx->board[i*3+j] == NONE)
					boardFilled = false;
			}
		}
		if (boardFilled == true)
			return true;
	}

	return false;
}
