/****************************************************************************\
 Copyright (c) 2010-2014 Stitch Works Software
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
#include "application.h"
#include "mainwindow.h"
#include "appinfo.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QTranslator>

#include "settings.h"

#include "splashscreen.h"
#include "theme.h"
#include "updatefunctions.h"

#include "errorhandler.h"

#ifdef Q_OS_MAC
#include <mach-o/dyld.h>
#include <unistd.h>
#endif

#ifdef Q_OS_MAC
namespace {

QString currentExecutablePath()
{
    uint32_t size = 0;
    _NSGetExecutablePath(0, &size);
    QByteArray buffer;
    buffer.resize(static_cast<int>(size));
    if(_NSGetExecutablePath(buffer.data(), &size) != 0)
        return QString();

    return QFileInfo(QString::fromLocal8Bit(buffer.constData())).canonicalFilePath();
}

void sanitizeMacQtRuntime(char *argv[])
{
    const bool alreadySanitized = qEnvironmentVariableIsSet("CROCHETCHARTS_QT_ENV_SANITIZED");

    const char *dyldVars[] = {
        "DYLD_FRAMEWORK_PATH",
        "DYLD_LIBRARY_PATH",
        "DYLD_INSERT_LIBRARIES",
        "DYLD_FALLBACK_FRAMEWORK_PATH",
        "DYLD_FALLBACK_LIBRARY_PATH"
    };

    const char *qtVars[] = {
        "QT_PLUGIN_PATH",
        "QT_QPA_PLATFORM_PLUGIN_PATH"
    };

    bool needsRelaunch = false;
    for(size_t i = 0; i < sizeof(dyldVars) / sizeof(dyldVars[0]); ++i) {
        if(qEnvironmentVariableIsSet(dyldVars[i]))
            needsRelaunch = true;
        qunsetenv(dyldVars[i]);
    }

    for(size_t i = 0; i < sizeof(qtVars) / sizeof(qtVars[0]); ++i)
        qunsetenv(qtVars[i]);

    const QString executablePath = currentExecutablePath();
    QFileInfo executableInfo(executablePath);
    QDir macOsDir = executableInfo.dir();
    if(macOsDir.dirName() == "MacOS") {
        QDir contentsDir = macOsDir;
        if(contentsDir.cdUp() && contentsDir.dirName() == "Contents") {
            const QString pluginsPath = contentsDir.absoluteFilePath("PlugIns");
            const QString platformPluginsPath = pluginsPath + "/platforms";
            qputenv("QT_PLUGIN_PATH", QFile::encodeName(pluginsPath));
            qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", QFile::encodeName(platformPluginsPath));
        }
    }

    if(!needsRelaunch || alreadySanitized)
        return;

    qputenv("CROCHETCHARTS_QT_ENV_SANITIZED", "1");
    execv(executablePath.toLocal8Bit().constData(), argv);
    fprintf(stderr, "Failed to relaunch CrochetCharts with a sanitized Qt runtime environment.\n");
}

} // namespace
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_MAC
    sanitizeMacQtRuntime(argv);
#endif
    qInstallMessageHandler(errorHandler);
    Application a(argc, argv);

    Q_INIT_RESOURCE(crochet);
    Q_INIT_RESOURCE(translations);
    Theme::applyApplicationTheme(&a);

    QTranslator translator;
    QByteArray languageOverride = qgetenv("CROCHETCHARTS_LANG").toLower();
    QString languageSetting = Settings::inst()->value("uiLanguage").toString().toLower();
    bool useRussian = false;

    if(languageOverride == "ru" || languageOverride == "russian")
        useRussian = true;
    else if(languageOverride == "en" || languageOverride == "english")
        useRussian = false;
    else if(languageSetting == "ru" || languageSetting == "russian")
        useRussian = true;
    else if(languageSetting == "en" || languageSetting == "english")
        useRussian = false;
    else
        useRussian = (QLocale::system().language() == QLocale::Russian);

    if(useRussian && translator.load(":/translations/crochetcharts_ru.qm"))
        a.installTranslator(&translator);

    SplashScreen splash;
    splash.show();
    splash.showMessage(QObject::tr("Loading stitch library..."));
    qApp->processEvents();

    a.loadStitchLibrary();

    QStringList arguments = QCoreApplication::arguments();
    arguments.removeFirst(); // remove the application name from the list.

    splash.showMessage(QObject::tr("Preparing main window..."));
    qApp->processEvents();

    MainWindow w(arguments);
    a.setMainWindow(&w);
   
    QString curVersion = AppInfo::inst()->appVersion;
    QString lastUsed = Settings::inst()->value("lastUsed").toString();
    updateFunction(lastUsed);
    Settings::inst()->setValue("version", curVersion);

    splash.showMessage(QObject::tr("Opening workspace..."));
    qApp->processEvents();

    w.showMaximized();
    splash.finish(&w);
    return a.exec();
}
