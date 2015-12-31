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

#include "Workflow.h"

#include <map>

#include <qboxlayout.h>
#include <QDebug>
#include <qlabel.h>
#include <qwidget.h>

#include <QtXml>

#include "Frame.h"
#include "ProcessingContext.h"
#include "WorkerThread.h"
#include "WorkflowModule.h"
#include "WorkflowSource.h"

namespace emd
{

/***************************** Static Methods ********************************/

static std::map<std::string, std::map<std::string, WorkflowSource *>> s_workflowMaps;
static std::map<std::string, std::string> s_workflowDescriptions;

void Workflow::declare(const std::string &group,
                       const std::string &name,
                       WorkflowSource *source,
                       const std::string &description)
{
    std::map<std::string, WorkflowSource *> &map = s_workflowMaps[group];

    map[name] = source;

    if(!description.empty())
    {
        s_workflowDescriptions[group + name] = description;
    }
}

Workflow *Workflow::create(const std::string &group,
                           const std::string &name)
{
    std::map<std::string, WorkflowSource *> &map = s_workflowMaps[group];

    WorkflowSource *source = map[name];

    if(source)
    {
        Workflow *workflow = new Workflow(QString(group.c_str()) + "." + QString(name.c_str()));
        if(!workflow)
            return nullptr;

        workflow->m_name = QString(name.c_str());
        workflow->m_group = QString(group.c_str());

        source->configureWorkflow(group, name, *workflow);

        return workflow;
    }

    return nullptr;
}

int Workflow::count()
{
    int size = 0;

    for(auto map : s_workflowMaps)
    {
        size += (int)map.second.size();
    }

    return size;
}

int Workflow::count(const std::string &group)
{
    if(s_workflowMaps.count(group) == 1)
        return (int)s_workflowMaps[group].size();

    return 0;
}

int Workflow::groupCount()
{
    return (int)s_workflowMaps.size();
}

std::vector<std::string> Workflow::groups()
{
    std::vector<std::string> groups;

    for(auto map : s_workflowMaps)
    {
        groups.push_back(map.first);
    }

    return groups;
}

std::vector<std::string> Workflow::workflows(const std::string &group)
{
    std::vector<std::string> workflows;

    if(s_workflowMaps.count(group) == 1)
    {
        for(auto map : s_workflowMaps.at(group))
        {
            workflows.push_back(map.first);
        }
    }

    return workflows;
}

std::string Workflow::description(const std::string &group, const std::string &name)
{
    std::string key = group + name;
    if(s_workflowDescriptions.count(key) == 1)
    {
        return s_workflowDescriptions[key];
    }

    return std::string();
}

/******************************* Instance Methods *******************************/

Workflow::Workflow(const QString &name)
    : m_active(false),
    m_paused(false),
    m_name(name)
{

}

Workflow::~Workflow()
{
	for(WorkflowModule *module : m_modules)
		module->deleteLater();
}

QString Workflow::name() const
{
    return m_name;
}

QString Workflow::group() const
{
    return m_group;
}

QString Workflow::description() const
{
    return QString(Workflow::description(m_group.toStdString(), m_name.toStdString()).c_str());
}

void Workflow::addModule(WorkflowModule *module)
{
    // TODO: should automatically add all attached modules?
	m_modules.append(module);

	connect(module, SIGNAL(moduleOutdated(WorkflowModule *)),
		this, SLOT(moduleOutdated(WorkflowModule *)));
	connect(module, SIGNAL(workFinished(WorkflowModule *)),
		this, SLOT(moduleFinishedProcessing(WorkflowModule *)));
}

void Workflow::removeModule(WorkflowModule *module)
{
    m_modules.removeAll(module);

    module->detach();
}

const QList<WorkflowModule *> &Workflow::modules() const
{
	return m_modules;
}


bool Workflow::paused() const
{
    return m_paused;
}

void Workflow::setPaused(bool paused)
{
    m_paused = paused;

    if(!m_paused)
    {
        start();
    }
}

WorkflowModule::RequiredFeatures Workflow::requiredFeatures() const
{
    WorkflowModule::RequiredFeatures features = WorkflowModule::NoFeatures;

    for(int iii = 0; iii < m_modules.count(); ++iii)
    {
        features |= m_modules.at(iii)->requiredFeatures();
    }
    
    return features;
}

QWidget *Workflow::controlWidget()
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    //QLabel *titleLabel = new QLabel(m_group + "." + m_name);
    //layout->addWidget(titleLabel, 0, Qt::AlignCenter);

