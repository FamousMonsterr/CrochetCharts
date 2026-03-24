/****************************************************************************\
 Copyright (c) 2026

 This file is part of Crochet Charts.

 Crochet Charts is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Crochet Charts is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Crochet Charts. If not, see <http://www.gnu.org/licenses/>.

 \****************************************************************************/
#include "theme.h"

#include <QApplication>
#include <QDockWidget>
#include <QFile>
#include <QMainWindow>
#include <QPalette>
#include <QTabWidget>
#include <QToolBar>

namespace Theme {

void applyApplicationTheme(QApplication *app)
{
    if(!app)
        return;

    QPalette pal = app->palette();
    pal.setColor(QPalette::Window, QColor("#eef1f5"));
    pal.setColor(QPalette::WindowText, QColor("#1f2937"));
    pal.setColor(QPalette::Base, QColor("#ffffff"));
    pal.setColor(QPalette::AlternateBase, QColor("#f8fafc"));
    pal.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
    pal.setColor(QPalette::ToolTipText, QColor("#1f2937"));
    pal.setColor(QPalette::Text, QColor("#1f2937"));
    pal.setColor(QPalette::Button, QColor("#f8fafc"));
    pal.setColor(QPalette::ButtonText, QColor("#1f2937"));
    pal.setColor(QPalette::BrightText, QColor("#ffffff"));
    pal.setColor(QPalette::Highlight, QColor("#3f6fd8"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    app->setPalette(pal);

    QFile qssFile(":/themes/desktop.qss");
    if(qssFile.open(QIODevice::ReadOnly | QIODevice::Text))
        app->setStyleSheet(QString::fromUtf8(qssFile.readAll()));
}

void polishMainWindow(QMainWindow *window)
{
    if(!window)
        return;

    QList<QToolBar*> toolBars = window->findChildren<QToolBar*>();
    foreach(QToolBar *toolBar, toolBars) {
        toolBar->setIconSize(QSize(18, 18));
        toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

    QList<QDockWidget*> docks = window->findChildren<QDockWidget*>();
    foreach(QDockWidget *dock, docks) {
        dock->setProperty("glassDock", true);
        if(dock->widget())
            dock->widget()->setProperty("panelSurface", true);
    }

    QList<QTabWidget*> tabs = window->findChildren<QTabWidget*>();
    foreach(QTabWidget *tab, tabs)
        tab->setDocumentMode(true);
}

}
