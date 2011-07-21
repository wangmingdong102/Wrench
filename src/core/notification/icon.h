/****************************************************************************************
* Copyright (c) 2011 Patrick von Reth <patrick.vonreth@gmail.com>                      *
*                                                                                      *
* This program is free software; you can redistribute it and/or modify it under        *
* the terms of the GNU General Public License as published by the Free Software        *
* Foundation; either version 2 of the License, or (at your option) any later           *
* version.                                                                             *
*                                                                                      *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
* PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
*                                                                                      *
* You should have received a copy of the GNU General Public License along with         *
* this program.  If not, see <http://www.gnu.org/licenses/>.                           *
****************************************************************************************/

#ifndef NOTIFICATION_ICON_H
#define NOTIFICATION_ICON_H
#include "../snore_exports.h"


#include <QImage>
#include <QSharedPointer>


class SNORE_EXPORT NotificationIcon
{
public:
	NotificationIcon();
    NotificationIcon(const QImage &img);
	NotificationIcon(const QByteArray &img);
	NotificationIcon(const NotificationIcon &other);

	const QImage &image() const;
	const QString &localUrl() const;
	const QByteArray &imageData() const;
	const bool isLocalFile() const;


private:
	static QHash<QString,QString> hasedImages;
private:
	class NotificationIconData;
	QSharedPointer<NotificationIconData> d;

	const QString &hash() const;




};



#endif // NOTIFICATION_ICON_H
