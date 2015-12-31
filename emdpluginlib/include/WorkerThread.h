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

#ifndef EMD_WORKERTHREAD_H
#define EMD_WORKERTHREAD_H

#include "EmdPluginLib.h"

#include <vector>

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

namespace emd
{

class WorkContext;

class EMDPLUGIN_API WorkerThread : public QThread
{
	Q_OBJECT

public: 
	WorkerThread(QObject *parent = 0);
	~WorkerThread();

	void work(WorkContext *context);

    static bool available();
    static WorkerThread *takeThread();
    static void returnThread(WorkerThread *thread);

protected:
	void run();

private:
    static void initialize();

signals:
	void finishedProcessing(WorkerThread *);

private:
    WorkContext *m_context;

	bool m_shuttingDown;
	QMutex m_mutex;
	QWaitCondition m_condition;

    static bool s_initialized;
    static std::vector<WorkerThread*> s_freeThreads;
};

} // namespace emd

#endif