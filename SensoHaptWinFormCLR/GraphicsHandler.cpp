#include "stdafx.h"

#include "common_variables.h"
#include "GraphicsHandler.h"

#define LIGHT_POSITION_XYZW 0.25f,1.0f,0.25f,0.0f //Homogeneous coordinates (X,Y,Z,W)
#define LIGHT_DIFFUSE_RGBA 1.0f,1.0f,1.0f,1.0f //Red, Green, Blue, Alpha
#define LIGHT_AMBIENT_RGBA 0.35f,0.35f,0.35f,1.0f 
#define LIGHT_SPECULAR_RGBA 0.7f,0.7f,0.7f,1.0f 

#define BACKGROUND_COLOR_RGBA 0.6f,0.4f,0.3f,1.0f

#define CUBE_SIZE 0.05

using namespace System::Diagnostics;

////////////////////////////////////////////////////////////////////////////////////////////////////

//COLOR RELATED CONSTANTS

//Different grayscale colours with varying intensities (used for emulating light in the environment)					
constexpr GLfloat gray01[] = { 0.1,0.1,0.1 };
constexpr GLfloat gray04[] = { 0.4,0.4,0.4 };
constexpr GLfloat gray06[] = { 0.6,0.6,0.6 };
constexpr GLfloat gray08[] = { 0.8,0.8,0.8 };

//Specular colors defined using RGBA
constexpr GLfloat cube_specular_material[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//Specular exponent can be an integer in range 0-128
constexpr GLfloat cube_specular_exponent = 32.0f;

////////////////////////////////////////////////////////////////////////////////////////////////////

//Initializing static variables
bool GraphicsHandler::stereo_enabled;//= true;
bool GraphicsHandler::cam_mirrored;
GLfloat GraphicsHandler::cam_rotation_adjust = 22;
int GraphicsHandler::current_frame_index = 59; //no delay
HANDLE GraphicsHandler::stopGraphicsUpdateEvent;
double GraphicsHandler::last_z_angle;
std::array<GLfloat, 4> GraphicsHandler::displayed_cube_colour{ 1.0f,1.0f,0.0f,1.0f };
hduMatrix GraphicsHandler::last_transform_matrix;
HDdouble GraphicsHandler::last_inverse_transform_matrix[4][4] = {
	{ 0,0,0,0 },
	{ 0,0,0,0 },
	{ 0,0,1,0 },
	{ 0,0,0,1 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsHandler::GraphicsHandler()
{
	//Initialize default graphics handler values
#ifndef NO_HARDWARE_DEBUG
	GraphicsHandler::stereo_enabled = true;
#else
	GraphicsHandler::stereo_enabled = false;
#endif
	GraphicsHandler::cam_mirrored = true;
	GraphicsHandler::stopGraphicsUpdateEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"stopGraphicsUpdateEvent"
	);
	if (stopGraphicsUpdateEvent == NULL) ErrorExit(L"Failed to create stopGraphicsUpdateEvent");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created stopGraphicsUpdateEvent", L"GraphicsHandler");
#endif

	//Start graphics thread
	thread_handle = CreateThread(
		NULL,
		0,
		graphicsThread,
		NULL,
		0,
		&thread_ID);

	if (thread_handle == NULL) ErrorExit(L"Failed to create graphics thread");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Created graphics thread with id: " + thread_ID.ToString(), L"GraphicsHandler");
#endif
}

GraphicsHandler::~GraphicsHandler()
{
	//Signal fpsTimer to stop
	if (!(SetEvent(GraphicsHandler::stopGraphicsUpdateEvent))) ErrorExit(L"Failed to signal stopGraphicsUpdateEvent");

	//Stop graphicsthread, unfortunately termination is the only way (GLUT doesn't support multithreading)
	if (!(TerminateThread(thread_handle, 0))) ErrorExit(L"Failed to terminate graphics thread");
#ifdef LOG_DEBUG
	else Debug::WriteLine(L"Graphics thread terminated", L"GraphicsHandler");
#endif

	//Wait until graphics thread has been signaled
	if (WaitForSingleObject(thread_handle, INFINITE) == WAIT_OBJECT_0)
	{
#ifdef LOG_DEBUG
		Debug::WriteLine(L"Graphics thread signaled", L"GraphicsHandler");
#endif
	}

	//Close system handles
	if (!(CloseHandle(GraphicsHandler::stopGraphicsUpdateEvent))) ErrorExit(L"Failed to close graphics update event");
	if (!(CloseHandle(thread_handle))) ErrorExit(L"Failed to close graphics thread handle");
}


DWORD WINAPI graphicsThread(LPVOID lpParam)
{
	//Enable stereo mode only if GPU supports it, otherwise System exception is thrown (crash)
#ifndef NO_HARDWARE_DEBUG
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STEREO); //GLUT display config (GLUT STEREO enables 3D functionality in GPU, supported GPU needed to run code)
#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
#endif

	//Create 3D Window
	glutInitWindowPosition(0, 0); 
	glutInitWindowSize(1280, 720); 
	glutCreateWindow("Experiment 3D Simulation");

	//Set background color
	constexpr GLfloat backgroundColor[] = { BACKGROUND_COLOR_RGBA };
	glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]); //bepaalt de kleur die pixels krijgen als glClear wordt uitgevoerd op de buffers (default color dus background color)
	
	//Initialize GL modes
	glShadeModel(GL_SMOOTH); //Smooth shading voor interpolatie van kleuren tussen vertices (realistischer dan flat shading)
	glEnable(GL_DEPTH_TEST); //nodig voor dieptetest waarbij hidden surface removal aan de pas komt
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL); //Materials will now track the current color (gives nice dynamic coloring method)

	//Initializing light used for color calculations of cube
	constexpr GLfloat lightPosition[] = { LIGHT_POSITION_XYZW };
	constexpr GLfloat lightDiffuse[] = { LIGHT_DIFFUSE_RGBA };
	constexpr GLfloat lightAmbient[] = { LIGHT_AMBIENT_RGBA };
	constexpr GLfloat lightSpecular[] = { LIGHT_SPECULAR_RGBA };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

	//Configuring the glut functions used in the glut event processing loop (glutMainLoop)
	glutDisplayFunc(displayFunc);
	glutSpecialFunc(specialFunc);
	glutReshapeFunc(reshapeFunc);
	glutKeyboardFunc(keyboardFunc);

	//Starting the sync function (to avoid flickering)
	fpsTimer(0);

	glutMainLoop();
	return 0;
}


