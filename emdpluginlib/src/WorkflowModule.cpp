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

#include "WorkflowModule.h"

#include <vector>

#include <QDomElement>

#include "Frame.h"
#include "ModuleSource.h"
#include "WorkContext.h"
#include "WorkerThread.h"

namespace emd
{

EMD_MODULE_DEFINITION(WorkflowModule)

static const unsigned int MAX_FRAMES_PER_THREAD = UINT_MAX;

/***************************** Static Methods ********************************/

static std::map<std::string, std::map<std::string, ModuleSource *>> s_moduleMaps;
static std::map<std::string, std::string> s_workflowDescriptions;

void WorkflowModule::declare(const std::string &group,
                             const std::string &name,
                             ModuleSource *source,
                             const std::string &description)
{
    std::map<std::string, ModuleSource *> &map = s_moduleMaps[group];

    map[name] = source;

    if(!description.empty())
    {
        s_workflowDescriptions[group + name] = description;
    }
}

WorkflowModule *WorkflowModule::create(const std::string &group,
                          const std::string &name)
{
    std::map<std::string, ModuleSource *> &map = s_moduleMaps[group];

    ModuleSource *source = map[name];

    if(source)
    {
        WorkflowModule *module = source->createModule(group, name);

        if(!module)
            return nullptr;

        module->m_name = QString(name.c_str());
        module->m_group = QString(group.c_str());

        return module;
    }

    return nullptr;
}

int WorkflowModule::count()
{
    int size = 0;

    for(auto map : s_moduleMaps)
    {
        size += (int)map.second.size();
    }

    return size;
}

int WorkflowModule::count(const std::string &group)
{
    if(s_moduleMaps.count(group) == 1)
        return (int)s_moduleMaps[group].size();

    return 0;
}

int WorkflowModule::groupCount()
{
    return (int)s_moduleMaps.size();
}

std::vector<std::string> WorkflowModule::groups()
{
    std::vector<std::string> groups;

    for(auto map : s_moduleMaps)
    {
        groups.push_back(map.first);
    }

    return groups;
}

std::vector<std::string> WorkflowModule::modules(const std::string &group)
{
    std::vector<std::string> modules;

    if(s_moduleMaps.count(group) == 1)
    {
        for(auto map : s_moduleMaps.at(group))
        {
            modules.push_back(map.first);
        }
    }

    return modules;
}

std::string WorkflowModule::description(const std::string &group, const std::string &name)
{
    std::string key = group + name;
    if(s_workflowDescriptions.count(key) == 1)
    {
        return s_workflowDescriptions[key];
    }

    return std::string();
}

WorkflowModule::WorkflowModule()
	:
	m_outdated(true),
	m_enabled(true),
    m_active(true),
    m_controlDisplayed(true)
{
    setProperty("ControlDisplayed", "true");
}

WorkflowModule::~WorkflowModule()
{
    
}

bool WorkflowModule::validate() const
{
    if(m_inputModules.count() > 0 && !m_inputContext.isValid())
        return false;

    return true;
}

bool WorkflowModule::addOutput(WorkflowModule *module, const QString &outputType)
{
    if(!module)
		return false;

    if(outputType.compare("Default") != 0)
    {
        qCritical() << "Unknown workflow output type: " << outputType;
        return false;
    }

	m_outputModules.append(module);
    m_outputTypes[module] = outputType;

	return true;
}

void WorkflowModule::removeOutput(WorkflowModule *module)
{
    if(!m_outputModules.contains(module))
        return;

    m_outputModules.removeAll(module);
    m_outputTypes.remove(module);
}

bool WorkflowModule::addInput(WorkflowModule *module, const QString &inputType)
{
    if(!module)
		return false;

    if(inputType.compare("Default") != 0)
    {
        qCritical() << "Unknown workflow input type: " << inputType;
        return false;
    }

	m_inputModules.append(module);
    m_inputTypes[module] = inputType;

	return true;
}

void WorkflowModule::removeInput(WorkflowModule *module)
{
    if(!m_inputModules.contains(module))
        return;

    m_inputModules.removeAll(module);
    m_inputTypes.remove(module);
}

void WorkflowModule::detach()
{
    for(WorkflowModule *module : m_inputModules)
    {
        module->removeOutput(this);
    }

    for(WorkflowModule *module : m_outputModules)
    {
        module->removeInput(this);
    }
}

const QList<WorkflowModule*> &WorkflowModule::outputModules() const
{
    return m_outputModules;
}
const QList<WorkflowModule*> &WorkflowModule::inputModules() const
{
    return m_inputModules;
}

QString WorkflowModule::outputType(WorkflowModule *output) const
{
    if(!m_outputModules.contains(output))
        return QString();

    return m_outputTypes[output];
}

QString WorkflowModule::inputType(WorkflowModule *input) const
{
    if(!m_inputModules.contains(input))
        return QString();

    return m_inputTypes[input];
}

QWidget *WorkflowModule::controlWidget()
{
    return NULL;
}

void WorkflowModule::reset()
{
    m_inputContext.reset();
    m_outputContext.reset();
}

void WorkflowModule::save(QDomElement &element, QDomDocument &doc)
{
    element.setAttribute("name", this->name());
    element.setAttribute("group", this->group());
    element.setAttribute("id", this->uniqueId());

    for(const QString &key : m_properties.keys())
    {
        QDomElement propElement = doc.createElement("property");
        propElement.setAttribute("key", key);
        propElement.setAttribute("value", m_properties[key].toString());
        propElement.setAttribute("type", m_properties[key].typeName());

        WorkflowModule *module = nullptr;
        const QList<ListenerTarget> &listeners = m_listenerMap[key];
        for(const ListenerTarget &lt : listeners)
        {
            if(module = dynamic_cast<WorkflowModule*>(lt.listener))
            {
                QDomElement e = doc.createElement("listener");
                e.setAttribute("id", module->uniqueId());
                e.setAttribute("target", lt.target);
            }
        }

        element.appendChild(propElement);
    }

    for(WorkflowModule *output : m_outputModules)
    {
        QDomElement e = doc.createElement("output");
        e.setAttribute("id", output->uniqueId());
        e.setAttribute("type", this->outputType(output));
        element.appendChild(e);
    }

    for(WorkflowModule *input : m_inputModules)
    {
        QDomElement e = doc.createElement("input");
        e.setAttribute("id", input->uniqueId());
        e.setAttribute("type", this->inputType(input));
        element.appendChild(e);
    }
}

void WorkflowModule::load(QDomElement & /*element*/)
{
    
}

WorkflowModule::RequiredFeatures WorkflowModule::requiredFeatures() const
{
    return NoFeatures;
}

void WorkflowModule::preprocess()
{
    
}

Frame *WorkflowModule::processFrame(Frame *frame, int /*index*/)
{
    // The output frame is a copy of the input frame, except that it doesn't
    // own the data.
    return new Frame(frame->data<void>(), frame->dataType(), false);
}

void WorkflowModule::process()
{
	m_activeThreadCount = 0;
    m_frameIndex = 0;
    
    m_outputContext.reset();
    m_outputContext.init(m_inputContext.frameCount());
	
	WorkerThread *thread = NULL;
    WorkContext *context = NULL;

    while(WorkerThread::available() && (context = this->workContext()) )
	{
        thread = WorkerThread::takeThread();

		connect(thread, SIGNAL(finishedProcessing(WorkerThread *)),
			this, SLOT(scheduleWork(WorkerThread *)));
		++m_activeThreadCount;

        thread->work(context);
	}

    // If we didn't schedule any threads, we're finished
    if(m_activeThreadCount == 0)
        emit(workFinished(this));
}

void WorkflowModule::postprocess()
{
	
}

void WorkflowModule::configureOutputModule(WorkflowModule *next)
{
    if(!next)
        return;

    if(m_outputTypes[next].compare("Default") != 0)
    {
        qWarning() << "Performing default configuration for output module with type "
            << m_outputTypes[next];
    }

    next->setInputContext(m_outputContext, this);
}

void WorkflowModule::scheduleWork(WorkerThread *thread)
{	
    WorkContext *context = this->workContext();

	if(context)
	{
		thread->work(context);
	}
	else
	{
		// If there are no frames left, return the thread
		disconnect(thread, SIGNAL(finishedProcessing(WorkerThread *)),
			this, SLOT(scheduleWork(WorkerThread *)));

		WorkerThread::returnThread(thread);
		--m_activeThreadCount;

		if(m_activeThreadCount == 0)
			emit(workFinished(this));
	}
}

WorkContext *WorkflowModule::workContext()
{
    int remainingFrames = m_inputContext.frameCount() - m_frameIndex;

    if(remainingFrames <= 0)
        return NULL;

    WorkContext *context = new WorkContext(this);
    // If there are unprocessed frames remaining, process them
	uint64_t frameCount = remainingFrames < MAX_FRAMES_PER_THREAD ?
	                        remainingFrames : MAX_FRAMES_PER_THREAD;

    context->setStart(m_frameIndex);
    context->setCount(frameCount);

    m_frameIndex += frameCount;

    return context;
}

void WorkflowModule::doWork(WorkContext *context)
{
    for(int index = context->start(); index < context->start() + context->count(); ++index)
    {
        if(index < m_inputContext.frameCount())
        {
            Frame *outputFrame = NULL;
            if(m_inputContext.frameAtIndex(index))
            {
	            outputFrame = this->processFrame(m_inputContext.frameAtIndex(index), index);
            }

            if(outputFrame)
            {
                outputFrame->setIndex(m_inputContext.frameAtIndex(index)->index());

                m_outputContext.setFrameAtIndex(outputFrame, index);
            }
        }
        else
        {
            qWarning() << "Invalid input index (" << index << ") in " << this->name();
        }
    }
}

/********************************* Base class methods ********************************/

QString WorkflowModule::name() const
{
    return m_name;
}

QString WorkflowModule::group() const
{
    return m_group;
}

QString WorkflowModule::description() const
{
    return QString(WorkflowModule::description(m_name.toStdString(), m_group.toStdString()).c_str());
}

int32_t WorkflowModule::uniqueId() const
{
    return (int32_t)(0x00000000FFFFFFFF & (int64_t)this);
}

QVariant WorkflowModule::property(const QString &key) const
{
    return m_properties.value(key);
}

bool WorkflowModule::setProperty(const QString &key, const QVariant &value)
{
    //if(m_properties.contains(key))
    {
        m_properties[key] = value;

        this->doPropertyChanged(key);

        QList<ListenerTarget> &listeners = m_listenerMap[key];
        for(ListenerTarget &lt : listeners)
        {
            if(lt.listener)
                lt.listener->modulePropertyChanged(this, key, lt.target);
        }

        return true;
    }

    return false;
}

void WorkflowModule::doPropertyChanged(const QString & /*key*/)
{
    update();
}

void WorkflowModule::addListener(const QString &key, 
                                 ModuleListener *listener, const QString &target)
{
    if(!listener)
        return;

    if(!m_properties.contains(key))
        return;

    QList<ListenerTarget> &listeners = m_listenerMap[key];

    ListenerTarget lt;
    lt.listener = listener;
    lt.target = target;

    listeners.append(lt);

    listener->modulePropertyChanged(this, key, target);
}

void WorkflowModule::modulePropertyChanged(WorkflowModule *sender, const QString &key, const QString &target)
{
    this->setProperty(target, sender->property(key));
}

void WorkflowModule::setInputContext(ProcessingContext context, WorkflowModule *previous)
{
    if(m_inputTypes[previous].compare("Default") != 0)
    {
        qWarning() << "Performing default configuration for module with input type "
            << m_inputTypes[previous];
    }

    m_inputContext = context;

    if(!this->active())
    {
        for(WorkflowModule *module : m_outputModules)
        {
            module->setInputContext(context, this);
        } 
    }
}

bool WorkflowModule::enabled() const
{
	return m_enabled;
}

void WorkflowModule::setEnabled(bool enabled)
{
	if(enabled == m_enabled)
		return;

	m_enabled = enabled;

    if(enabled && outdated())
        emit(moduleOutdated(this));
}

bool WorkflowModule::active() const
{
    return m_active;
}

void WorkflowModule::setActive(bool active)
{
    if(active == m_active)
        return;

    m_active = active;

    emit(activityChanged(m_active));
}

bool WorkflowModule::outdated() const
{
	return m_outdated;
}

void WorkflowModule::setOutdated(bool outdated)
{
	if(outdated == m_outdated)
		return;

	if(m_enabled)
		m_outdated = outdated;

	if(outdated)
	{
		for(WorkflowModule *module : m_outputModules)
			module->setOutdated(true);
	}
}

void WorkflowModule::update()
{
	this->setOutdated(true);

    if(enabled() && validate())
	    emit(moduleOutdated(this));
}

} // namespace emd


