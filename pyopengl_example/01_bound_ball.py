#!python
# -*- mode: python; Encoding: utf-8; coding: utf-8 -*-
# Last updated: <2022/09/18 21:40:32 +0900>
"""
GL bound ball by mieki256.

License: CC0 / Public Domain.

ESC, q : Exit
f : Display FPS

* Windows10 x64 21H2 + Python 3.9.13 + PyOpenGL 3.1.6
* Windows10 x64 21H2 + Python 2.7.18 + PyOpenGL 3.1.0
"""

import sys
import math
import time
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

# SCRW, SCRH = 1600, 900
SCRW, SCRH = 1280, 720
# SCRW, SCRH = 512, 288
# SCRW, SCRH = 640, 480

FPS = 60

BOX_W = 20
SPHERE_R = 0.5

scr_w, scr_h = SCRW, SCRH

ang = 0.0
ang_d = 0.0
bx, by, bz = 0.0, 0.0, 0.0
dx, dy, dz = 0.0, 0.0, 0.0
cx, cy, cz = 0.0, 0.0, 0.0
hit = 0.0
hit_dir = 0

fps_bar_enable = False
rec_time = 0
count_fps = 0
count_frame = 0

window = 0


def init_count_fps():
    global rec_time, count_fps, count_frame
    rec_time = time.time()
    count_fps = 0
    count_frame = 0


def calc_fps():
    global rec_time, count_fps, count_frame

    count_frame += 1
    t = time.time() - rec_time
    if t >= 1.0:
        rec_time += 1.0
        count_fps = count_frame
        count_frame = 0
    elif t < 0.0:
        rec_time = time.time()
        count_fps = 0
        count_frame = 0


def init_work():
    global ang, ang_d, bx, by, bz, dx, dy, dz, cx, cy, cz, hit, hit_dir
    ang = 0.0
    ang_d = 360.0 / (FPS * 30)
    bx, by, bz = 0.0, 0.0, 0.0
    dx = (float(BOX_W) / float(FPS)) * 0.5
    dy = dx * 0.3
    dz = dx * 0.7
    cx, cy, cz = 0.0, 0.0, 0.0
    hit = 0.0
    hit_dir = 0

    init_count_fps()


def draw_string(x, y, str):
    # light disable
    glDisable(GL_LIGHT0)
    glDisable(GL_LIGHTING)

    glColor3f(1.0, 1.0, 1.0)
    glWindowPos2f(x, y)
    for c in str:
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, ord(c))


def draw_wire_cube(width):
    global count_fps

    w = float(width) / 2.0
    pos = [
        [-1, -1, -1],
        [-1, 1, -1],
        [1, 1, -1],
        [1, -1, -1],
        [-1, -1, 1],
        [-1, 1, 1],
        [1, 1, 1],
        [1, -1, 1],
    ]
    idxs = [
        [0, 1],
        [1, 2],
        [2, 3],
        [3, 0],
        [4, 5],
        [5, 6],
        [6, 7],
        [7, 4],
        [0, 4],
        [1, 5],
        [2, 6],
        [3, 7],
    ]

    for idx in idxs:
        i0, i1 = idx
        x0, y0, z0 = pos[i0]
        x1, y1, z1 = pos[i1]
        glBegin(GL_LINES)
        glVertex3f(w * x0, w * y0, w * z0)
        glVertex3f(w * x1, w * y1, w * z1)
        glEnd()

    if fps_bar_enable:
        ww = width * float(count_fps) / FPS
        glColor3f(1.0, 0.0, 0.0)
        glBegin(GL_LINES)
        glVertex3f(-w, -w, w * 0.98)
        glVertex3f(ww - w, -w, w * 0.98)
        glEnd()


def draw_grid(width):
    w = float(width) / 2.0
    x = -w
    y = -w
    d = float(width) / 10
    while x <= (width / 2):
        glBegin(GL_LINES)
        glVertex3f(x, y, -w)
        glVertex3f(x, y, w)
        glEnd()

        glBegin(GL_LINES)
        glVertex3f(-w, y, x)
        glVertex3f(w, y, x)
        glEnd()

        x += d


def draw_circle(bx, by, bz, r, hit_dir):
    glPushMatrix()

    glTranslatef(bx, by, bz)

    if hit_dir == 0:
        glRotatef(90.0, 0.0, 1.0, 0.0)
    elif hit_dir == 1:
        glRotatef(90.0, 1.0, 0.0, 0.0)

    y = -r
    d = float(r) / 5.0
    while y <= r:
        x = math.sqrt(r * r - (y * y))
        glBegin(GL_LINES)
        glVertex3f(-x, y, 0.0)
        glVertex3f(+x, y, 0.0)
        glEnd()
        glBegin(GL_LINES)
        glVertex3f(y, -x, 0.0)
        glVertex3f(y, +x, 0.0)
        glEnd()
        y += d

    glPopMatrix()


