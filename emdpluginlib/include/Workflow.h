/*
 * emdViewer, a program for working with electron microscopy dataset 
 * (emd) files.
 * Copyright (C) 2015  Phil Ophus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EMD_WORKFLOW_H
#define EMD_WORKFLOW_H

#include "EmdPluginLib.h"

#include <memory>
#include <vector>

#include <QList>
#include <QObject>

#include "WorkflowModule.h"

namespace emd
{

class ModuleSource;
class WorkflowSource;

class EMDPLUGIN_API Workflow : public QObject
{
	Q_OBJECT

public:
    static void declare(const std::string &group,
                        const std::string &name,
                        WorkflowSource *source,
                        const std::string &description = std::string());
    static Workflow *create(const std::string &group,
                            const std::string &name);

    static int count();
    static int count(const std::string &group);
    static int groupCount();
    static std::vector<std::string> groups();
    static std::vector<std::string> workflows(const std::string &group);
    static std::string description(const std::string &group, const std::string &name);

public:
	Workflow(const QString &name = QString());
	~Workflow();

    QString name() const;
    QString group() const;
    QString description() const;

	void addModule(WorkflowModule *module);
    void removeModule(WorkflowModule *module);

    const QList<WorkflowModule *> &modules() const;

    bool paused() const;
    void setPaused(bool paused);

    WorkflowModule::RequiredFeatures requiredFeatures() const;

    QWidget *controlWidget();

	bool readyToStart() const;
	void start();
    void reset();

    bool save(const QString &path);
    bool load(const QString &path);

private:
	void processNextModule();
    bool findInputModules(QList<WorkflowModule*> &inputModules);
    void finish();

public slots:
	void moduleOutdated(WorkflowModule *module);
	void moduleFinishedProcessing(WorkflowModule *module);

signals:
	// Control
	void readyToProcess();
    void finishedProcessing();

private:
    QString m_name;
    QString m_group;
	QList<WorkflowModule *> m_modules;
    QList<WorkflowModule *> m_modulesToProcess;
	bool m_active;
    bool m_paused;
};

} // namespace emd

#endif