    QWidget *moduleWidget = NULL;

    for(int index = 0; index < m_modules.count(); ++index)
    {
        if(m_modules.at(index)->property("ControlDisplayed").toBool())
        {
            moduleWidget = m_modules.at(index)->controlWidget();
            if(moduleWidget)
                layout->addWidget(moduleWidget);
        }
    }

    if(layout->count() == 0)
        return NULL;

    QWidget *controlWidget = new QWidget();
    //controlWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    controlWidget->setLayout(layout);

    return controlWidget;
}

bool Workflow::readyToStart() const
{
	if(m_modules.size() == 0)
		return false;

	for(WorkflowModule *module : m_modules)
	{
		if(!module->validate())
			return false;
	}

    if(m_paused)
        return false;

	return true;
}

void Workflow::start()
{
	if(m_active)
		return;

    if(m_modulesToProcess.count() == 0)
    {
        QList<WorkflowModule*> inputModules;
	    if(!findInputModules(inputModules))
		    return;

	    m_modulesToProcess.append(inputModules);
    }

	m_active = true;

	processNextModule();
}

void Workflow::reset()
{
    for(int index = 0; index < m_modules.size(); ++ index)
    {
        m_modules.at(index)->reset();
    }
}

bool Workflow::save(const QString &path)
{
    QFile file(path);
    if(file.exists())
    {
        bool removed = file.remove();
        if(!removed)
        {
            qCritical() << "Failed to delete file at " << path;
            return false;
        }
    }

    bool opened = file.open(QIODevice::WriteOnly);
    if(!opened)
    {
        qCritical() << "Failed to create file at " << path;
        return false;
    }

    QDomDocument doc("workflow");
    
    QDomElement root = doc.createElement("workflow");
    doc.appendChild(root);
    root.setAttribute("name", m_name);
    root.setAttribute("group", m_group);

    for(WorkflowModule *module : m_modules)
    {
        QDomElement element = doc.createElement("module");
        root.appendChild(element);

        module->save(element, doc);
    }

    QTextStream out(&file);
    doc.save(out, 0);

    file.close();

    return true;
}