void displayFunc()
{
	if (GraphicsHandler::stereo_enabled)
	{
		// Drawing code for right eye

		// Select the right-eye back buffer as the drawing destination
		glDrawBuffer(GL_BACK_RIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// View frustum settings
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(GraphicsHandler::left_righteye, GraphicsHandler::right_righteye, GraphicsHandler::bottom_righteye, GraphicsHandler::top_righteye, SCREEN_NEAR, SCREEN_FAR); //glFrustum nodig voor stereoscopisch (ipv gluPerspective())											  
		//Camera settings
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// Flip the image if there is a mirror
		if (GraphicsHandler::cam_mirrored) glScaled(1.0, -1.0, 1.0);
		// Viewpoint position setting
		glTranslatef(GraphicsHandler::viewpoint_rightX, GraphicsHandler::viewpoint_rightY, GraphicsHandler::viewpoint_rightZ);
		// Rotation adjustment (around X-axis)
		glRotatef(GraphicsHandler::cam_rotation_adjust, 1.0, 0.0, 0.0);
		// Drawing the scene
		glPushMatrix();
		drawScene();
		glPopMatrix();


		// Drawing code for left eye

		// Select left eye back buffer as drawing destination
		glDrawBuffer(GL_BACK_LEFT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// View frustum settings
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(GraphicsHandler::left_lefteye, GraphicsHandler::right_lefteye, GraphicsHandler::bottom_lefteye, GraphicsHandler::top_lefteye, SCREEN_NEAR, SCREEN_FAR);
		// Camera settings
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// flip upside down
		if (GraphicsHandler::cam_mirrored) glScaled(1.0, -1.0, 1.0);
		// Viewpoint position setting
		glTranslatef(GraphicsHandler::viewpoint_leftx, GraphicsHandler::viewpoint_lefty, GraphicsHandler::viewpoint_leftz);
		// Rotation adjustment (around X-axis)
		glRotatef(GraphicsHandler::cam_rotation_adjust, 1.0, 0.0, 0.0);
		// Drawing the scene
		glPushMatrix();
		drawScene();
		glPopMatrix();
	}
	else // Drawing processing for 2D display (writing only right eye output to window)
	{
		glDrawBuffer(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// View frustum settings
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(GraphicsHandler::left_righteye, GraphicsHandler::right_righteye, GraphicsHandler::bottom_righteye, GraphicsHandler::top_righteye, SCREEN_NEAR, SCREEN_FAR); //glFrustum nodig voor stereoscopisch (ipv gluPerspective())											  
																																											  //Camera settings
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// Flip the image if there is a mirror
		if (GraphicsHandler::cam_mirrored) glScaled(1.0, -1.0, 1.0);
		// Viewpoint position setting
		glTranslatef(GraphicsHandler::viewpoint_rightX, GraphicsHandler::viewpoint_rightY, GraphicsHandler::viewpoint_rightZ);
		// Rotation adjustment (around X-axis)
		glRotatef(GraphicsHandler::cam_rotation_adjust, 1.0, 0.0, 0.0);
		// Drawing the scene
		glPushMatrix();
		drawScene();
		glPopMatrix();
	}

	// Swap buffers (display new data)
	glutSwapBuffers(); //swapt huidige display buffers met nieuwe gecalculeerde buffer (impliciete call naar glFlush verzekert dat deze berekeningen al gedaan zijn)
}

//Callback function: called at window resize
void reshapeFunc(int width, int height)
{	
	glViewport(0, 0, width, height); // Set viewport (clipping area)
}

//Display func is called according to the refresh rate of the used monitor (removed flickering)
void fpsTimer(int value) {
	if (WaitForSingleObject(GraphicsHandler::stopGraphicsUpdateEvent, 0) == WAIT_TIMEOUT)
	{
		glutPostRedisplay();
		glutTimerFunc(1000 / FRAMES_PER_SECOND, fpsTimer, 0);
	}
} 

void drawScene()
{
	glPushMatrix();
	drawEnvironment();
	glPopMatrix();

	glEnable(GL_LIGHTING); //re-enable lighting after drawEnvironment disabled it (misschien niet nodig, testen!)

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cube_specular_material);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &cube_specular_exponent);
	//Note: diffuse and ambient materials don't need to be defined because the current color is tracked (glColor3f)
	
	if (ExperimentCommon::experiment_state == ExperimentCommon::State::TRIAL)
	{
		glColor4f(GraphicsHandler::displayed_cube_colour.at(0), GraphicsHandler::displayed_cube_colour.at(1), GraphicsHandler::displayed_cube_colour.at(2), GraphicsHandler::displayed_cube_colour.at(3)); //Current color (blue), niet zeker waarom deze hier staat
#ifndef NO_HARDWARE_DEBUG
		GraphicsHandler::last_z_angle = ExperimentCommon::z_angle_buffer.at(GraphicsHandler::current_frame_index-1);
		GraphicsHandler::last_transform_matrix = ExperimentCommon::transform_buffer.at(GraphicsHandler::current_frame_index - 1);
		GraphicsHandler::last_inverse_transform_matrix[0][0] = cos(GraphicsHandler::last_z_angle * 2);
		GraphicsHandler::last_inverse_transform_matrix[0][1] = sin(GraphicsHandler::last_z_angle * 2);
		GraphicsHandler::last_inverse_transform_matrix[1][0] = -sin(GraphicsHandler::last_z_angle * 2);
		GraphicsHandler::last_inverse_transform_matrix[1][1] = cos(GraphicsHandler::last_z_angle * 2);
#endif
	}
	glTranslatef(0, GROUND_CALIBRATION, 0); //add Y ground calibration
#ifndef NO_HARDWARE_DEBUG
	glMultMatrixd(GraphicsHandler::last_transform_matrix); //Transformation matrix fetched from haptic device
	glMultMatrixd(hduMatrix(GraphicsHandler::last_inverse_transform_matrix)); //Adjusted transformation due to faulty installation of setup
#endif
	glutSolidCube(CUBE_SIZE); //Drawing the cube 
}

void drawEnvironment()
{
	glDisable(GL_LIGHTING); //Light is simulated using the grayscale colours

	//Background checkered wall
	for (int k = 0; k < 2; k++)
		for (int l = 0; l < 8; l++) {
			glPushMatrix(); 
			glTranslatef(-0.15 + (l*0.04), -0.040 + GROUND_CALIBRATION + (k*0.04), -0.144 + GROUND_CALIBRATION_Z);
			glColor3f(gray04[0], gray04[1], gray04[2]); 
			glutSolidCube(0.020); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(-0.15 + (l*0.04), -0.020 + GROUND_CALIBRATION + (k*0.04), -0.144 + GROUND_CALIBRATION_Z);
			glColor3f(gray08[0], gray08[1], gray08[2]);
			glutSolidCube(0.020); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(-0.13 + (l*0.04), -0.040 + GROUND_CALIBRATION + (k*0.04), -0.144 + GROUND_CALIBRATION_Z);
			glColor3f(gray06[0], gray06[1], gray06[2]);
			glutSolidCube(0.020); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(-0.13 + (l*0.04), -0.020 + GROUND_CALIBRATION + (k*0.04), -0.144 + GROUND_CALIBRATION_Z);
			glColor3f(gray01[0], gray01[1], gray01[2]);
			glutSolidCube(0.020);	
			glPopMatrix();
		}

	//Right checkered wall
	for (int m = 0; m < 1; m++)
		for (int n = 0; n < 3; n++) {
			glPushMatrix(); 
			glTranslatef(0.180, -0.030 + GROUND_CALIBRATION + (m*0.08), -0.115 + GROUND_CALIBRATION_Z + (n*0.08));
			glColor3f(gray04[0], gray04[1], gray04[2]); 
			glutSolidCube(0.040); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(0.180, -0.030 + GROUND_CALIBRATION + (m*0.08), -0.075 + GROUND_CALIBRATION_Z + (n*0.08));
			glColor3f(gray08[0], gray08[1], gray08[2]);
			glutSolidCube(0.040); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(0.180, 0.010 + GROUND_CALIBRATION + (m*0.08), -0.115 + GROUND_CALIBRATION_Z + (n*0.08));
			glColor3f(gray06[0], gray06[1], gray06[2]);
			glutSolidCube(0.040); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(0.180, 0.010 + GROUND_CALIBRATION + (m*0.08), -0.075 + GROUND_CALIBRATION_Z + (n*0.08));
			glColor3f(gray01[0], gray01[1], gray01[2]);
			glutSolidCube(0.040); 
			glPopMatrix();
		}

	//Left checkered wall
	for (int lm = 0; lm < 1; lm++)
		for (int ln = 0; ln < 3; ln++) {
			glPushMatrix(); 
			glTranslatef(-0.180, -0.030 + GROUND_CALIBRATION + (lm*0.08), -0.115 + GROUND_CALIBRATION_Z + (ln*0.08));
			glColor3f(gray04[0], gray04[1], gray04[2]);
			glutSolidCube(0.040); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(-0.180, -0.030 + GROUND_CALIBRATION + (lm*0.08), -0.075 + GROUND_CALIBRATION_Z + (ln*0.08));
			glColor3f(gray08[0], gray08[1], gray08[2]);
			glutSolidCube(0.040); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(-0.180, 0.010 + GROUND_CALIBRATION + (lm*0.08), -0.115 + GROUND_CALIBRATION_Z + (ln*0.08));
			glColor3f(gray06[0], gray06[1], gray06[2]);
			glutSolidCube(0.040); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(-0.180, 0.010 + GROUND_CALIBRATION + (lm*0.08), -0.075 + GROUND_CALIBRATION_Z + (ln*0.08));
			glColor3f(gray01[0], gray01[1], gray01[2]);
			glutSolidCube(0.040); 
			glPopMatrix();
		}

	//Checkered ground
	for (int j = 0; j < 17; j++)
		for (int i = 0; i < 14; i++) {
			glPushMatrix(); 
			glTranslatef(-0.165 + (j*0.02), -0.0549 + GROUND_CALIBRATION, -0.130 + GROUND_CALIBRATION_Z + (i*0.02));
			glColor3f(gray04[0], gray04[1], gray04[2]);
			glutSolidCube(0.010); 
			glPopMatrix();

			glPushMatrix();
			glTranslatef(-0.165 + (j*0.02), -0.0549 + GROUND_CALIBRATION, -0.120 + GROUND_CALIBRATION_Z + (i*0.02));
			glColor3f(gray08[0], gray08[1], gray08[2]);
			glutSolidCube(0.010); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(-0.155 + (j*0.02), -0.0549 + GROUND_CALIBRATION, -0.130 + GROUND_CALIBRATION_Z + (i*0.02));
			glColor3f(gray06[0], gray06[1], gray06[2]);
			glutSolidCube(0.010); 
			glPopMatrix();

			glPushMatrix(); 
			glTranslatef(-0.155 + (j*0.02), -0.0549 + GROUND_CALIBRATION, -0.120 + GROUND_CALIBRATION_Z + (i*0.02));
			glColor3f(gray01[0], gray01[1], gray01[2]); 
			glutSolidCube(0.010);
			glPopMatrix();
		}
}

