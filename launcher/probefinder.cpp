/*
  probefinder.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2010-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  Licensees holding valid commercial KDAB GammaRay licenses may use this file in
  accordance with GammaRay Commercial License Agreement provided with the Software.

  Contact info@kdab.com if any conditions of this licensing are not clear to you.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config-gammaray.h"
#include "probefinder.h"
#include "probeabi.h"

#include <common/paths.h>

#include <qglobal.h>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QString>
#include <QStringBuilder>

#include <algorithm>

namespace GammaRay {

namespace ProbeFinder {

QString findProbe(const QString &baseName, const ProbeABI &probeAbi)
{
  const QString probePath =
    Paths::probePath(probeAbi.id()) %
    QDir::separator() %
    baseName %
    Paths::libraryExtension();

  const QFileInfo fi(probePath);
  const QString canonicalPath = fi.canonicalFilePath();
  if (!fi.isFile() || !fi.isReadable() || canonicalPath.isEmpty()) {
    qWarning() << "Cannot locate probe" << probePath;
    qWarning() << "This is likely a setup problem, due to an incomplete or partially moved installation.";
    return QString();
  }

  return canonicalPath;
}

ProbeABI findBestMatchingABI(const ProbeABI& targetABI)
{
  return findBestMatchingABI(targetABI, listProbeABIs());
}

ProbeABI findBestMatchingABI(const ProbeABI &targetABI, const QVector<ProbeABI> &availableABIs)
{
  QVector<ProbeABI> compatABIs;
  foreach (const ProbeABI &abi, availableABIs) {
    if (targetABI.isCompatible(abi))
      compatABIs.push_back(abi);
  }

  if (compatABIs.isEmpty())
    return ProbeABI();

  std::sort(compatABIs.begin(), compatABIs.end());
  return compatABIs.last();
}

QVector<ProbeABI> listProbeABIs()
{
  QVector<ProbeABI> abis;
  const QDir dir(Paths::probePath(QString()));
#if defined(GAMMARAY_INSTALL_QT_LAYOUT)
  const QString filter = QString::fromLatin1("%1*").arg(GAMMARAY_PROBE_NAME);
  foreach (const QFileInfo &abiId, dir.entryInfoList(QStringList(filter), QDir::Files)) {
    if (abiId.isSymLink() || !QLibrary::isLibrary(abiId.fileName()))
      continue;
    const QString baseNameSection = abiId.completeBaseName().section('-', 1);
    QString baseName = GAMMARAY_PROBE_ABI;

    if (!baseNameSection.isEmpty()) {
        baseName.append("-");
        baseName.append(baseNameSection);
    }

    const ProbeABI abi = ProbeABI::fromString(baseName);
    if (abi.isValid())
      abis.push_back(abi);
  }
#else
  foreach (const QString &abiId, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    const ProbeABI abi = ProbeABI::fromString(abiId);
    if (abi.isValid())
      abis.push_back(abi);
  }
#endif
  return abis;
}

}

}
