#ifndef _SURFACE_IMPORT_PLUGIN_H_
#define _SURFACE_IMPORT_PLUGIN_H_

#include "plugin_processing.h"

namespace CGoGN
{

namespace SCHNApps
{

class Surface_Import_Plugin : public PluginProcessing
{
	Q_OBJECT
	Q_INTERFACES(CGoGN::SCHNApps::Plugin)
#if CGOGN_QT_DESIRED_VERSION == 5
	Q_PLUGIN_METADATA(IID "CGoGN.SCHNapps.Plugin")
#endif

public:
	Surface_Import_Plugin()
	{}

	~Surface_Import_Plugin()
	{}

private:
	virtual bool enable();
	virtual void disable() {}

public slots:
	MapHandlerGen* importMeshFromFile(const QString& fileName);
	void importMeshFromFileDialog();

	MapHandlerGen* importImageFromFile(const QString& fileName);
	void importImageFromFileDialog();

private:
	QAction* importMeshAction;
	QAction* importImageAction;
};

} // namespace SCHNApps

} // namespace CGoGN

#endif
