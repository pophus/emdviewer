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

#ifndef EMD_PLUGIN_H
#define EMD_PLUGIN_H

#include <QString>
#include <QtPlugin>

#include "ModuleSource.h"

class QWidget;

namespace emd
{

class Model;
class Workflow;

class Plugin : public ModuleSource
{
public:
	virtual ~Plugin() {}
	
	virtual QString name() = 0;

    void declareModules() override {};

    WorkflowModule *createModule(const std::string & /*group*/,
                                 const std::string & /*name*/) override 
        { return nullptr; }
};

} // namespace emd

Q_DECLARE_INTERFACE(emd::Plugin, "com.mesobine.emd.ViewerPlugin/1.0");

#endif
