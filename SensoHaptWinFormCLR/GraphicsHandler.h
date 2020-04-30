#pragma once
#ifndef _GRAPHICSHANDLER
#define _GRAPHICSHANDLER

#define STEREO_IOD 0.045 // interocular distance [m]
#define SCREEN_WIDTH 0.53 // actual screen width [m]
#define SCREEN_HEIGHT 0.3 // Actual screen height [m]s
#define SCREEN_DISTANCE 0.9 // Optical distance to screen [m]
#define SCREEN_NEAR 0.1 // Near clip plane [m]
#define SCREEN_FAR 10.0 // far clip plane [m]
#define GROUND_CALIBRATION 0.01 //Y ground calibration (used for objects & environment) //BEIDE GROUND CALIBRATIONS KUNNEN NOG GEÏNTEGREERD WORDEN MSS (TESTEN!)
#define GROUND_CALIBRATION_Z 0.0 //Z ground calibration (used for environment)
#define CAM_POSX 0.0 //Offset of camera viewpoint
#define CAM_POSY 0.0
#define CAM_POSZ 0.9

#include "HDU\hduMatrix.h"
#include "HD\hdDefines.h"
#include "GL\glut.h"
#include <array>

//Forward declarations
void fpsTimer(int value);
void reshapeFunc(int width, int height);
void drawScene();
void drawEnvironment();
DWORD WINAPI graphicsThread(LPVOID lpParam);

//Class starting the OpenGL thread and describing its behaviour
class GraphicsHandler {
	friend void fpsTimer(int value);
	friend void keyboardFunc(unsigned char key, int x, int y);
	friend void specialFunc(int key, int x, int y);
	friend void drawScene();
	friend void displayFunc();
public:
	GraphicsHandler();
	~GraphicsHandler();

	static int current_frame_index; //No thread synchronization needed because of no possible concurrent read/write operations (consequence of global experiment state variable)
	static std::array<GLfloat, 4> displayed_cube_colour;

private:
	
	static HDdouble last_inverse_transform_matrix[4][4];
	static hduMatrix last_transform_matrix;
	static double last_z_angle;

	static bool stereo_enabled, //Enable stereoscopic 3D view (can be bound to keys in KeyboardCallbackFunctions / needs synchronization)
		cam_mirrored; //Mirror cam (because of the mirror in the setup) (can be bound to keys in KeyboardCallbackFunctions / needs synchronization)

	static GLfloat cam_rotation_adjust; // Tilt correction value when using mirror [deg] (can be bound to keys in KeyboardCallbackFunctions / needs synchronization)

	constexpr static GLdouble left_righteye = ((-SCREEN_WIDTH / 2.0) - (STEREO_IOD / 2.0)) * (SCREEN_NEAR / SCREEN_DISTANCE),
		right_righteye = ((SCREEN_WIDTH / 2.0) - (STEREO_IOD / 2.0)) * (SCREEN_NEAR / SCREEN_DISTANCE),
		bottom_righteye = (-SCREEN_HEIGHT / 2.0) * (SCREEN_NEAR / SCREEN_DISTANCE),
		top_righteye = (SCREEN_HEIGHT / 2.0) * (SCREEN_NEAR / SCREEN_DISTANCE),
		left_lefteye = ((-SCREEN_WIDTH / 2.0) + (STEREO_IOD / 2.0)) * (SCREEN_NEAR / SCREEN_DISTANCE),
		right_lefteye = ((SCREEN_WIDTH / 2.0) + (STEREO_IOD / 2.0)) * (SCREEN_NEAR / SCREEN_DISTANCE),
		bottom_lefteye = bottom_righteye,
		top_lefteye = top_righteye;
	
	constexpr static GLfloat viewpoint_rightX = (-CAM_POSX - (STEREO_IOD/2.0)), 
		viewpoint_rightY = (-CAM_POSY), 
		viewpoint_rightZ = (-CAM_POSZ),
		viewpoint_leftx = (-CAM_POSX + (STEREO_IOD / 2.0)),
		viewpoint_lefty = viewpoint_rightY, 
		viewpoint_leftz = viewpoint_rightZ;

	HANDLE thread_handle;
	static HANDLE stopGraphicsUpdateEvent;
	DWORD thread_ID;

};

#endif

