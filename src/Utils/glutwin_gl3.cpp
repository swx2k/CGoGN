/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* version 0.1                                                                  *
* Copyright (C) 2009, IGG Team, LSIIT, University of Strasbourg                *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: https://iggservis.u-strasbg.fr/CGoGN/                              *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

//#ifndef MAC_OSX

#include <stdlib.h>

#include "Utils/glutwingl3.h"
#include <math.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <iostream>

#include "gzstream.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_projection.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/inverse_transpose.hpp"

namespace CGoGN
{

namespace Utils
{

SimpleGlutWinGL3* SimpleGlutWinGL3::instance=NULL;
float SimpleGlutWinGL3::scalefactor = 1.0f;
float SimpleGlutWinGL3::trans_x=0.;
float SimpleGlutWinGL3::trans_y=0.;
float SimpleGlutWinGL3::trans_z=0.;

float SimpleGlutWinGL3::foc=2.0f;

float SimpleGlutWinGL3::curquat[4]={0.0f,0.0f,0.0f,0.0f};
float SimpleGlutWinGL3::lastquat[4]={0.0f,0.0f,0.0f,0.0f};
int SimpleGlutWinGL3::spinning = 0;
int SimpleGlutWinGL3::moving = 0;
int SimpleGlutWinGL3::newModel = 1;
int SimpleGlutWinGL3::translating=0;
int SimpleGlutWinGL3::scaling=0;
int SimpleGlutWinGL3::beginx=0;
int SimpleGlutWinGL3::beginy=0;
int SimpleGlutWinGL3::W=0;
int SimpleGlutWinGL3::H=0;
bool SimpleGlutWinGL3::m_noMouse=false;

float SimpleGlutWinGL3::m_obj_sc;
glm::vec3 SimpleGlutWinGL3::m_obj_pos;

Utils::GLSLShader* SimpleGlutWinGL3::m_current_shaders=NULL;

glm::mat4 SimpleGlutWinGL3::m_projection_matrix;

glm::mat4 SimpleGlutWinGL3::m_modelView_matrix;

std::stack<glm::mat4> SimpleGlutWinGL3::m_stack_mv;

// light
//glm::vec4 SimpleGlutWinGL3::m_materialAmbient;
//glm::vec4 SimpleGlutWinGL3::m_materialDiffuse;
//glm::vec4 SimpleGlutWinGL3::m_materialSpecular;
//float SimpleGlutWinGL3::m_materialShininess;
//
//glm::vec4 SimpleGlutWinGL3::m_lightPosition;
//glm::vec4 SimpleGlutWinGL3::m_lightDiffuse;
//glm::vec4 SimpleGlutWinGL3::m_lightSpecular;
//glm::vec4 SimpleGlutWinGL3::m_lightAmbient;


SimpleGlutWinGL3::SimpleGlutWinGL3(int* argc, char **argv, int winX, int winY)
{
	instance = this;
	spinning = 0;
	moving = 0;
	newModel = 1;
	scalefactor = 1.0f;
	trans_x=0.;
	trans_y=0.;
	trans_z=-50.0f;

	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(argv[0]);
//	glutInitContextVersion (3, 2);
//	glutInitContextFlags (GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutReshapeWindow(winX,winY);
  	W=winX; H=winY;
	trackball(curquat, 0.0f, 0.0f, 0.0f, 0.0f);
	glutDisplayFunc(redraw);
	glutReshapeFunc(reshape);
	glutVisibilityFunc(vis);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);

	GLint MajorVersionContext = 0;
	GLint MinorVersionContext = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &MajorVersionContext);
	glGetIntegerv(GL_MINOR_VERSION, &MinorVersionContext);

	std::cout << "OpenGL v"<<MajorVersionContext<<"."<<MinorVersionContext<<std::endl;
	std::cout << glGetString(GL_VERSION)<<std::endl;

	int nbatt;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nbatt);
	std::cout << "Attribute max: "<< nbatt << std::endl;

	shaderOk = Utils::GLSLShader::init();




//	glViewport(0, 0, W, H);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	glFrustum(-1.0f,1.0f,-1.0f*H/W,1.0f*H/W,foc,500.0f);
//	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.1,0.1,0.3,0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glEnable(GL_LIGHTING);

	//Store context infos

#ifdef WIN32
	m_drawable = wglGetCurrentDC();
	m_context = wglGetCurrentContext();
#else
	#ifdef MAC_OSX
		m_context = CGLGetCurrentContext();
	#else
		m_dpy = glXGetCurrentDisplay();
		m_drawable = glXGetCurrentDrawable();
		m_context = glXGetCurrentContext();
	#endif
#endif
	// Call other initialization (possibly overloaded in instances)
	instance->init();
}



void SimpleGlutWinGL3::oglRotate(float angle, float x, float y, float z)
{
	m_modelView_matrix = glm::rotate(m_modelView_matrix, angle, glm::vec3(x,y,z));
}

