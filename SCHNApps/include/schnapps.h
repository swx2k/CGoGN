#ifndef _SCHNAPPS_H_
#define _SCHNAPPS_H_

#include "dll.h"

#include "ui_schnapps.h"

#include "types.h"

#include "PythonQt.h"
#include "gui/PythonQtScriptingConsole.h"
#include "slot_debug.h"
#include <QTextStream>

class QVBoxLayout;
class QSplitter;
class QFile;


namespace CGoGN
{

namespace SCHNApps
{

class ControlDock_CameraTab;
class ControlDock_MapTab;
class ControlDock_PluginTab;

class SCHNAPPS_API SCHNApps : public QMainWindow, Ui::SCHNApps
{
	Q_OBJECT

public:
	SCHNApps(const QString& appPath, PythonQtObjectPtr& pythonContext, PythonQtScriptingConsole& pythonConsole);
	~SCHNApps();

public slots:
	const QString& getAppPath() { return m_appPath; }

	/*********************************************************
	 * MANAGE CAMERAS
	 *********************************************************/

public slots:
	Camera* addCamera(const QString& name);
	Camera* addCamera();
	void removeCamera(const QString& name);

	Camera* getCamera(const QString& name) const;
	const CameraSet& getCameraSet() const { return m_cameras; }

	/*********************************************************
	 * MANAGE VIEWS
	 *********************************************************/
public:
	void redrawAllViews();


public slots:
	View* addView(const QString& name);
	View* addView();
	void removeView(const QString& name);

	View* getView(const QString& name) const;
	const ViewSet& getViewSet() const { return m_views; }

	View* getSelectedView() const { return m_selectedView; }
	void setSelectedView(View* view);

	void splitView(const QString& name, Qt::Orientation orientation);

	QString saveSplitViewPositions();
	void restoreSplitViewPositions(QString stringStates);


	/*********************************************************
	 * MANAGE PLUGINS
	 *********************************************************/

public slots:
	void registerPluginsDirectory(const QString& path);

	Plugin* enablePlugin(const QString& pluginName);
	void disablePlugin(const QString& pluginName);

	Plugin* getPlugin(const QString& name) const;
	const PluginSet& getPluginSet() const { return m_plugins; }

	const QMap<QString, QString>& getAvailablePlugins() const { return m_availablePlugins; }

public:
	void addPluginDockTab(Plugin* plugin, QWidget* tabWidget, const QString& tabText);
	void removePluginDockTab(Plugin* plugin, QWidget* tabWidget);

private slots:
	void enablePluginTabWidgets(PluginInteraction* plugin);
	void disablePluginTabWidgets(PluginInteraction* plugin);

	/*********************************************************
	 * MANAGE MAPS
	 *********************************************************/

public slots:
	MapHandlerGen* addMap(const QString& name, unsigned int dim);
	void removeMap(const QString& name);
	MapHandlerGen* duplicateMap(const QString& name, bool properties);


	void setSelectedMap(const QString& mapName);

	MapHandlerGen* getMap(const QString& name) const;
	const MapSet& getMapSet() const { return m_maps; }

	void notifySelectedMapChanged(MapHandlerGen* old, MapHandlerGen* cur) { DEBUG_EMIT("selectedMapChanged"); emit(selectedMapChanged(old, cur)); }
	MapHandlerGen* getSelectedMap() const;

	unsigned int getCurrentOrbit() const;
	void notifySelectedCellSelectorChanged(CellSelectorGen* cs) { DEBUG_EMIT("selectedCellSelectorChanged"); emit(selectedCellSelectorChanged(cs)); }
	CellSelectorGen* getSelectedSelector(unsigned int orbit) const;

	const StaticPointers& getStaticPointers() const { return m_sp; }

	/*********************************************************
	 * MANAGE TEXTURES
	 *********************************************************/

public:
	Texture* getTexture(const QString& image);
	void releaseTexture(const QString& image);

	/*********************************************************
	 * MANAGE MENU ACTIONS
	 *********************************************************/

public:
	void addMenuAction(Plugin* plugin, const QString& menuPath, QAction* action);
	void removeMenuAction(Plugin* plugin, QAction* action);

public slots:
	void aboutSCHNApps();
	void aboutCGoGN();

	void showHideControlDock();
	void showHidePluginDock();
	void showHidePythonDock();

	void loadPythonScriptFromFile(const QString& fileName);

	void statusBarMessage(const QString& msg, int msec);

	QString openFileDialog(const QString& title, const QString& dir = QString(), const QString& filter = QString());

	QString saveFileDialog(const QString& title, const QString& dir = QString(), const QString& filter = QString());

	void setWindowSize(int w, int h) { this->resize(w, h); }

private slots:
	void loadPythonScriptFromFileDialog();


	/*********************************************************
	* MANAGE PYTHON RECORDING
	*********************************************************/
protected:
	QTextStream* m_pyRecording;
	QFile* m_pyRecFile;
	QList<QString> m_pyVarNames;
	QString m_pyBuffer;

private slots:
	void pyRecording();
	void appendPyRecording();
	//void endPyRecording();

public:
	inline QTextStream* pythonStreamRecorder()  { return m_pyRecording; }
	inline void pythonVarDeclare(const QString& var) { m_pyVarNames.push_back(var); }
	inline void pythonVarsClear() { m_pyVarNames.clear(); }

signals:
	void cameraAdded(Camera* camera);
	void cameraRemoved(Camera* camera);

	void viewAdded(View* view);
	void viewRemoved(View* view);
	void selectedViewChanged(View* old, View* cur);

	void mapAdded(MapHandlerGen* map);
	void mapRemoved(MapHandlerGen* map);
	void selectedMapChanged(MapHandlerGen* old, MapHandlerGen* cur);
	void selectedCellSelectorChanged(CellSelectorGen* cs);

	void pluginAvailableAdded(QString name);
	void pluginEnabled(Plugin* plugin);
	void pluginDisabled(Plugin* plugin);

	void schnappsClosing();

protected:
	QString m_appPath;
	PythonQtObjectPtr& m_pythonContext;
	PythonQtScriptingConsole& m_pythonConsole;

	QDockWidget* m_controlDock;
	QTabWidget* m_controlDockTabWidget;
	ControlDock_CameraTab* m_controlCameraTab;
	ControlDock_MapTab* m_controlMapTab;
	ControlDock_PluginTab* m_controlPluginTab;

	QDockWidget* m_pluginDock;
	QTabWidget* m_pluginDockTabWidget;

	QDockWidget* m_pythonDock;

	QVBoxLayout* m_centralLayout;
	QSplitter* m_rootSplitter;
	bool b_rootSplitterInitialized;

	View* m_firstView;
	View* m_selectedView;

	QMap<QString, QString> m_availablePlugins;
	QMap<Plugin*, QList<QWidget*> > m_pluginTabs;
	QMap<Plugin*, QList<QAction*> > m_pluginMenuActions;

	CameraSet m_cameras;
	ViewSet m_views;
	PluginSet m_plugins;
	MapSet m_maps;

	TextureSet m_textures;

	StaticPointers m_sp;

	void closeEvent(QCloseEvent *event);


};

} // namespace SCHNApps

} // namespace CGoGN

#endif
