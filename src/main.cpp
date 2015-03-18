
#include <libsnore/snore.h>
#include <libsnore/notification/notification.h>
#include <libsnore/log.h>
#include <libsnore/version.h>

#include <QApplication>
#include <QCommandLineParser>

#include <iostream>

using namespace Snore;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("snore-send");
    app.setOrganizationName("snore-send");
    app.setApplicationVersion(Snore::Version::version());

    QCommandLineParser parser;
    parser.setApplicationDescription("A command line interface for Snorenotify.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption title(QStringList() << "t" << "title", "Set the notification title.", "title");
    parser.addOption(title);

    QCommandLineOption message(QStringList() << "m" << "message", "Set the notification body.", "message");
    parser.addOption(message);

    QCommandLineOption applicationName(QStringList() << "a" << "application", "Set the notification applicattion.", "application", app.applicationName());
    parser.addOption(applicationName);

    QCommandLineOption alertName(QStringList() << "c" << "alert", "Set the notification alert class.", "alert", "Default");
    parser.addOption(alertName);

    QCommandLineOption iconPath(QStringList() << "i" << "icon", "Set the notification icon.", "icon", ":/root/snore.png");
    parser.addOption(iconPath);

    QCommandLineOption silent(QStringList() << "silent", "Don't print to stdout.");
    parser.addOption(silent);

    parser.process(app);
    if (parser.isSet(title) && parser.isSet(message)) {
        SnoreCore &core = SnoreCore::instance();

        core.loadPlugins(SnorePlugin::BACKEND | SnorePlugin::SECONDARY_BACKEND);

        Icon icon(parser.value(iconPath));
        Application application(parser.value(applicationName), icon);
        Alert alert(parser.value(alertName), icon);
        application.addAlert(alert);

        core.registerApplication(application);

        Notification n(application, alert, parser.value(title), parser.value(message), icon);

        core.broadcastNotification(n);
        int returnCode = -1;

        app.connect(&core, &SnoreCore::notificationClosed, [&](Notification noti) {
            if (!parser.isSet(silent)) {
                QString reason;
                QDebug(&reason) << noti.closeReason();
                std::cout << qPrintable(reason) << std::endl;
            }
            returnCode = noti.closeReason();
        });
        app.connect(&core, &SnoreCore::notificationClosed, &app, &QApplication::quit);
        app.exec();
        return returnCode;
    }
    parser.showHelp(1);
}

