/****************************************************************************\
 Copyright (c) 2011-2014 Stitch Works Software
 Brian C. Milco <bcmilco@gmail.com>

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
#include "splashscreen.h"

#include <QPainter>
#include <QBitmap>
#include "appinfo.h"

#include <QDebug>

SplashScreen::SplashScreen()
{
    QPixmap p(":/images/splashscreen.png");
    this->setPixmap(p);
    this->setMask(p.mask());
}

void SplashScreen::drawContents (QPainter* painter)
{
    const QRect rect(QPoint(0,0), this->pixmap().size());
    painter->drawPixmap(rect, this->pixmap());
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    const QRect panelRect(28, rect.height() - 88, rect.width() - 56, 54);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(255, 255, 255, 222));
    painter->drawRoundedRect(panelRect, 10, 10);

    QFont versionFont = painter->font();
    versionFont.setPointSize(versionFont.pointSize() + 1);
    painter->setFont(versionFont);
    painter->setPen(QColor(58, 66, 82));
    const QString version = tr("Version: %1").arg(AppInfo::inst()->appVersionShort);
    painter->drawText(panelRect.adjusted(14, 6, -14, -26), Qt::AlignLeft | Qt::AlignVCenter, version);

    QFont messageFont = versionFont;
    messageFont.setBold(true);
    painter->setFont(messageFont);
    painter->setPen(QColor(24, 31, 45));
    painter->drawText(panelRect.adjusted(14, 22, -14, -4), Qt::AlignLeft | Qt::AlignVCenter, mMessage);
}

void SplashScreen::showMessage (const QString &message)
{
    if(message != mMessage) {
        mMessage = message;
        this->update();
    }
}
