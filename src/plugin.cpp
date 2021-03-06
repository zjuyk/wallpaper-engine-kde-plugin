/*
 *  Copyright 2020 catsout  <outl941@163.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

#include <QQmlExtensionPlugin>
#include <QQmlEngine>
#include "weProjectLoader.h"
#include "mpvbackend.h"
#include "SceneViewer.h"

class Port : public QQmlExtensionPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri)
    {  
		if(strcmp(uri, "com.github.catsout.wallpaperEngineKde") != 0)
			return;
		qmlRegisterType<WEProject>(uri, 1, 0, "WEProject");
		qmlRegisterType<SceneViewer>(uri, 1, 0, "SceneViewer");
		std::setlocale(LC_NUMERIC, "C");
		qmlRegisterType<MpvObject>(uri, 1, 0, "MpvObject");
    }   
};

#include "plugin.moc"
