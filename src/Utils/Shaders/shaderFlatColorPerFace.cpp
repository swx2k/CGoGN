/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* version 0.1                                                                  *
* Copyright (C) 2009-2011, IGG Team, LSIIT, University of Strasbourg           *
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
* Web site: http://cgogn.u-strasbg.fr/                                         *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#include <string.h>
#include <GL/glew.h>
#include "Utils/Shaders/shaderFlatColorPerFace.h"

namespace CGoGN
{

namespace Utils
{
#include "shaderFlatColorPerFace.vert"
#include "shaderFlatColorPerFace.frag"
#include "shaderFlatColorPerFace.geom"


ShaderFlatColorPerFace::ShaderFlatColorPerFace()
{
	m_nameVS = "shaderFlatColorPerFace_vs";
	m_nameFS = "shaderFlatColorPerFace_fs";
	m_nameGS = "shaderFlatColorPerFace_gs";

	std::string glxvert(*GLSLShader::DEFINES_GL);
	glxvert.append(vertexShaderText);

	std::string glxgeom = GLSLShader::defines_Geom("triangles", "triangle_strip", 3);
	glxgeom.append(geometryShaderText);

	std::string glxfrag(*GLSLShader::DEFINES_GL);
	glxfrag.append(fragmentShaderText);

	loadShadersFromMemory(glxvert.c_str(), glxfrag.c_str(), glxgeom.c_str(), GL_TRIANGLES, GL_TRIANGLE_STRIP,3);

	getLocations();

	//Default values
	m_explode = 1.0f;
	m_ambiant = Geom::Vec4f(0.05f, 0.05f, 0.1f, 0.0f);
	m_light_pos = Geom::Vec3f(10.0f, 10.0f, 1000.0f);

	setParams(m_explode, m_ambiant, m_light_pos);
}

void ShaderFlatColorPerFace::getLocations()
{
	m_unif_explode  = glGetUniformLocation(program_handler(),"explode");
	m_unif_ambiant  = glGetUniformLocation(program_handler(),"ambient");
	m_unif_lightPos = glGetUniformLocation(program_handler(),"lightPosition");
}

void ShaderFlatColorPerFace::setAttributePosition(VBO* vbo)
{
	m_vboPos = vbo;
	bindVA_VBO("VertexPosition", vbo);
}
void ShaderFlatColorPerFace::setAttributeColor(VBO* vbo)
{
	m_vboColor = vbo;
	bindVA_VBO("VertexColor", vbo);
}



void ShaderFlatColorPerFace::setParams(float expl, const Geom::Vec4f& ambiant, const Geom::Vec3f& lightPos)
{
	m_explode = expl;
	m_ambiant = ambiant;
	m_light_pos = lightPos;

	bind();

	glUniform1f(m_unif_explode, expl);
	glUniform4fv(m_unif_ambiant, 1, ambiant.data());
	glUniform3fv(m_unif_lightPos, 1, lightPos.data());

	unbind(); // ??
}

void ShaderFlatColorPerFace::setExplode(float explode)
{
	m_explode = explode;
	bind();
	glUniform1f(m_unif_explode, explode);
}

void ShaderFlatColorPerFace::setAmbiant(const Geom::Vec4f& ambiant)
{
	m_ambiant = ambiant;
	bind();
	glUniform4fv(m_unif_ambiant,1, ambiant.data());
}


void ShaderFlatColorPerFace::setLightPosition(const Geom::Vec3f& lp)
{
	m_light_pos = lp;
	bind();
	glUniform3fv(m_unif_lightPos,1,lp.data());
}

void ShaderFlatColorPerFace::restoreUniformsAttribs()
{
	m_unif_explode   = glGetUniformLocation(program_handler(),"explode");
	m_unif_ambiant   = glGetUniformLocation(program_handler(),"ambient");
	m_unif_lightPos =  glGetUniformLocation(program_handler(),"lightPosition");

	bind();
	glUniform1f (m_unif_explode, m_explode);
	glUniform4fv(m_unif_ambiant,  1, m_ambiant.data());
	glUniform3fv(m_unif_lightPos, 1, m_light_pos.data());

	bindVA_VBO("VertexPosition", m_vboPos);
	bindVA_VBO("VertexColor", m_vboPos);
	unbind();
}

} // namespace Utils

} // namespace CGoGN