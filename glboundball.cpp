// Last updated: <2022/09/19 00:36:36 +0900>
//
// GL Bound Ball screen saver by mieki256.
//
// Original open-source example screensaver by Rachel Grey, lemming@alum.mit.edu.
//
//
// Use Windows10 x64 21H2 + MinGW (gcc 9.2.0) + scrnsave.h + libscrnsave.a
//
// Build:
//
// windres resource.rc resource.o
// g++ -c glboundball.cpp
// g++ glboundball.o resource.o -o glboundball.scr -static -lstdc++ -lgcc -lscrnsave -lopengl32 -lglu32 -lgdi32 -lcomctl32 -lwinmm -mwindows

#define _USE_MATH_DEFINES

#include <windows.h>
#include <scrnsave.h>
#include <math.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "resource.h"

// get rid of these warnings: truncation from const double to float conversion from double to float
// #pragma warning(disable: 4305 4244)

//Define a Windows timer
#define TIMER           1

#define FPS             63
#define FPS_BAR         0

#define BOX_W           20
#define SPHERE_R        0.5

#define DEG2RAD(x)      (double)((x) * M_PI / 180.0)

// globals for size of screen
static int Width, Height;

// static bool bTumble = true;

// a global to keep track of the square's spinning
// static GLfloat spin = 0;

// global work
static double ang = 0.0;
static double ang_d = 0.0;
static GLfloat bx = 0.0;
static GLfloat by = 0.0;
static GLfloat bz = 0.0;
static GLfloat dx = 0.0;
static GLfloat dy = 0.0;
static GLfloat dz = 0.0;
static GLfloat cx = 0.0;
static GLfloat cy = 0.0;
static GLfloat cz = 0.0;
static float hit = 0.0;
static int hit_dir = 0;
static GLUquadricObj* qobj;

static DWORD rec_time;
static int count_frame;
static int count_fps;


// light settings
static GLfloat light0pos[] = {BOX_W * 0.25, BOX_W * 1.0, BOX_W * 0.5, 1.0};
static GLfloat light0dif[] = {1.0, 1.0, 1.0, 1.0};
static GLfloat light0spe[] = {1.0, 1.0, 1.0, 1.0};
static GLfloat light0amb[] = {0.5, 0.5, 1.0, 1.0};
static GLfloat green[] = {0.0, 1.0, 0.0, 1.0};

// Init count FPS work
static void InitCountFps(void)
{
  timeBeginPeriod(1);
  rec_time = timeGetTime();
  count_fps = 0;
  count_frame = 0;
}

// Close count FPS
static void CloseCountFps(void)
{
  timeEndPeriod(1);
}

// count FPS
static void CountFps(void)
{
  count_frame++;
  DWORD t = timeGetTime() - rec_time;

  if (t >= 1000)
    {
      rec_time += 1000;
      count_fps = count_frame;
      count_frame = 0;
    }
  else if (t < 0)
    {
      rec_time = timeGetTime();
      count_fps = 0;
      count_frame = 0;
    }
}


static void InitWork(int box_w, int fps)
{
  ang = 0.0;
  ang_d = 360.0 / (FPS * 30.0);
  bx = 0.0;
  by = 0.0;
  bz = 0.0;
  cx = 0.0;
  cy = 0.0;
  cz = 0.0;
  hit = 0.0;
  hit_dir = 0;
  dx = ((float)box_w / (float)fps) * 0.5;
  dy = dx * 0.3;
  dz = dx * 0.7;
}

// cube vertex position
static GLfloat cube_pos[8][3] =
{
  {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}, {1, -1, -1},
  {-1, -1, 1}, {-1, 1, 1}, {1, 1, 1}, {1, -1, 1}
};

// cube vertex indexes
static int cube_indexs[12][2] =
{
  {0, 1}, {1, 2}, {2, 3}, {3, 0},
  {4, 5}, {5, 6}, {6, 7}, {7, 4},
  {0, 4}, {1, 5}, {2, 6}, {3, 7}
};