bool Workflow::load(const QString &path)
{
    QFile file(path);
    if(!file.exists())
    {
        qCritical() << "File does not exist: " << path;
        return false;
    }

    if(!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Failed to open file: " << path;
        return false;
    }

    QDomDocument doc("workflow");
    doc.setContent(&file);

    QDomElement root = doc.firstChildElement("workflow");
    if(!root.isElement())
    {
        qCritical() << "Invalid workflow file: " << path;
        return false;
    }

    QDomNodeList moduleList = root.elementsByTagName("module");

    QMap<QString, WorkflowModule *> idMap;

    for(int index = 0; index < moduleList.count(); ++index)
    {
        QDomNode node = moduleList.item(index);
        if(node.isElement())
        {
            QDomElement element = node.toElement();
            QString name = element.attribute("name");
            QString group = element.attribute("group");
            
            WorkflowModule *module = WorkflowModule::create(group.toStdString(),
                                                            name.toStdString());

            if(!module)
            {
                qCritical() << "Missing required module for workflow " << m_name
                    << " in group " << m_group << ": " << group << "." << name;
                return false;
            }

            idMap[element.attribute("id", "missing")] = module;

            module->load(element);

            addModule(module);
        }
    }

    for(int index = 0; index < moduleList.count(); ++index)
    {
        QDomNode node = moduleList.item(index);
        if(node.isElement())
        {
            QDomElement element = node.toElement();

            QString id = element.attribute("id");
            if(id.isNull())
                continue;

            WorkflowModule *module = idMap[id];

            QDomNodeList pList = element.elementsByTagName("property");
            for(int index = 0; index < pList.size(); ++index)
            {
                if(pList.at(index).isElement())
                {
                    QDomElement prop = pList.at(index).toElement();

                    if(prop.attribute("key").isEmpty())
                        continue;

                    if(!prop.attribute("value").isEmpty())
                        module->setProperty(prop.attribute("key"), prop.attribute("value"));

                    QDomNodeList listeners = prop.elementsByTagName("listener");
                    for(int li = 0; li < listeners.size(); ++li)
                    {
                        if(listeners.at(li).isElement())
                        {
                            QDomElement listener = listeners.at(li).toElement();
                            WorkflowModule *lm = idMap[listener.attribute("id")];
                            module->addListener(prop.attribute("key"), lm, 
                                listener.attribute("target"));
                        }
                    }
                }
            }

            QDomNodeList outList = element.elementsByTagName("output");
            for(int iii = 0; iii < outList.size(); ++iii)
            {
                const QDomNode &node = outList.at(iii);
                if(node.isElement())
                {
                    QDomElement e = node.toElement();
                    QString nid = e.attribute("id");
                    if(nid.isNull())
                        continue;

                    QString type = e.attribute("type");

                    module->addOutput(idMap[nid], type);
                }
            }

            QDomNodeList inList = element.elementsByTagName("input");
            for(int iii = 0; iii < inList.size(); ++iii)
            {
                const QDomNode &node = inList.at(iii);
                if(node.isElement())
                {
                    QDomElement e = node.toElement();
                    QString nid = e.attribute("id");
                    if(nid.isNull())
                        continue;

                    QString type = e.attribute("type");

                    module->addInput(idMap[nid], type);
                }
            }
        }
    }

    return true;
}

/****************************** Private *************************************/

void Workflow::processNextModule()
{
    WorkflowModule *nextModule = NULL;

    if(m_modulesToProcess.count() == 0)
    {
        finish();
        return;
    }

	while(!nextModule || !nextModule->validate() 
        || !nextModule->active() || !nextModule->enabled())
	{
        if(nextModule)
        {
            if(nextModule->validate() && nextModule->enabled())
            {
                QList<WorkflowModule*> outputModules = nextModule->outputModules();

                for(int index = 0; index < outputModules.count(); ++index)
                {
                    m_modulesToProcess.insert(index, outputModules.at(index));
                }
            }
        }

		if(m_modulesToProcess.count() > 0)
		{
			nextModule = m_modulesToProcess.takeFirst();
		}
		else
		{
            finish();
            return;
		}
	}

	nextModule->preprocess();

	nextModule->process();
}

bool Workflow::findInputModules(QList<WorkflowModule*> &inputModules)
{
    if(m_modules.size() == 0)
        return false;

    for(WorkflowModule *module : m_modules)
    {
        if(module->inputModules().size() == 0)
            inputModules.append(module);
    }

    if(inputModules.size() == 0)
        return false;

    return true;
}

void Workflow::finish()
{
    m_active = false;

    emit(finishedProcessing());
}

/**************************** Slots ************************************/

void Workflow::moduleOutdated(WorkflowModule *module)
{
	if(!m_modulesToProcess.contains(module))
    {
        m_modulesToProcess.append(module);

	    if(!m_active && !m_paused)
        {
            m_active = true;
            processNextModule();
        }
    }
}

void Workflow::moduleFinishedProcessing(WorkflowModule *module)
{
    // Remove duplicates
    // Commented out, assume that if the module appears again here
    // then it needs to be reprocessed?
    //m_modulesToProcess.removeAll(module);

    module->postprocess();

	QList<WorkflowModule*> outputModules = module->outputModules();

    for(int index = 0; index < outputModules.count(); ++index)
    {
        WorkflowModule *output = outputModules.at(index);

        module->configureOutputModule(output);

        if(output->enabled() && output->validate())
            m_modulesToProcess.insert(index, output);
    }

	processNextModule();
}

} // namespace emd