void SimpleGlutWinGL3::oglTranslate(float tx, float ty, float tz)
{
	m_modelView_matrix = glm::translate(m_modelView_matrix, glm::vec3(tx,ty,tz));
}

void SimpleGlutWinGL3::oglScale(float sx, float sy, float sz)
{
	m_modelView_matrix = glm::scale(m_modelView_matrix, glm::vec3(sx,sy,sz));
}

void SimpleGlutWinGL3::oglPushModelViewMatrix()
{
	m_stack_mv.push(m_modelView_matrix);
}


bool SimpleGlutWinGL3::oglPopModelViewMatrix()
{
	if (m_stack_mv.empty())
		return false;
	m_modelView_matrix = m_stack_mv.top();
	m_stack_mv.pop();
	return true;
}


//void SimpleGlutWinGL3::setParameters(int spec, float* val)
//{
//	switch(spec)
//	{
//	case DIFFUSE_MATERIAL:
//		m_materialDiffuse = glm::vec4(val[0],val[1],val[2],val[3]);
//		break;
//	case SPECULAR_MATERIAL:
//		m_materialSpecular = glm::vec4(val[0],val[1],val[2],val[3]);
//		break;
//	case AMBIENT_MATERIAL:
//		m_materialAmbient = glm::vec4(val[0],val[1],val[2],val[3]);
//		break;
//	case SHININESS:
//		m_materialShininess = *val;
//		break;
//	case DIFFUSE_LIGHT:
//		m_lightDiffuse = glm::vec4(val[0],val[1],val[2],val[3]);
//		break;
//	case SPECULAR_LIGHT:
//		m_lightSpecular = glm::vec4(val[0],val[1],val[2],val[3]);
//		break;
//	case AMBIENT_LIGHT:
//		m_lightAmbient = glm::vec4(val[0],val[1],val[2],val[3]);
//		break;
//	case LIGHT_POSITION:
//		m_lightPosition = glm::vec4(val[0],val[1],val[2],val[3]);
//		break;
//	default:
//		std::cerr << "setParameters: wrong parameter"<< std::endl;
//		break;
//	}
//}

void SimpleGlutWinGL3::setParamObject(float width, float* pos)
{
	m_obj_sc = 50.0f / width;
	m_obj_pos = glm::vec3(-pos[0],-pos[1],-pos[2]);
}


void SimpleGlutWinGL3::setCurrentShader(Utils::GLSLShader* sh)
{
	m_current_shaders = sh;
	m_current_shaders->bind();
	setModelViewProjectionMatrix(m_current_shaders);
	glutPostRedisplay();
}


void SimpleGlutWinGL3::releaseContext()
{
#ifdef WIN32
	wglMakeCurrent(NULL,NULL);
#else
	#ifdef MAC_OSX
		CGLSetCurrentContext(NULL);
	#else
		glXMakeCurrent(m_dpy,None,NULL);
	#endif
#endif
}

void SimpleGlutWinGL3::useContext()
{
#ifdef WIN32
	wglMakeCurrent(m_drawable, m_context);
#else
	#ifdef MAC_OSX
		CGLSetCurrentContext(m_context);
	#else
		glXMakeCurrent(m_dpy, m_drawable, m_context);
	#endif
#endif

}





void SimpleGlutWinGL3::recalcModelView(void)
{
	glm::mat4 m;

	oglPopModelViewMatrix();
	oglPushModelViewMatrix();

	oglTranslate(trans_x,trans_y,trans_z+foc);

	build_rotmatrixgl3(m, curquat);

	m_modelView_matrix *= m;

	float sc = getScale();
	oglScale(sc,sc,sc);


	oglScale(m_obj_sc,m_obj_sc,m_obj_sc);
	oglTranslate(m_obj_pos[0], m_obj_pos[1], m_obj_pos[2]);


	newModel = 0;

	setModelViewProjectionMatrix(m_current_shaders);
}

