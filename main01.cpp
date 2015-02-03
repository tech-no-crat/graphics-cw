/************************************************************************/
/*    Graphics 317 coursework exercise 01                               */
/*    Author: Bernhard Kainz                                            */
/*    This file has to be altered for this exercise                     */
/************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <GL/glew.h>

#ifdef __linux__
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

///////////////////////////////////////////////////////////////////////
// Helper for submission
#include "inc/lodepng.h"

///////////////////////////////////////////////////////////////////////
// Declarations
void captureFrame();

void CheckOpenGLError(const char* stmt, const char* fname, int line);
#define _DEBUG 1
#if _DEBUG
#define GL_CHECK(stmt) do { \
  stmt; \
  CheckOpenGLError(#stmt, __FILE__, __LINE__); \
} while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

///////////////////////////////////////////////////////////////////////
// Shaders and light pos variables
GLuint v, f, p, g;
float lpos[4] = { 15.0, 0.5, 15.0, 0.0 };
int subdivLevel;
GLuint tex;

///////////////////////////////////////////////////////////////////////
// Mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
float move_x = 0.0, move_y = 0.0;
unsigned int win_width = 128, win_height = 128;
float translate_z = 0.0;
unsigned int frameCaptured = 0;

//////////////////////////////////////////////////////////////////////
// TODO finish the following three functions to complete Exercise 1
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// The actual render function, which is called for each frame

void renderScene(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0f, 1.0f, 0.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glLightfv(GL_LIGHT0, GL_POSITION, lpos);
  /////////////////////////////////////////////////
  //TODO add scene interaction code here
  glTranslatef(move_x, move_y, translate_z);
  glRotatef(rotate_y, 1, 0, 0);
  glRotatef(rotate_x, 0, 1, 0);
  /////////////////////////////////////////////////
  GL_CHECK(glUseProgram(p));
  glutSolidTeapot(0.5);
  GL_CHECK(glUseProgram(0));

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Helper function for submission. Will capture 2nd frame of task
  frameCaptured++;
  if (frameCaptured == 2)
  {
    renderID(win_width, win_height);
    captureFrame();
  }

  GL_CHECK(glutSwapBuffers());
}

///////////////////////////////////////////////////////////////////////
// Mouse interaction functions

#define MOUSE_BTN_MASK(btnType) (1 << btnType)
#define MOUSE_BTN_DOWN(btnType) \
  (mouse_buttons && MOUSE_BTN_MASK(btnType))

void mouseClick(int button, int state, int x, int y)
{
  /////////////////////////////////////////////////
  // TODO add code to handle mouse click events
  // use GLUT_UP and GLUT_DOWN to evaluate the current
  // "state" of the mouse.
  /////////////////////////////////////////////////

  if (state == GLUT_DOWN)
  {
    mouse_buttons |= MOUSE_BTN_MASK(button);
    mouse_old_x = x;
    mouse_old_y = y;
  }
  else if (state == GLUT_UP)
  {
    mouse_buttons &= ~MOUSE_BTN_MASK(button);
  }

  /////////////////////////////////////////////////
}

void mouseMotion(int x, int y)
{
  /////////////////////////////////////////////////
  // TODO add code to handle mouse move events
  // and calculate reasonable values for object
  // rotations
  /////////////////////////////////////////////////

  float dx = (float)(x - mouse_old_x)
      , dy = (float)(y - mouse_old_y);

  if (MOUSE_BTN_DOWN(GLUT_LEFT_BUTTON))
  {
    rotate_x += dx;
    rotate_y += dy;
  }
  if (MOUSE_BTN_DOWN(GLUT_RIGHT_BUTTON))
  {
    translate_z += dy * 0.1f;
  }
  if (MOUSE_BTN_DOWN(GLUT_MIDDLE_BUTTON))
  {
    move_x += dx * (1.0f / win_width);
    move_y += dy * (1.0f / win_height);
  }

  mouse_old_x = x;
  mouse_old_y = y;

  /////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////
// Nothing to do from here on
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Text file IO for shader files

char *textFileRead(char *fn)
{
  FILE *fp;
  char *content = NULL;

  int count = 0;

  if (fn != NULL) {
    fp = fopen(fn, "rt");

    if (fp != NULL) {

      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

      if (count > 0) {
        content = (char *)malloc(sizeof(char) * (count + 1));
        count = fread(content, sizeof(char), count, fp);
        content[count] = '\0';
      }
      fclose(fp);
    }
  }

  if (content == NULL)
  {
    fprintf(stderr, "ERROR: could not load in file %s\n", fn);
    exit(1);
  }
  return content;
}

///////////////////////////////////////////////////////////////////////
// Helper for submission

void captureFrame()
{
  unsigned char* pixels = new unsigned char[3 * win_width * win_height];
  glReadPixels(0, 0, win_width, win_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  encodeOneStep(pixels, win_width, win_height, std::string("01"));
  delete[] pixels;
}

///////////////////////////////////////////////////////////////////////
// Keyboard functions

void processNormalKeys(unsigned char key, int x, int y)
{
  if (key == 27)
    exit(0);
}

///////////////////////////////////////////////////////////////////////
// Adapt viewport when window size changes

void changeSize(int w, int h)
{
  // Prevent a divide by zero, when window is too short
  // (you cant make a window of zero width).
  if (h == 0)
    h = 1;

  float ratio = 1.0* w / h;

  // Reset the coordinate system before modifying
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Set the viewport to be the entire window
  glViewport(0, 0, w, h);

  // Set the correct perspective.
  gluPerspective(45, ratio, 0.1, 1000);
  glMatrixMode(GL_MODELVIEW);
  win_width = w;
  win_height = h;
}

///////////////////////////////////////////////////////////////////////
// Load, compile and set the shaders

void setShaders()
{
  char *vs, *fs, *gs;

  v = glCreateShader(GL_VERTEX_SHADER);
  f = glCreateShader(GL_FRAGMENT_SHADER);
  g = glCreateShader(GL_GEOMETRY_SHADER);

  vs = textFileRead(const_cast<char *>("./shader01.vert"));
  fs = textFileRead(const_cast<char *>("./shader01.frag"));
  gs = textFileRead(const_cast<char *>("./shader01.geom"));

  const char * ff = fs;
  const char * vv = vs;
  const char * gg = gs;

  GL_CHECK(glShaderSource(v, 1, &vv, NULL));
  GL_CHECK(glShaderSource(f, 1, &ff, NULL));
  GL_CHECK(glShaderSource(g, 1, &gg, NULL));

  free(vs); free(fs); free(gs);

  GL_CHECK(glCompileShader(v));
  GL_CHECK(glCompileShader(f));
  GL_CHECK(glCompileShader(g));

  GLint blen = 0;
  GLsizei slen = 0;

  glGetShaderiv(v, GL_INFO_LOG_LENGTH, &blen);
  if (blen > 1)
  {
    GLchar* compiler_log = (GLchar*)malloc(blen);
    glGetShaderInfoLog(v, blen, &slen, compiler_log);
    std::cout << "compiler_log vertex shader:\n" << compiler_log << std::endl;
    free(compiler_log);
  }
  blen = 0;
  slen = 0;
  glGetShaderiv(f, GL_INFO_LOG_LENGTH, &blen);
  if (blen > 1)
  {
    GLchar* compiler_log = (GLchar*)malloc(blen);
    glGetShaderInfoLog(f, blen, &slen, compiler_log);
    std::cout << "compiler_log fragment shader:\n" << compiler_log << std::endl;
    free(compiler_log);
  }
  blen = 0;
  slen = 0;
  glGetShaderiv(g, GL_INFO_LOG_LENGTH, &blen);
  if (blen > 1)
  {
    GLchar* compiler_log = (GLchar*)malloc(blen);
    glGetShaderInfoLog(g, blen, &slen, compiler_log);
    std::cout << "compiler_log geometry shader:\n" << compiler_log << std::endl;
    free(compiler_log);
  }

  p = glCreateProgram();

  GL_CHECK(glAttachShader(p, f));
  GL_CHECK(glAttachShader(p, v));
  GL_CHECK(glAttachShader(p, g));

  GL_CHECK(glLinkProgram(p));
  // Comment out this line to not use the shaders at all
  GL_CHECK(glUseProgram(p));
}

void initialize()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLfloat aspect = (GLfloat)320 / 320;
  gluPerspective(45, aspect, 0.1, 1000.0);
  glMatrixMode(GL_MODELVIEW);
  glShadeModel(GL_SMOOTH);
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  GLfloat amb_light[] = { 0.1, 0.1, 0.1, 1.0 };
  GLfloat diffuse[] = { 0.6, 0.6, 0.6, 1 };
  GLfloat specular[] = { 0.7, 0.7, 0.3, 1 };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb_light);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glTranslatef(0.0, 0.0, -1.0);
}

///////////////////////////////////////////////////////////////////////
// Main, setup and execution of environment

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(320, 320);
  glutCreateWindow("Computer Graphics");

  glutDisplayFunc(renderScene);
  glutIdleFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutKeyboardFunc(processNormalKeys);

  glutMouseFunc(mouseClick);
  glutMotionFunc(mouseMotion);

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 1.0);

  glewInit();
  if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
    printf("Ready for GLSL\n");
  else {
    printf("No GLSL support\n");
    exit(1);
  }

  if (glewIsSupported("GL_VERSION_3_1"))
    printf("Ready for OpenGL 3.1\n");
  else {
    printf("OpenGL 3.1 not supported\n");
    exit(1);
  }
  if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
    printf("Ready for GLSL - vertex, fragment, and geometry units\n");
  else {
    printf("Not totally ready :( \n");
    exit(1);
  }
  initialize();
#ifdef __linux__
  int i=pthread_getconcurrency();
#endif
  setShaders();

  glutMainLoop();
  return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// Error checking functions

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
  GLenum err = glGetError();
  if (err != GL_NO_ERROR)
  {
    printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
    abort();
  }
}