// Draw wireframe cube
static void DrawWireCube(GLfloat width)
{
  GLfloat w = width / 2.0;

  for (int i = 0; i < 12; i++)
    {
      int i0 = cube_indexs[i][0];
      int i1 = cube_indexs[i][1];
      GLfloat x0 = cube_pos[i0][0];
      GLfloat y0 = cube_pos[i0][1];
      GLfloat z0 = cube_pos[i0][2];
      GLfloat x1 = cube_pos[i1][0];
      GLfloat y1 = cube_pos[i1][1];
      GLfloat z1 = cube_pos[i1][2];
      glBegin(GL_LINES);
      glVertex3f(w * x0, w * y0, w * z0);
      glVertex3f(w * x1, w * y1, w * z1);
      glEnd();
    }

#if FPS_BAR
  // Draw the FPS bar. Draw a single red line on the grid.
  {
    GLfloat ww = width * (float)count_fps / (float)FPS;
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(-w, -w, w * 0.98);
    glVertex3f(ww - w, -w, w * 0.98);
    glEnd();
  }
#endif
}


// Draw grid lines
static void DrawGrid(GLfloat width)
{
  GLfloat w, x, y, d;
  w = width / 2.0;
  x = -w;
  y = -w;
  d = width / 10.0;

  while (x <= (width / 2))
    {
      glBegin(GL_LINES);
      glVertex3f(x, y, -w);
      glVertex3f(x, y, w);
      glEnd();

      glBegin(GL_LINES);
      glVertex3f(-w, y, x);
      glVertex3f(w, y, x);
      glEnd();

      x += d;
    }
}


// Draw circle
// hit_dir      0 : y-z, 1 : x-z, 2 : x-y
static void DrawCircle(GLfloat bx, GLfloat by, GLfloat bz, GLfloat r, int hit_dir)
{
  GLfloat x, y, z, d;

  glPushMatrix();

  glTranslatef(bx, by, bz);

  if (hit_dir == 0)
    glRotatef(90.0, 0.0, 1.0, 0.0);
  else if (hit_dir == 1)
    glRotatef(90.0, 1.0, 0.0, 0.0);

  y = -r;
  d = float(r) / 5.0;

  while (y <= r)
    {
      x = sqrt((double)(r * r - (y * y)));
      glBegin(GL_LINES);
      glVertex3f(-x, y, 0.0);
      glVertex3f(+x, y, 0.0);
      glEnd();

      glBegin(GL_LINES);
      glVertex3f(y, -x, 0.0);
      glVertex3f(y, +x, 0.0);
      glEnd();

      y += d;
    }

  glPopMatrix();
}


// update angle and position
static void Update()
{
  ang += ang_d;

  if (ang >= 360.0) ang -= 360.0;

  bx += dx;
  by += dy;
  bz += dz;

  if (hit > 0.0)
    {
      hit -= (1.0 / ((float)FPS * 0.5));

      if (hit <= 0.0) hit = 0.0;
    }

  // collision check
  {
    float w, v0, v1;
    w = BOX_W / 2.0;
    v0 = bx - SPHERE_R;
    v1 = bx + SPHERE_R;

    if (v0 <= -w || v1 >= w)
      {
        dx *= -1;
        hit = 1.0;
        hit_dir = 0;
        cx = (v0 <= -w) ? v0 : v1;
        cy = by;
        cz = bz;
      }

    v0 = by - SPHERE_R;
    v1 = by + SPHERE_R;

    if (v0 <= -w || v1 >= w)
      {
        dy *= -1;
        hit = 1.0;
        hit_dir = 1;
        cx = bx;
        cy = (v0 <= -w) ? v0 : v1;
        cz = bz;
      }

    v0 = bz - SPHERE_R;
    v1 = bz + SPHERE_R;

    if (v0 <= -w || v1 >= w)
      {
        dz *= -1;
        hit = 1.0;
        hit_dir = 2;
        cx = bx;
        cy = by;
        cz = (v0 <= -w) ? v0 : v1;
      }
  }
}

