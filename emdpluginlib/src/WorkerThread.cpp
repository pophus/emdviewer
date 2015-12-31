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

#include "WorkerThread.h"

#include "WorkContext.h"
#include "WorkflowModule.h"

namespace emd
{

const int kMaxThreads = 1;

bool WorkerThread::s_initialized = false;

std::vector<WorkerThread*> WorkerThread::s_freeThreads;

/******************************** Static Methods ********************************/

void WorkerThread::initialize()
{
    if(!s_initialized)
    {
        for(int iii = 0; iii < kMaxThreads; ++iii)
        {
            WorkerThread *thread = new WorkerThread();
            s_freeThreads.push_back(thread);
        } 

        s_initialized = true;
    }
}

bool WorkerThread::available()
{
    initialize();

    return s_freeThreads.size() > 0;
}

WorkerThread *WorkerThread::takeThread()
{
    initialize();

    if(s_freeThreads.size() > 0)
    {
        WorkerThread *thread = s_freeThreads.back();

        s_freeThreads.pop_back();

        return thread;
    }

    return NULL;
}

void WorkerThread::returnThread(WorkerThread *thread)
{
    s_freeThreads.push_back(thread);
}

/******************************** Instance Methods *******************************/

WorkerThread::WorkerThread(QObject *parent)
	: QThread(parent),
	m_shuttingDown(false),
    m_context(NULL)
{
	m_shuttingDown = false;
}

WorkerThread::~WorkerThread()
{
	m_mutex.lock();
	m_shuttingDown = true;
	m_condition.wakeOne();
	m_mutex.unlock();

	wait();
}

void WorkerThread::work(WorkContext *context)
{
	QMutexLocker locker(&m_mutex);

    if(m_context)
        delete m_context;

    m_context = context;

    if(m_context)
    {
	    if(!isRunning())
		    start(LowPriority);
	    else 
		    m_condition.wakeOne();
    }
}

void WorkerThread::run()
{
	forever
	{
		m_mutex.lock();
		// If there are no frames to process, block until there are
		if(!m_context)
			m_condition.wait(&m_mutex);
		if(m_shuttingDown)
			return;
		
		m_context->module()->doWork(m_context);

        delete m_context;
        m_context = NULL;

		m_mutex.unlock();

		emit(finishedProcessing(this));
	}
}

} // namespace emd
