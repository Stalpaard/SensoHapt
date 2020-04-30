#include "stdafx.h"

#include "ExperimentController.h"
#include "HapticHandler.h"
#include "GraphicsHandler.h"

#define GLUT_KEY_ESC = 0x1b;
#define GLUT_KEY_BACKSPACE = 0x08;
#define GLUT_KEY_DELETE = 0x10;

using namespace System::Diagnostics;

void keyboardFunc(unsigned char key, int x, int y)
{
	//Process key events
	switch (key)
	{
	case 'e':
	{
	}
	break;
	case 'd':
	{
	}
	break;
	case 'r':
	{
	}
	break;
	case 'f':
	{
	}
	break;
	case 'h':
	{
	}
	break;
	case 'g':
	{
	}
	break;
	case 's':
	{
	}
	break;
	case 'm':
	{
	}
	break;
	case '[':
	{
	}
	break;
	case ']':
	{
	}
	break;
	
	}
}

void specialFunc(int key, int x, int y)
{
	//Process key events of special keyboard keys
	switch (key)
	{
	case GLUT_KEY_F1: break;
	case GLUT_KEY_F2: break;
	case GLUT_KEY_F3: break;
	case GLUT_KEY_F4: break;
	case GLUT_KEY_F5: break;
	case GLUT_KEY_F6: break;
	case GLUT_KEY_F7: break;
	case GLUT_KEY_F8: break;
	case GLUT_KEY_F9: break;
	case GLUT_KEY_F10: break;
	case GLUT_KEY_F11: break;
	case GLUT_KEY_F12: break;
	case GLUT_KEY_LEFT: break;
	case GLUT_KEY_UP:
	case GLUT_KEY_RIGHT: break;
	case GLUT_KEY_DOWN:
	case GLUT_KEY_PAGE_UP: break;
	case GLUT_KEY_PAGE_DOWN: break;
	case GLUT_KEY_HOME: break;
	case GLUT_KEY_END: break;
	case GLUT_KEY_INSERT: break;
	}
}