void SimpleGlutWinGL3::redraw(void)
{
	if (newModel)
	{
	    recalcModelView();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	instance->myRedraw();

	glutSwapBuffers();
}

void SimpleGlutWinGL3::reshape(int w, int h)
{
	if (w>0) W = w;
	if (h>0) H = h;

	glViewport(0,0,W,H);
	m_projection_matrix = glm::frustum(-1.0f,1.0f,-1.0f*H/W,1.0f*H/W,foc,500.0f);
	recalcModelView();
}

void SimpleGlutWinGL3::animate(void)
{
	add_quats(lastquat, curquat, curquat);
	newModel = 1;
	glutPostRedisplay();
}

void SimpleGlutWinGL3::motion(int x, int y)
{
	if (!m_noMouse)
	{
		if (scaling || translating)
		{
			if (scaling)
			{
				scalefactor = scalefactor * (1.0f + (((float) (beginy - y)) / H));
			}
			else if (translating)
			{
				trans_x += 0.01f*(x - beginx);
				trans_y += 0.01f*(beginy - y);
			}
			beginx = x;
			beginy = y;
			newModel = 1;
			glutPostRedisplay();
			return;
		}

		if (moving)
		{
			trackball(lastquat, (2.0f * beginx - W) / W,(H - 2.0f * beginy) / H,
								(2.0f * x - W) / W,(H - 2.0f * y) / H );
			beginx = x;
			beginy = y;
			spinning = 1;
			//    glutIdleFunc(animate);
			animate();
		}
	}

	instance->myMouseMotion(x, y);
}

void SimpleGlutWinGL3::vis(int visible)
{
	if (spinning)
	{
		glutIdleFunc(NULL);
	}

}

void SimpleGlutWinGL3::mouse(int button, int state, int x, int y)
{
	if (!m_noMouse)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			spinning = 0;
			glutIdleFunc(NULL);
			moving = 1;
			beginx = x;
			beginy = y;
			scaling = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
			translating = (glutGetModifiers() & GLUT_ACTIVE_CTRL) != 0;
		}
		else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		{
			moving = 0;
		}
	}
	instance->myMouseClick(button, state, x, y);
}

void SimpleGlutWinGL3::keyboard(unsigned char keycode, int x, int y)
{
	if (keycode == 27)
	{
		instance->exitCallback();
		exit(0);
	}
	instance->myKeyboard(keycode, x, y);
}

int SimpleGlutWinGL3::mainLoop()
{
	glutMainLoop();
	return 0;
}





void SimpleGlutWinGL3::setModelViewProjectionMatrix(Utils::GLSLShader* current)
{
	glm::mat4 PMV = m_projection_matrix*m_modelView_matrix;
	current->setuniformf<16>("ModelViewProjectionMatrix", &PMV[0][0]);

	current->setuniformf<16>("ModelViewMatrix", &m_modelView_matrix[0][0]);

	glm::mat4 normalMatrix = glm::gtx::inverse_transpose::inverseTranspose(m_modelView_matrix);
	current->setuniformf<16>("NormalMatrix", &normalMatrix[0][0]);
}





void SimpleGlutWinGL3::capturePNG(const char* filename)
{
	ILuint imgName;
	ilGenImages(1,&imgName);
	ilBindImage(imgName);
	ilutGLScreen();

	// save image
	ilEnable(IL_FILE_OVERWRITE);
	ilSaveImage((ILstring) filename);
	ilDeleteImages(1,&imgName);
}

GLfloat SimpleGlutWinGL3::getOrthoScreenRay(int x, int y, Geom::Vec3f& rayA, Geom::Vec3f& rayB)
{
//	GLdouble model[16];
//	GLdouble proj[16];
//	GLint view[4];
//
//	// get Z from depth buffer
//	GLfloat prof;
//	glReadPixels(x,H-y,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&prof);
//
//	// get matrices
////	glGetDoublev(GL_MODELVIEW_MATRIX,model);
////	glGetDoublev(GL_PROJECTION_MATRIX,proj);
////	glGetIntegerv(GL_VIEWPORT,view);
//
//	// unproject x,y,0
//	GLdouble xx, yy, zz;
//	gluUnProject(x, H-y, 0.0f, model, proj, view, &xx, &yy, &zz);
//	rayA[0] = float(xx);
//	rayA[1] = float(yy);
//	rayA[2] = float(zz);
//
//	// unprojet x,y,z
//	gluUnProject(x, H-y, prof, model, proj, view, &xx, &yy, &zz);
//	rayB[0] = float(xx);
//	rayB[1] = float(yy);
//	rayB[2] = float(zz);
//	return prof;
	return 0.0;
}

//void SimpleGlutWinGL3::printString2D(int x, int y, const std::string& str)
//{
//	// PASSAGE EN 2D
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	glLoadIdentity();
//	glOrtho(0, W, H,0, -1, 1);
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//	glLoadIdentity();
//	glDisable(GL_DEPTH_TEST);
//	glDisable(GL_LIGHTING);
//
//	// AFFICHAGE EN 2D
//	int x1 = x;
//	for (unsigned i=0; i<str.length(); ++i)
//	{
//		char c = str[i];
//		if (c=='\n')
//		{
//			x1 = x;
//			y += 14;
//		}
//		glRasterPos2i(x1,y);
//		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
//		x1 += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12,c);
//	}
//
//	// RETOUR EN 3D
//	glMatrixMode(GL_PROJECTION);
//	glPopMatrix();
//	glMatrixMode(GL_MODELVIEW);
//	glPopMatrix();
//	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_LIGHTING);
//}


} // namespace Utils
} // namespace CGoGN

//#endif