// main loop
void Render(HDC hDC)
{
  Update();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set light
  glDisable(GL_LIGHTING);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glLightfv(GL_LIGHT0, GL_POSITION, light0pos);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0dif);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light0spe);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0amb);

  glLoadIdentity();

  // rotate camera position
  {
    double r = (BOX_W / 2.0) + 15.0;
    GLdouble x = r * cos(DEG2RAD(ang));
    GLdouble y = r * 0.2 + 10 * sin(DEG2RAD(ang * 2));
    GLdouble z = r * sin(DEG2RAD(ang));

    //camera xyz, the xyz to look at, and the up vector (+y is up)
    gluLookAt(x, y, z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  }

  glScalef(1.0, 1.0, 1.0);

  // light disable
  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHTING);

  // set color
  if (hit <= 0.0)
    glColor3f(0.0, 0.0, 0.8);
  else
    glColor3f(hit, hit, 0.8 + 0.2 * hit);

  // draw grid
  DrawGrid(BOX_W);

  // draw cube
  // glutWireCube(BOX_W)
  DrawWireCube(BOX_W);

  // draw hit effect
  if (hit > 0.0)
    {
      glColor3f(0.0, hit, hit);
      DrawCircle(cx, cy, cz, 0.1 + 2.0 - 2.0 * hit, hit_dir);
    }

  // light enable
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glPushMatrix();

  // translate ball
  glTranslatef(bx, by, bz);

  // set color
  if (hit <= 0.0)
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, green);
  else
    {
      GLfloat col[] = {0.0, 0.0, 0.0, 1.0};
      col[0] = hit;
      col[1] = 1.0 - hit;
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
    }

  // draw sphere
  // gluSphere(qobj, radius, slices, stacks);
  // gluSphere(qobj, SPHERE_R, 16, 8);
  gluSphere(qobj, SPHERE_R, 32, 16);

  glPopMatrix();

  // glFlush();
  // glFinish();

  SwapBuffers(hDC);

  // calc FPS
  CountFps();
}


void SetupAnimation(int Width, int Height)
{
  //window resizing stuff
  glViewport(0, 0, (GLsizei) Width, (GLsizei) Height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (float)Width / (float)Height, 0.1, 100.0);
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();

  //background
  glClearColor(0.0, 0.0, 0.0, 0.0); //0.0s is black
  glClearDepth(1.0);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);

  // glShadeModel(GL_FLAT);
  glShadeModel(GL_SMOOTH);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);

  glDisable(GL_LIGHTING);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);

  InitWork(BOX_W, FPS);
}


// Initialize OpenGL
static bool InitGL(HWND hWnd, HDC &hDC, HGLRC &hRC)
{
  hDC = GetDC(hWnd);

#if 0
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd, sizeof pfd);
  pfd.nSize = sizeof pfd;
  pfd.nVersion = 1;

  //pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; //blaine's
  // pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;

  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
#else

  try
    {
      PIXELFORMATDESCRIPTOR pfd =
      {
        sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
        1,                     // version number
        // support window, OpenGL, double bufferd
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,         // RGBA type
        32,                    // color
        0, 0,                  // R
        0, 0,                  // G
        0, 0,                  // B
        0, 0,                  // A
        0,                     // accumulation buffer
        0, 0, 0, 0,            // accum bits ignored
        24,                    // depth
        8,                     // stencil buffer
        0,                     // auxiliary buffer
        PFD_MAIN_PLANE,        // main layer
        0,                     // reserved
        0, 0, 0                // layermask, visiblemask, damagemask
      };

      int format = ChoosePixelFormat(hDC, &pfd);

      if (format == 0) throw "";

      if (!SetPixelFormat(hDC, format, &pfd)) throw "";

      hRC = wglCreateContext(hDC);

      if (!hRC) throw "";

    }
  catch (...)
    {
      ReleaseDC(hWnd, hDC);
      return false;
    }

#endif

  wglMakeCurrent(hDC, hRC);

  qobj = gluNewQuadric();
  return true;
}


// Shut down OpenGL
static void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
  gluDeleteQuadric(qobj);

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hRC);
  ReleaseDC(hWnd, hDC);
}


void CleanupAnimation()
{
  // didn't create any objects, so no need to clean them up
}