def draw_func():
    global bx, by, bz, hit, hit_dir, cx, cy, cz, count_fps, scr_h

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    if fps_bar_enable:
        draw_string(8, scr_h - (24 + 8), "FPS: %d" % count_fps)

    # set light
    light0pos = [BOX_W * 0.25, BOX_W * 1.0, BOX_W * 0.5, 0]
    light0def = [1, 1, 1, 1]
    light0spe = [1, 1, 1, 1]
    light0amb = [0.5, 0.5, 1.0, 1.0]
    glDisable(GL_LIGHTING)
    glEnable(GL_LIGHTING)
    glEnable(GL_NORMALIZE)
    glLightfv(GL_LIGHT0, GL_POSITION, light0pos)
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0def)
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0spe)
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0amb)

    glLoadIdentity()

    # rotate camera position
    r = (BOX_W / 2) + 15
    x = r * math.cos(math.radians(ang))
    y = r * 0.2 + 10 * math.sin(math.radians(ang * 2))
    z = r * math.sin(math.radians(ang))
    tx, ty, tz = 0, 0, 0
    gluLookAt(x, y, z, tx, ty, tz, 0, 1, 0)

    glScalef(1.0, 1.0, 1.0)

    # light disable
    glDisable(GL_LIGHT0)
    glDisable(GL_LIGHTING)

    # set color
    if hit == 0.0:
        glColor3f(0.0, 0.0, 0.8)
    else:
        glColor3f(hit, hit, 0.8 + 0.2 * hit)

    # draw grid
    draw_grid(BOX_W)

    # draw cube
    # glutWireCube(BOX_W)
    draw_wire_cube(BOX_W)

    if hit > 0.0:
        glColor3f(0.0, hit, hit)
        draw_circle(cx, cy, cz, 0.1 + 2.0 - 2.0 * hit, hit_dir)

    # light enable
    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)

    glPushMatrix()

    glTranslatef(bx, by, bz)

    # set color
    if hit == 0.0:
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, [0.0, 1.0, 0.0, 1.0])
    else:
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, [hit, 1.0 - hit, 0.0, 1.0])
        hit -= 0.03
        if hit <= 0.0:
            hit = 0.0

    # draw sphere
    quadric = gluNewQuadric()
    radius = SPHERE_R
    slices = 32
    stacks = 16
    gluSphere(quadric, radius, slices, stacks)

    glPopMatrix()

    glutSwapBuffers()

    calc_fps()


def init_GL(w, h):
    global window

    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)
    # glutInitWindowPosition(0, 0)
    glutInitWindowSize(w, h)
    window = glutCreateWindow("GL Bound Ball by PyOpenGL")

    glClearColor(0.0, 0.0, 0.0, 0.0)
    glClearDepth(1.0)

    glDepthFunc(GL_LESS)
    glEnable(GL_DEPTH_TEST)

    glShadeModel(GL_SMOOTH)
    # glShadeModel(GL_FLAT)

    glDisable(GL_LIGHTING)
    glEnable(GL_LIGHTING)
    glEnable(GL_NORMALIZE)

    # glEnable(GL_CULL_FACE)
    # glCullFace(GL_BACK)

    init_view_and_pers(w, h)


def init_view_and_pers(w, h):
    """Init viewport and Perspective."""
    global scr_w, scr_h
    scr_w, scr_h = w, h
    glViewport(0, 0, w, h)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(60.0, float(w) / float(h), 0.1, 100.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()


def reshape(width, height):
    """Window resize callback."""
    init_view_and_pers(width, height)


def on_timer(value):
    global ang, ang_d, bx, by, bz, dx, dy, dz, cx, cy, cz, hit, hit_dir

    # update angle and position
    ang += ang_d
    if ang >= 360.0:
        ang -= 360.0

    bx += dx
    by += dy
    bz += dz

    hit -= 0.03
    if hit < 0.0:
        hit = 0.0

    # collision check
    w = BOX_W / 2.0
    if bx - SPHERE_R <= -w or bx + SPHERE_R >= w:
        dx *= -1
        hit = 1.0
        hit_dir = 0
        cx = bx + (SPHERE_R * (-1 if (bx - SPHERE_R <= -w) else 1))
        cy = by
        cz = bz
    if by - SPHERE_R <= -w or by + SPHERE_R >= w:
        dy *= -1
        hit = 1.0
        hit_dir = 1
        cx = bx
        cy = by + (SPHERE_R * (-1 if (by - SPHERE_R <= -w) else 1))
        cz = bz
    if bz - SPHERE_R <= -w or bz + SPHERE_R >= w:
        dz *= -1
        hit = 1.0
        hit_dir = 2
        cx = bx
        cy = by
        cz = bz + (SPHERE_R * (-1 if (bz - SPHERE_R <= -w) else 1))

    glutPostRedisplay()
    glutTimerFunc(int(1000 / FPS), on_timer, 0)


def key_func(key, x, y):
    global window, fps_bar_enable

    ESCAPE = b"\x1b"
    print("Push key,", key, x, y)
    if key == ESCAPE or key == b"q":
        print("Exit")
        if glutLeaveMainLoop:
            glutLeaveMainLoop()
        else:
            sys.exit()
    elif key == b"f":
        fps_bar_enable = not fps_bar_enable


def main():
    # main
    init_GL(SCRW, SCRH)
    init_work()
    glutKeyboardFunc(key_func)
    glutDisplayFunc(draw_func)
    glutReshapeFunc(reshape)
    glutTimerFunc(int(1000 / FPS), on_timer, 0)
    glutMainLoop()


if __name__ == "__main__":
    main()
