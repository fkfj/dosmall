#include <QtCore/QSettings>
#include "global.h"

QString Global::globalConfigPathVar = QDir::homePath() + "/.dosmall";
QString Global::projectHomeVar = QString();
QString Global::projectConfigTemplateVar = globalConfigPathVar + "/cproject.template";
QString Global::historyProjectFileVar = globalConfigPathVar + "/historyproject";
ProjectManager* Global::projectManagerVar = 0;
