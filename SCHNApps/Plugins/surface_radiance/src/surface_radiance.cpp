#include "surface_radiance.h"

#include "meshTableSurfaceRadiance.h"
#include "halfEdgeSelectorRadiance.h"
#include "edgeSelectorRadiance.h"

#include "mapHandler.h"
#include "camera.h"

#include <QFileDialog>
#include <QFileInfo>

namespace CGoGN
{

namespace SCHNApps
{

bool Surface_Radiance_Plugin::enable()
{
	//	magic line that init static variables of GenericMap in the plugins
	GenericMap::copyAllStatics(m_schnapps->getStaticPointers());

	m_dockTab = new Surface_Radiance_DockTab(m_schnapps, this);
	m_schnapps->addPluginDockTab(this, m_dockTab, "Surface_Radiance");

	connect(m_schnapps, SIGNAL(selectedViewChanged(View*, View*)), this, SLOT(selectedViewChanged(View*, View*)));
	connect(m_schnapps, SIGNAL(selectedMapChanged(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selectedMapChanged(MapHandlerGen*, MapHandlerGen*)));
	connect(m_schnapps, SIGNAL(mapAdded(MapHandlerGen*)), this, SLOT(mapAdded(MapHandlerGen*)));
	connect(m_schnapps, SIGNAL(mapRemoved(MapHandlerGen*)), this, SLOT(mapRemoved(MapHandlerGen*)));

	foreach(MapHandlerGen* map, m_schnapps->getMapSet().values())
		mapAdded(map);

	m_dockTab->updateMapParameters();

	m_importAction = new QAction("import", this);
	m_schnapps->addMenuAction(this, "Radiance;Import", m_importAction);
	connect(m_importAction, SIGNAL(triggered()), this, SLOT(importFromFileDialog()));

	return true;
}

void Surface_Radiance_Plugin::disable()
{
	disconnect(m_schnapps, SIGNAL(selectedViewChanged(View*, View*)), this, SLOT(selectedViewChanged(View*, View*)));
	disconnect(m_schnapps, SIGNAL(selectedMapChanged(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selectedMapChanged(MapHandlerGen*, MapHandlerGen*)));
	disconnect(m_schnapps, SIGNAL(mapAdded(MapHandlerGen*)), this, SLOT(mapAdded(MapHandlerGen*)));
	disconnect(m_schnapps, SIGNAL(mapRemoved(MapHandlerGen*)), this, SLOT(mapRemoved(MapHandlerGen*)));

	foreach(MapHandlerGen* map, m_schnapps->getMapSet().values())
		mapRemoved(map);
}

void Surface_Radiance_Plugin::drawMap(View* view, MapHandlerGen* map)
{
	const MapParameters& p = h_mapParameterSet[map];

	if(p.positionVBO && p.normalVBO)
	{
		p.radiancePerVertexShader->setAttributePosition(p.positionVBO);
		p.radiancePerVertexShader->setAttributeNormal(p.normalVBO);
		p.radiancePerVertexShader->setAttributeRadiance(p.paramVBO, p.radianceTexture, GL_TEXTURE1);

		qglviewer::Vec c = view->getCurrentCamera()->position();
		PFP2::VEC3 camera(c.x, c.y, c.z);
		p.radiancePerVertexShader->setCamera(Geom::Vec3f(camera[0], camera[1], camera[2])); // convert to Vec3f because PFP2 can hold Vec3d !

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		map->draw(p.radiancePerVertexShader, Algo::Render::GL2::TRIANGLES);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}





void Surface_Radiance_Plugin::selectedViewChanged(View *prev, View *cur)
{
	m_dockTab->updateMapParameters();
}

void Surface_Radiance_Plugin::selectedMapChanged(MapHandlerGen *prev, MapHandlerGen *cur)
{
	m_dockTab->updateMapParameters();
	if (cur == NULL)
		m_dockTab->setDisabled(true);
	else
		m_dockTab->setDisabled(false);
}

void Surface_Radiance_Plugin::mapAdded(MapHandlerGen* map)
{
	connect(map, SIGNAL(vboAdded(Utils::VBO*)), this, SLOT(vboAdded(Utils::VBO*)));
	connect(map, SIGNAL(vboRemoved(Utils::VBO*)), this, SLOT(vboRemoved(Utils::VBO*)));
	connect(map, SIGNAL(attributeModified(unsigned int, QString)), this, SLOT(attributeModified(unsigned int, QString)));
}

void Surface_Radiance_Plugin::mapRemoved(MapHandlerGen* map)
{
	disconnect(map, SIGNAL(vboAdded(Utils::VBO*)), this, SLOT(vboAdded(Utils::VBO*)));
	disconnect(map, SIGNAL(vboRemoved(Utils::VBO*)), this, SLOT(vboRemoved(Utils::VBO*)));
	disconnect(map, SIGNAL(attributeModified(unsigned int, QString)), this, SLOT(attributeModified(unsigned int, QString)));
}





void Surface_Radiance_Plugin::vboAdded(Utils::VBO *vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if(map == m_schnapps->getSelectedMap())
	{
		if(vbo->dataSize() == 3)
		{
			m_dockTab->addPositionVBO(QString::fromStdString(vbo->name()));
			m_dockTab->addNormalVBO(QString::fromStdString(vbo->name()));
		}
	}
}

void Surface_Radiance_Plugin::vboRemoved(Utils::VBO *vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if(map == m_schnapps->getSelectedMap())
	{
		if(vbo->dataSize() == 3)
		{
			m_dockTab->removePositionVBO(QString::fromStdString(vbo->name()));
			m_dockTab->removeNormalVBO(QString::fromStdString(vbo->name()));
		}
	}

	MapParameters& mapParam = h_mapParameterSet[map];
	if(mapParam.positionVBO == vbo)
	{
		mapParam.positionVBO = NULL;
	}
	if(mapParam.normalVBO == vbo)
	{
		mapParam.normalVBO = NULL;
	}
}

void Surface_Radiance_Plugin::attributeModified(unsigned int orbit, QString nameAttr)
{
	if(orbit == VERTEX)
	{

	}
}

void Surface_Radiance_Plugin::importFromFileDialog()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(m_schnapps, "Import surface with radiance", m_schnapps->getAppPath(), "Surface with radiance Files (*.ply)");
	QStringList::Iterator it = fileNames.begin();
	while(it != fileNames.end()) {
		importFromFile(*it);
		++it;
	}
}





void Surface_Radiance_Plugin::changePositionVBO(const QString& map, const QString& vbo)
{
	MapHandlerGen* m = m_schnapps->getMap(map);
	if(m)
	{
		Utils::VBO* vbuf = m->getVBO(vbo);
		h_mapParameterSet[m].positionVBO = vbuf;
		if(m->isSelectedMap())
			m_dockTab->updateMapParameters();
	}
}

void Surface_Radiance_Plugin::changeNormalVBO(const QString& map, const QString& vbo)
{
	MapHandlerGen* m = m_schnapps->getMap(map);
	if(m)
	{
		Utils::VBO* vbuf = m->getVBO(vbo);
		h_mapParameterSet[m].normalVBO = vbuf;
		if(m->isSelectedMap())
			m_dockTab->updateMapParameters();
	}
}

MapHandlerGen* Surface_Radiance_Plugin::importFromFile(const QString& fileName)
{
	QFileInfo fi(fileName);
	if(fi.exists())
	{
		MapHandlerGen* mhg = m_schnapps->addMap(fi.baseName(), 2);
		if(mhg)
		{
			MapHandler<PFP2>* mh = static_cast<MapHandler<PFP2>*>(mhg);
			PFP2::MAP* map = mh->getMap();

			MeshTablesSurface_Radiance importer(*map);
			if (!importer.importPLY<Utils::SphericalHarmonics<PFP2::REAL, PFP2::VEC3> >(fileName.toStdString()))
			{
				std::cout << "could not import " << fileName.toStdString() << std::endl;
				return NULL;
			}
			CGoGN::Algo::Surface::Import::importMesh<PFP2>(*map, importer);

			// get vertex position attribute
			VertexAttribute<PFP2::VEC3, PFP2::MAP> position = map->getAttribute<PFP2::VEC3, VERTEX, PFP2::MAP>("position") ;
			VertexAttribute<PFP2::VEC3, PFP2::MAP> normal = map->getAttribute<PFP2::VEC3, VERTEX, PFP2::MAP>("normal");
			mh->registerAttribute(position);
			mh->registerAttribute(normal);

			MapParameters& mapParams = h_mapParameterSet[mhg];

			mapParams.nbVertices = Algo::Topo::getNbOrbits<VERTEX>(*map);

			mapParams.radiance = map->getAttribute<Utils::SphericalHarmonics<PFP2::REAL, PFP2::VEC3>, VERTEX, PFP2::MAP>("radiance") ;
			mapParams.radianceTexture = new Utils::Texture<2, Geom::Vec3f>(GL_FLOAT);
			mapParams.param = map->checkAttribute<Geom::Vec2i, VERTEX, PFP2::MAP>("param");

			// create texture
			unsigned int nbv_nbc = Algo::Topo::getNbOrbits<VERTEX>(*map) * Utils::SphericalHarmonics<PFP2::REAL, PFP2::VEC3>::get_nb_coefs();
			unsigned int size = 1;
			while (size * size < nbv_nbc)
				size <<= 1;

			mapParams.radianceTexture->create(Geom::Vec2i(size, size));

			// fill texture
			unsigned int count = 0;
			foreach_cell<VERTEX>(*map, [&] (Vertex v)
			{
				unsigned int i = count / size;
				unsigned int j = count % size;
				mapParams.param[v] = Geom::Vec2i(i, j) ; // first index for current vertex
				for (int l = 0 ; l <= Utils::SphericalHarmonics<PFP2::REAL, PFP2::VEC3>::get_resolution() ; ++l)
				{
					for (int m = -l ; m <= l ; ++m)
					{
						i = count / size;
						j = count % size;
						(*(mapParams.radianceTexture))(i,j) = mapParams.radiance[v].get_coef(l, m);
						++count;
					}
				}
			}) ;
			// resulting texture : SH00_vx0, SH1-1_vx0, ..., SHlm_vx0, SH00_vx1, SH1-1_vx1, ..., SHlm_vx1, etc.
			// resulting param : param[vxI] points to SH00_vxI
			// the size of the texture is needed to know where to do the divisions and modulos.

			mapParams.radianceTexture->update();

			// uncomment this line to be able to load multiple objects with different SH basis
			// (decimation will be unavailable)
//			map->removeAttribute(mapParams.radiance);

			mapParams.paramVBO = new Utils::VBO();
			mapParams.paramVBO->updateData(mapParams.param);

			mapParams.radiancePerVertexShader = new Utils::ShaderRadiancePerVertex(Utils::SphericalHarmonics<PFP2::REAL, PFP2::VEC3>::get_resolution());
			registerShader(mapParams.radiancePerVertexShader);
		}

		this->pythonRecording("importFile", mhg->getName(), fi.baseName());

		return mhg;
	}
	else
		return NULL;
}

void Surface_Radiance_Plugin::decimate(const QString& mapName, const QString& positionAttributeName, const QString& normalAttributeName, float decimationGoal, bool halfCollapse, bool exportMeshes)
{
	MapHandler<PFP2>* mh = static_cast<MapHandler<PFP2>*>(m_schnapps->getMap(mapName));
	if(mh == NULL)
		return;

	VertexAttribute<PFP2::VEC3, PFP2::MAP> position = mh->getAttribute<PFP2::VEC3, VERTEX>(positionAttributeName);
	if(!position.isValid())
		return;

	VertexAttribute<PFP2::VEC3, PFP2::MAP> normal = mh->getAttribute<PFP2::VEC3, VERTEX>(normalAttributeName);
	if(!normal.isValid())
		return;

	PFP2::MAP* map = mh->getMap();

	unsigned int nbVertices = Algo::Topo::getNbOrbits<VERTEX>(*map);

	MapParameters& mapParams = h_mapParameterSet[mh];

	if (halfCollapse)
	{
		if (mapParams.positionApproximator == NULL)
		{
			mapParams.positionApproximator = new Algo::Surface::Decimation::Approximator_HalfCollapse<PFP2, PFP2::VEC3>(*map, position);
		}

		if (mapParams.normalApproximator == NULL)
		{
			mapParams.normalApproximator = new Algo::Surface::Decimation::Approximator_HalfCollapse<PFP2, PFP2::VEC3>(*map, normal);
		}

		if (mapParams.radianceApproximator == NULL)
		{
			mapParams.radianceApproximator = new Algo::Surface::Decimation::Approximator_HalfCollapse<PFP2, Utils::SphericalHarmonics<PFP2::REAL, PFP2::VEC3> >(*map, mapParams.radiance);
		}

		if (mapParams.selector == NULL)
		{
			mapParams.selector = new HalfEdgeSelector_Radiance<PFP2>(
				*map,
				position,
				normal,
				mapParams.radiance,
				*(Algo::Surface::Decimation::Approximator<PFP2, PFP2::VEC3, DART>*)(mapParams.positionApproximator),
				*(Algo::Surface::Decimation::Approximator<PFP2, PFP2::VEC3, DART>*)(mapParams.normalApproximator),
				*(Algo::Surface::Decimation::Approximator<PFP2, Utils::SphericalHarmonics<PFP2::REAL, PFP2::VEC3>, DART>*)(mapParams.radianceApproximator)
			);
		}
	}
	else
	{
		if (mapParams.positionApproximator == NULL)
		{
			mapParams.positionApproximator = new Algo::Surface::Decimation::Approximator_QEM<PFP2>(*map, position);
		}

		if (mapParams.normalApproximator == NULL)
		{
			mapParams.normalApproximator =
				new Algo::Surface::Decimation::Approximator_InterpolateAlongEdge<PFP2, PFP2::VEC3>(
					*map,
					normal,
					position,
					((Algo::Surface::Decimation::Approximator<PFP2, PFP2::VEC3, EDGE>*)(mapParams.positionApproximator))->getApproximationResultAttribute()
				);
		}

		if (mapParams.radianceApproximator == NULL)
		{
			mapParams.radianceApproximator =
				new Algo::Surface::Decimation::Approximator_InterpolateAlongEdge<PFP2, Utils::SphericalHarmonics<PFP2::REAL, PFP2::VEC3> >(
					*map,
					mapParams.radiance,
					position,
					((Algo::Surface::Decimation::Approximator<PFP2, PFP2::VEC3, EDGE>*)(mapParams.positionApproximator))->getApproximationResultAttribute()
				);
		}

		if (mapParams.selector == NULL)
		{
			mapParams.selector =
				new EdgeSelector_Radiance<PFP2>(
					*map,
					position,
					normal,
					mapParams.radiance,
					*(Algo::Surface::Decimation::Approximator<PFP2, PFP2::VEC3, EDGE>*)(mapParams.positionApproximator),
					*(Algo::Surface::Decimation::Approximator<PFP2, PFP2::VEC3, EDGE>*)(mapParams.normalApproximator),
					*(Algo::Surface::Decimation::Approximator<PFP2, Utils::SphericalHarmonics<PFP2::REAL, PFP2::VEC3>, EDGE>*)(mapParams.radianceApproximator)
				);

			mapParams.selector =
				new Algo::Surface::Decimation::EdgeSelector_QEM<PFP2>(
					*map,
					position,
					*(Algo::Surface::Decimation::Approximator<PFP2, PFP2::VEC3, EDGE>*)(mapParams.positionApproximator)
				);
		}
	}

	std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP2>*> approximators;
	approximators.push_back(mapParams.positionApproximator);
	approximators.push_back(mapParams.normalApproximator);
	approximators.push_back(mapParams.radianceApproximator);

	exportNbVert.clear();
	if (exportMeshes)
	{
		unsigned int goalNbV = decimationGoal * nbVertices;
		unsigned int curNbV = nbVertices / 2;
		while (curNbV > goalNbV)
		{
			exportNbVert.push_back(curNbV);
			curNbV /= 2;
		}
		exportNbVert.push_back(goalNbV);
		nextExportIndex = 0;
	}

	std::cout << "nb vert -> " << nbVertices << std::endl;
	for (unsigned int v : exportNbVert)
	{
		std::cout << v << std::endl;
	}

	m_currentlyDecimatedMap = mh;
	m_currentDecimationHalf = halfCollapse;
	Algo::Surface::Decimation::decimate<PFP2>(*map, mapParams.selector, approximators, decimationGoal * nbVertices, true, NULL, (void (*)(void*, const void*))(Surface_Radiance_Plugin::checkNbVerticesAndExport), (void*)(this));
	m_currentlyDecimatedMap = NULL;

	mh->notifyConnectivityModification();
	mh->notifyAttributeModification(position);
}

void Surface_Radiance_Plugin::checkNbVerticesAndExport(Surface_Radiance_Plugin* p, const unsigned int* nbVertices)
{
	if (!p->exportNbVert.empty())
	{
		MapHandlerGen* mhg = p->currentlyDecimatedMap();
		if (*nbVertices == p->exportNbVert[p->nextExportIndex])
		{
			std::stringstream exportName;
			exportName << p->currentlyDecimatedMap()->getName().toStdString() << "_" << (p->currentDecimationHalf() ? "half_" : "full_") << (*nbVertices) << ".ply";
			std::cout << "export : " << exportName.str() << std::endl;
			p->exportPLY(mhg->getName(), "position", "normal", QString::fromStdString(exportName.str()));
			p->nextExportIndex++;
		}
	}
}

void Surface_Radiance_Plugin::exportPLY(
	const QString& mapName,
	const QString& positionAttributeName,
	const QString& normalAttributeName,
	const QString& filename)
{
	typedef PFP2::MAP MAP;
	typedef PFP2::REAL REAL;
	typedef PFP2::VEC3 VEC3;

	MapHandler<PFP2>* mh = static_cast<MapHandler<PFP2>*>(m_schnapps->getMap(mapName));
	if(mh == NULL)
		return;

	VertexAttribute<VEC3, MAP> position = mh->getAttribute<VEC3, VERTEX>(positionAttributeName);
	if(!position.isValid())
		return;

	VertexAttribute<VEC3, MAP> normal = mh->getAttribute<VEC3, VERTEX>(normalAttributeName);
	if(!normal.isValid())
		return;

	VertexAttribute<Utils::SphericalHarmonics<REAL, VEC3>, MAP> radiance = h_mapParameterSet[mh].radiance;
	if(!radiance.isValid())
		return;

	// open file
	std::ofstream out ;
	out.open(filename.toStdString(), std::ios::out | std::ios::binary) ;

	if (!out.good())
	{
		CGoGNerr << "Unable to open file " << CGoGNendl ;
		return ;
	}

	MAP* map = mh->getMap();

	unsigned int nbDarts = map->getNbDarts() ;
	std::vector<unsigned int> facesSize ;
	std::vector<std::vector<unsigned int> > facesIdx ;
	facesSize.reserve(nbDarts/3) ;
	facesIdx.reserve(nbDarts/3) ;
	std::map<unsigned int, unsigned int> vIndex ;
	unsigned int vCpt = 0 ;
	std::vector<unsigned int> vertices ;
	vertices.reserve(nbDarts/6) ;

	// Go over all faces
	CellMarker<MAP, VERTEX> markV(*map) ;
	TraversorF<MAP> t(*map) ;
	for(Dart d = t.begin(); d != t.end(); d = t.next())
	{
		std::vector<unsigned int> fidx ;
		fidx.reserve(8) ;
		unsigned int degree = 0 ;
		Traversor2FV<MAP> tfv(*map, d) ;
		for(Dart it = tfv.begin(); it != tfv.end(); it = tfv.next())
		{
			++degree ;
			unsigned int vNum = map->getEmbedding<VERTEX>(it) ;
			if(!markV.isMarked(it))
			{
				markV.mark(it) ;
				vIndex[vNum] = vCpt++ ;
				vertices.push_back(vNum) ;
			}
			fidx.push_back(vIndex[vNum]) ;
		}
		facesSize.push_back(degree) ;
		facesIdx.push_back(fidx) ;
	}

	// Start writing the file
	out << "ply" << std::endl ;

	// test endianness
	union
	{
		uint32_t i ;
		char c[4] ;
	} bint = {0x01020304} ;
	if (bint.c[0] == 1) // big endian
		out << "format binary_big_endian 1.0" << std::endl ;
	else
		out << "format binary_little_endian 1.0" << std::endl ;

	out << "comment File generated by the CGoGN library" << std::endl ;
	out << "comment See : http://cgogn.unistra.fr/" << std::endl ;
	out << "comment or contact : cgogn@unistra.fr" << std::endl ;
	// Vertex elements
	out << "element vertex " << vertices.size() << std::endl ;

	std::string nameOfTypePly_REAL(nameOfTypePly(position[0][0])) ;

	out << "property " << nameOfTypePly_REAL << " x" << std::endl ;
	out << "property " << nameOfTypePly_REAL << " y" << std::endl ;
	out << "property " << nameOfTypePly_REAL << " z" << std::endl ;


	out << "property " << nameOfTypePly_REAL << " nx" << std::endl ;
	out << "property " << nameOfTypePly_REAL << " ny" << std::endl ;
	out << "property " << nameOfTypePly_REAL << " nz" << std::endl ;

	int res = Utils::SphericalHarmonics<REAL, VEC3>::get_resolution() ;
	for (int l = 0 ; l <= res ; ++l)
	{
		for (int m = -l ; m <= l ; ++m)
		{
			out << "property " << nameOfTypePly_REAL << " SHcoef_" << l << "_" << m << "_r" << std::endl ;
			out << "property " << nameOfTypePly_REAL << " SHcoef_" << l << "_" << m << "_g" << std::endl ;
			out << "property " << nameOfTypePly_REAL << " SHcoef_" << l << "_" << m << "_b" << std::endl ;
		}
	}

	// Face element
	out << "element face " << facesSize.size() << std::endl ;
	out << "property list uint8 " << nameOfTypePly(facesIdx[0][0]) << " vertex_indices" << std::endl ;
	out << "end_header" << std::endl ;

	// binary vertices
	for(unsigned int i = 0; i < vertices.size(); ++i)
	{
		const VEC3& p = position[vertices[i]] ;
		out.write((char*)(&(p[0])), sizeof(p)) ;
		const VEC3& n = normal[vertices[i]] ;
		out.write((char*)(&(n[0])), sizeof(n)) ;

		for (int l=0 ; l <= res ; ++l)
		{
			for (int m=-l ; m <= l ; ++m)
			{
				const VEC3& r = radiance[vertices[i]].get_coef(l,m) ;
				out.write((char*)(&(r[0])), sizeof(r)) ;
			}
		}
	}

	// binary faces
	for(unsigned int i = 0; i < facesSize.size(); ++i)
	{
		uint8_t nbe = facesSize[i] ;
		out.write((char*)(&nbe), sizeof(uint8_t)) ;
		out.write((char*)(&(facesIdx[i][0])), facesSize[i] * sizeof(facesIdx[i][0])) ;
	}

	out.close() ;

	this->pythonRecording("exportPLY", "", mapName, positionAttributeName, normalAttributeName, filename);
}


#if CGOGN_QT_DESIRED_VERSION == 5
	Q_PLUGIN_METADATA(IID "CGoGN.SCHNapps.Plugin")
#else
	Q_EXPORT_PLUGIN2(Surface_Radiance_Plugin, Surface_Radiance_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
