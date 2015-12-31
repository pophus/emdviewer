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

#ifndef EMD_WORKFLOWMODULE_H
#define EMD_WORKFLOWMODULE_H

#include "EmdPluginLib.h"

#include <memory>
#include <stdint.h>
#include <vector>

#include <qlist.h>
#include <QObject>
#include <QString>

#include "DataGroup.h"
#include "Dataset.h"
#include "ProcessingContext.h"

#define EMD_MODULE_DECLARATION \
    private: \
    static int64_t kClassIdVar; \
    public: \
    static int64_t classId(); \
    virtual int64_t instanceId() const;

#define EMD_MODULE_DEFINITION(ModuleClass) \
    int64_t ModuleClass::kClassIdVar = 0; \
    int64_t ModuleClass::classId() \
    { \
        return (int64_t) &kClassIdVar; \
    } \
    int64_t ModuleClass::instanceId() const \
    { \
        return ModuleClass::classId(); \
    } 
        
class QDomDocument;
class QDomElement;

namespace emd
{

class Frame;
class ModuleSource;
class WorkContext;
class WorkerThread;
class WorkflowModule;

class ModuleListener 
{
public:
    virtual void modulePropertyChanged(WorkflowModule *module,
        const QString &key, const QString &target) = 0;
};

class EMDPLUGIN_API WorkflowModule : public QObject,
                                     public ModuleListener
{
	Q_OBJECT

    EMD_MODULE_DECLARATION

public:
    static void declare(const std::string &group,
                        const std::string &name,
                        ModuleSource *source,
                        const std::string &description = std::string());
    static WorkflowModule *create(const std::string &group,
                                  const std::string &name);

    static int count();
    static int count(const std::string &group);
    static int groupCount();
    static std::vector<std::string> groups();
    static std::vector<std::string> modules(const std::string &group);
    static std::string description(const std::string &group, const std::string &name);

public:
    enum RequiredFeature {
        NoFeatures                      = 0,
        RangeSelectionFeature           = 1 << 0
    };

    typedef int RequiredFeatures;

protected:
	WorkflowModule();

public:
    virtual ~WorkflowModule();

	QString name() const;
    QString group() const;
    QString description() const;

    // TODO: use instanceId instead
    int32_t uniqueId() const;

    QVariant property(const QString &key) const;
    bool setProperty(const QString &key, const QVariant &value);

    // Called when a property value is changed.
    // The default implementation calls update().
    virtual void doPropertyChanged(const QString & key);

    void addListener(const QString &key, ModuleListener *listener, const QString &target);

    void modulePropertyChanged(WorkflowModule *sender,
        const QString &key, const QString &target) override;

	virtual bool validate() const;
	virtual QWidget *controlWidget();
    virtual void reset();

    // TODO: add output/input methods should not be virtual,
    // since the base class implementation must be called.
    // Replace with "doConfigureOutput()" etc.
    virtual bool addOutput(WorkflowModule *output, const QString &outputType = "Default");
    void removeOutput(WorkflowModule *output);

    virtual bool addInput(WorkflowModule *input,
        const QString &inputType = "Default");
    void removeInput(WorkflowModule *input);

    void detach();

    const QList<WorkflowModule*> &outputModules() const;
    const QList<WorkflowModule*> &inputModules() const;

    QString outputType(WorkflowModule *module) const;
    QString inputType(WorkflowModule *module) const;

    virtual void save(QDomElement &element, QDomDocument &doc);
    virtual void load(QDomElement &element);

    virtual RequiredFeatures requiredFeatures() const;

    virtual void setInputContext(ProcessingContext context, WorkflowModule *previous);

	bool enabled() const;
	void setEnabled(bool enabled);

    bool active() const;
    void setActive(bool active);

    // TODO: fix this. outdated modules should always trigger processing
    // of their next modules.
	bool outdated() const;
	void setOutdated(bool outdated);
	
	virtual void preprocess();
	virtual void process();
	virtual void postprocess();
    
	virtual Frame *processFrame(Frame *frame, int index);

    virtual void configureOutputModule(WorkflowModule *next);

    virtual WorkContext *workContext();
	virtual void doWork(WorkContext *context);

protected:
    struct ListenerTarget 
    {
        ModuleListener *listener;
        QString target;
        ListenerTarget()
            : listener(nullptr)
        {}
    };

public slots:
    void update();

    // Selection Feature
    virtual void setSelectionDimensions(int) {}

private slots:
	virtual void scheduleWork(WorkerThread *thread);

signals:
	void moduleOutdated(WorkflowModule *module);
	void workFinished(WorkflowModule *module);
    void activityChanged(bool active);

    // Selection Feature
    void dimensionSelectionRequested(int, bool);

protected:
    QString m_name;
    QString m_group;
    QMap<QString, QVariant> m_properties;
    uint64_t m_frameIndex;
    ProcessingContext m_inputContext;
    ProcessingContext m_outputContext;
	bool m_enabled;
    bool m_active;
	bool m_outdated;
    bool m_controlDisplayed;
	QList<WorkflowModule*> m_outputModules;
    QMap<WorkflowModule*, QString> m_outputTypes;
	QList<WorkflowModule*> m_inputModules;
    QMap<WorkflowModule*, QString> m_inputTypes;
    QMap<QString, QList<ListenerTarget>> m_listenerMap;
	unsigned int m_activeThreadCount;
};

} // namespace emd

Q_DECLARE_METATYPE(emd::DataGroup *);
Q_DECLARE_METATYPE(const emd::DataGroup *);

#endif
