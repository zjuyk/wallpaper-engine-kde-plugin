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

#ifndef WE_PROJECTLOADER
#define WE_PROJECTLOADER

#include <QLibrary>
#include <QJsonDocument>

#include "viewModel.h"

class WEProject : public QObject
{
	Q_OBJECT
		Q_PROPERTY(QList<ViewModel *> model READ model NOTIFY modelChanged)
		Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

	public:
		WEProject(QObject *parent=0);
		~WEProject() override;
		QList<ViewModel *> model();
		QString url();
		void setUrl(const QString &url);

signals:
		void modelChanged();
		void urlChanged();
	private:
		QString m_url = "";
		QList<ViewModel *> m_model = {};
		void updateModel();
};

#endif
