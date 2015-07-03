#include "snoretoast.h"
#include "libsnore/snore.h"
#include "libsnore/snore_p.h"
#include "libsnore/utils.h"

#include "libsnore/plugins/plugins.h"
#include "libsnore/plugins/snorebackend.h"

#include <QDir>
#include <QApplication>
#include <QSysInfo>

#include <windows.h>

using namespace Snore;

bool SnoreToast::initialize()
{
    if (!SnoreBackend::initialize()) {
        return false;
    }
    if (QSysInfo::windowsVersion() < QSysInfo::WV_WINDOWS8) {
        snoreDebug(SNORE_DEBUG) << "SnoreToast does not work on windows" << QSysInfo::windowsVersion();
        return false;
    }
    return true;
}

bool SnoreToast::canCloseNotification() const
{
    return true;
}

void SnoreToast::slotNotify(Notification notification)
{
    QProcess *p = new QProcess(this);
    p->setReadChannelMode(QProcess::MergedChannels);

    connect(p, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotToastNotificationClosed(int,QProcess::ExitStatus)));
    connect(qApp, SIGNAL(aboutToQuit()), p, SLOT(kill()));

    QStringList arguements;
    arguements << QLatin1String("-t")
               << notification.title()
               << QLatin1String("-m")
               << notification.text();
    if (notification.icon().isValid()) {
        arguements << QLatin1String("-p")
                   << QDir::toNativeSeparators(notification.icon().localUrl());
    }
    arguements << QLatin1String("-w")
               << QLatin1String("-appID")
               << appId(notification.application())
               << QLatin1String("-id")
               << QString::number(notification.id());
    //TODO: could clash with sound backend
    if (notification.hints().value("silent").toBool() || notification.hints().value("sound").isValid()) {
        arguements << QLatin1String("-silent");
    }
    snoreDebug(SNORE_DEBUG) << "SnoreToast" << arguements;
    p->start(QLatin1String("SnoreToast"), arguements);

    p->setProperty("SNORE_NOTIFICATION", QVariant::fromValue(notification));
}

void SnoreToast::slotRegisterApplication(const Application &application)
{
    if (!application.constHints().contains("windows_app_id")) {
        QProcess *p = new QProcess(this);
        p->setReadChannelMode(QProcess::MergedChannels);

        QStringList arguements;
        arguements << QLatin1String("-install")
                   << QLatin1String("SnoreNotify\\") + qApp->applicationName()
                   << QDir::toNativeSeparators(qApp->applicationFilePath())
                   << appId(application);
        snoreDebug(SNORE_DEBUG) << "SnoreToast" << arguements;
        p->start(QLatin1String("SnoreToast"), arguements);

        connect(p, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotPrintExitStatus(int,QProcess::ExitStatus)));
        connect(qApp, SIGNAL(aboutToQuit()), p, SLOT(kill()));
    }
}

void SnoreToast::slotCloseNotification(Notification notification)
{
    QProcess *p = new QProcess(this);
    p->setReadChannelMode(QProcess::MergedChannels);

    QStringList arguements;
    arguements << QLatin1String("-close")
               << QString::number(notification.id());
    snoreDebug(SNORE_DEBUG) << "SnoreToast" << arguements;
    p->start(QLatin1String("SnoreToast"), arguements);

    connect(p, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotPrintExitStatus(int,QProcess::ExitStatus)));
    connect(qApp, SIGNAL(aboutToQuit()), p, SLOT(kill()));
}

void SnoreToast::slotToastNotificationClosed(int code, QProcess::ExitStatus)
{
    QProcess *p = qobject_cast<QProcess *>(sender());
    snoreDebug(SNORE_DEBUG) << p->readAll();
    snoreDebug(SNORE_DEBUG) << "Exit code:" << code;

    Notification n = p->property("SNORE_NOTIFICATION").value<Notification>();
    Notification::CloseReasons reason = Notification::NONE;

    switch (code) {
    case 0:
        reason = Notification::ACTIVATED;
        slotNotificationActionInvoked(n);
        break;
    case 1:
        //hidden;
        break;
    case 2:
        reason = Notification::DISMISSED;
        break;
    case 3:
        reason = Notification::TIMED_OUT;
        break;
    case -1:
        //failed
        snoreDebug(SNORE_WARNING) << "SnoreToast failed to display " << n;
        break;
    }

    closeNotification(n, reason);
    p->deleteLater();
}

void SnoreToast::slotPrintExitStatus(int, QProcess::ExitStatus)
{
    QProcess *p = qobject_cast<QProcess *>(sender());
    snoreDebug(SNORE_DEBUG) << p->readAll();
    p->deleteLater();
}

QString SnoreToast::appId(const Application &application)
{

    QString appID = application.constHints().value("windows-app-id").toString();
    if (appID.isEmpty()) {
        appID = application.constHints().value("windows_app_id").toString();
    }
    if (appID.isEmpty()) {
        appID = QString(qApp->organizationName() + QLatin1Char('.') + qApp->applicationName() + QLatin1String(".SnoreToast")).remove(QLatin1Char(' '));
    }
    return appID;
}
