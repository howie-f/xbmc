/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "PlatformAndroid.h"

#include "filesystem/SpecialProtocol.h"
#include "windowing/android/WinSystemAndroidGLESContext.h"

#include "platform/android/powermanagement/AndroidPowerSyscall.h"

#include <stdlib.h>

#include "utils/log.h"

CPlatform* CPlatform::CreateInstance()
{
  return new CPlatformAndroid();
}

bool CPlatformAndroid::Init()
{
  CLog::Log(LOGINFO, "FHW: start CPlatformAndroid::Init()");
  if (!CPlatformPosix::Init())
    return false;
  setenv("SSL_CERT_FILE", CSpecialProtocol::TranslatePath("special://xbmc/system/certs/cacert.pem").c_str(), 1);

  setenv("OS", "Linux", true); // for python scripts that check the OS

  CWinSystemAndroidGLESContext::Register();

  CAndroidPowerSyscall::Register();

  CLog::Log(LOGINFO, "FHW: exit CPlatformAndroid::Init()");
  return true;
}