/////////   REGISTRY ACCESS FUNCTIONS     ///////////

void GetConfig()
{
  if (0)
    {
      // get configuration from registry

      HKEY key;
      //DWORD lpdw;

      if (RegOpenKeyEx(HKEY_CURRENT_USER,
                       "Software\\GreenSquare", //lpctstr
                       0,                       //reserved
                       KEY_QUERY_VALUE,
                       &key) == ERROR_SUCCESS)
        {
          // DWORD dsize = sizeof(bTumble);
          DWORD dwtype =  0;

          // RegQueryValueEx(key, "Tumble", NULL, &dwtype, (BYTE*)&bTumble, &dsize);

          //Finished with key
          RegCloseKey(key);
        }
      else //key isn't there yet--set defaults
        {
          // bTumble = true;
        }
    }
  else
    {
      // bTumble = true;
    }
}

void WriteConfig(HWND hDlg)
{
  if (0)
    {
      HKEY key;
      DWORD lpdw;

      const char* null_str = "";
      LPSTR str = const_cast<LPSTR>(null_str);

      if (RegCreateKeyEx(HKEY_CURRENT_USER,
                         "Software\\GreenSquare", // lpctstr
                         0,                       // reserved
                         str,                     // ptr to null-term string specifying the object type of this key
                         REG_OPTION_NON_VOLATILE,
                         KEY_WRITE,
                         NULL,
                         &key,
                         &lpdw) == ERROR_SUCCESS)

        {
          // RegSetValueEx(key, "Tumble", 0, REG_DWORD, (BYTE*)&bTumble, sizeof(bTumble));

          //Finished with keys
          RegCloseKey(key);
        }
    }
}

//////////////////////////////////////////////////
////   INFRASTRUCTURE -- THE THREE FUNCTIONS   ///
//////////////////////////////////////////////////

// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam)
{
  static HDC hDC;
  static HGLRC hRC;
  static RECT rect;

  switch (message)
    {
      case WM_CREATE:
        // get window dimensions
        GetClientRect(hWnd, &rect);
        Width = rect.right;
        Height = rect.bottom;

        // get configuration from registry
        GetConfig();

        // setup OpenGL, then animation
        if (!InitGL(hWnd, hDC, hRC)) break;

        SetupAnimation(Width, Height);

        // set timer to tick every 10 ms
        SetTimer(hWnd, TIMER, (int)(1000 / FPS), NULL);

        InitCountFps();
        return 0;

      case WM_TIMER:
        Render(hDC);       // animate!
        return 0;

      case WM_DESTROY:
        CloseCountFps();
        KillTimer(hWnd, TIMER);
        CleanupAnimation();
        CloseGL(hWnd, hDC, hRC);
        return 0;
    }

  return DefScreenSaverProc(hWnd, message, wParam, lParam);
}

BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message,
                                       WPARAM wParam, LPARAM lParam)
{
  //InitCommonControls();
  //would need this for slider bars or other common controls

  // HWND aCheck;

  switch (message)
    {
      case WM_INITDIALOG:
        LoadString(hMainInstance, IDS_DESCRIPTION, szAppName, 40);

        GetConfig();

        // aCheck = GetDlgItem(hDlg, IDC_TUMBLE);
        // SendMessage(aCheck, BM_SETCHECK, (bTumble) ? BST_CHECKED : BST_UNCHECKED, 0);
        return TRUE;

      case WM_COMMAND:
        switch (LOWORD(wParam))
          {
            // case IDC_TUMBLE:
            //   bTumble = (IsDlgButtonChecked(hDlg, IDC_TUMBLE) == BST_CHECKED);
            //   return TRUE;

            //cases for other controls would go here

            case IDOK:
              WriteConfig(hDlg);  // get info from controls
              EndDialog(hDlg, LOWORD(wParam) == IDOK);
              return TRUE;

            case IDCANCEL:
              EndDialog(hDlg, LOWORD(wParam) == IDOK);
              return TRUE;
          }

        return FALSE;
    }  //end command switch

  return FALSE;
}

// needed for SCRNSAVE.LIB (or libscrnsave.a)
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
  return TRUE;
}
