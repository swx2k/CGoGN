#ifndef _SURFACE_SUBDIVISION_PLUGIN_H_
#define _SURFACE_SUBDIVISION_PLUGIN_H_

#include "plugin_processing.h"

#include "dialog_surface_subdivision.h"

namespace CGoGN
{

namespace SCHNApps
{

class Surface_Subdivision_Plugin : public Plugin
{
	Q_OBJECT
	Q_INTERFACES(CGoGN::SCHNApps::Plugin)
#if CGOGN_QT_DESIRED_VERSION == 5
	Q_PLUGIN_METADATA(IID "CGoGN.SCHNapps.Plugin")
#endif
public:
	Surface_Subdivision_Plugin()
	{}

	~Surface_Subdivision_Plugin()
	{}

	virtual bool enable();
	virtual void disable();

private slots:
	void openSubdivisionDialog();
	void subdivideFromDialog();
	void schnappsClosing();

public slots:
	void loopSubdivision(
		const QString& mapName,
		const QString& positionAttributeName = "position");

	void CCSubdivision(
		const QString& mapName,
		const QString& positionAttributeName = "position",
		bool interp = false );

	void DoSabinSubdivision(
		const QString& mapName,
		const QString& positionAttributeName = "position");

	void trianguleFaces(
		const QString& mapName,
		const QString& positionAttributeName = "position");

	void quadranguleFaces(
		const QString& mapName,
		const QString& positionAttributeName = "position");

private:
	Dialog_Surface_Subdivision* m_subdivisionDialog;

	QAction* m_subdivisionAction;
};

} // namespace SCHNApps

} // namespace CGoGN

#endif
