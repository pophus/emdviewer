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

#include "FileHelper.h"

namespace emd
{

static FileHelper::DuplicateFilePolicy s_duplicateFilePolicy = FileHelper::DuplicateFilePolicy::Undecided;

bool FileHelper::checkFileOverwrite(QString &path, bool &abort)
{
    bool fileExists = false;

     QFileInfo fileInfo(path);
    if(fileInfo.exists())
    {
        DuplicateFilePolicy duplicateFilePolicy = s_duplicateFilePolicy;

        if(s_duplicateFilePolicy == DuplicateFilePolicy::Undecided)
        {
            QDialog *conflictDialog = new QDialog();
            conflictDialog->setWindowTitle("File Conflict");
            conflictDialog->setModal(true);

            QLabel *messageLabel = new QLabel("A file already exists at path\n" + path + "\nSelect an option to resolve the conflict:");

            QPushButton *cancelButton = new QPushButton("Cancel");
            cancelButton->setCheckable(true);
            QObject::connect(cancelButton, SIGNAL(clicked()),
                conflictDialog, SLOT(accept()));

            QPushButton *overwriteButton = new QPushButton("Overwrite");
            overwriteButton->setCheckable(true);
            QObject::connect(overwriteButton, SIGNAL(clicked()),
                conflictDialog, SLOT(accept()));

            QPushButton *skipButton = new QPushButton("Skip");
            skipButton->setCheckable(true);
            QObject::connect(skipButton, SIGNAL(clicked()),
                conflictDialog, SLOT(accept()));

            QPushButton *renameButton = new QPushButton("Rename");
            renameButton->setCheckable(true);
            QObject::connect(renameButton, SIGNAL(clicked()),
                conflictDialog, SLOT(accept()));

            QButtonGroup *buttonGroup = new QButtonGroup();
            buttonGroup->setExclusive(true);
            buttonGroup->addButton(cancelButton, (int)DuplicateFilePolicy::Undecided);
            buttonGroup->addButton(overwriteButton, (int)DuplicateFilePolicy::Overwrite);
            buttonGroup->addButton(skipButton, (int)DuplicateFilePolicy::Skip);
            buttonGroup->addButton(renameButton, (int)DuplicateFilePolicy::Rename);

            QHBoxLayout *buttonLayout = new QHBoxLayout();
            buttonLayout->addWidget(cancelButton);
            buttonLayout->addWidget(overwriteButton);
            buttonLayout->addWidget(skipButton);
            buttonLayout->addWidget(renameButton);

            QCheckBox *rememberBox = new QCheckBox("Remember this choice for other conflicts in this export operation");
            rememberBox->setChecked(true);

            QVBoxLayout *layout = new QVBoxLayout();
            layout->addWidget(messageLabel);
            layout->addLayout(buttonLayout);
            layout->addWidget(rememberBox);
            conflictDialog->setLayout(layout);

            conflictDialog->exec();

            switch(buttonGroup->checkedId())
            {
            case static_cast<int>(DuplicateFilePolicy::Undecided):
                abort = true;
                fileExists = true;
                break;
            case static_cast<int>(DuplicateFilePolicy::Overwrite):
                duplicateFilePolicy = DuplicateFilePolicy::Overwrite;
                break;
            case static_cast<int>(DuplicateFilePolicy::Skip):
                duplicateFilePolicy = DuplicateFilePolicy::Skip;
                break;
            case static_cast<int>(DuplicateFilePolicy::Rename):
                duplicateFilePolicy = DuplicateFilePolicy::Rename;
                break;
            }

            if(rememberBox->isChecked())
                s_duplicateFilePolicy = duplicateFilePolicy;

            delete conflictDialog;
        }

        switch(duplicateFilePolicy)
        {
        case DuplicateFilePolicy::Overwrite:
            // Do nothing, the file will be saved.
            break;
        case DuplicateFilePolicy::Skip:
            // Skip this file.
            fileExists = true;
            break;
        case DuplicateFilePolicy::Rename:
            // Change the name and save the file.
            QFileInfo pathInfo(path);
            QString stem(pathInfo.path() %"/" % pathInfo.completeBaseName() % " (%1)" % "." % pathInfo.suffix());
            int count = 0;
            QFileInfo info;
            do
            {
                info = QFileInfo(stem.arg(++count));
            } while(info.exists());

            path = stem.arg(count);

            break;
        }
    }

    return fileExists;
}

}