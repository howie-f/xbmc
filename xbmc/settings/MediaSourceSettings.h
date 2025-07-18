/*
 *  Copyright (C) 2013-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "MediaSource.h"
#include "settings/lib/ISettingsHandler.h"

#include <string>
#include <string_view>

class CProfileManager;

namespace tinyxml2
{
class XMLNode;
}

class CMediaSourceSettings : public ISettingsHandler
{
public:
  static CMediaSourceSettings& GetInstance();

  static std::string GetSourcesFile();

  void OnSettingsLoaded() override;
  void OnSettingsUnloaded() override;

  bool Load();
  bool Load(const std::string &file);
  bool Save() const;
  bool Save(const std::string &file) const;
  void Clear();

  std::vector<CMediaSource>* GetSources(std::string_view type);
  const std::string& GetDefaultSource(std::string_view type) const;
  void SetDefaultSource(std::string_view type, std::string_view source);

  bool UpdateSource(std::string_view strType,
                    std::string_view strOldName,
                    std::string_view strUpdateChild,
                    const std::string& strUpdateValue);
  bool DeleteSource(std::string_view strType,
                    std::string_view strName,
                    std::string_view strPath,
                    bool virtualSource = false);
  bool AddShare(std::string_view type, const CMediaSource& share);
  bool UpdateShare(std::string_view type, std::string_view oldName, const CMediaSource& share);

protected:
  CMediaSourceSettings();
  CMediaSourceSettings(const CMediaSourceSettings&) = delete;
  CMediaSourceSettings& operator=(CMediaSourceSettings const&) = delete;
  ~CMediaSourceSettings() override;

private:
  bool GetSource(const std::string& category,
                 const tinyxml2::XMLNode* source,
                 CMediaSource& share) const;
  void GetSources(const tinyxml2::XMLNode* rootElement,
                  const std::string& tagName,
                  std::vector<CMediaSource>& items,
                  std::string& defaultString) const;
  bool SetSources(tinyxml2::XMLNode* rootNode,
                  const char* section,
                  const std::vector<CMediaSource>& shares,
                  const std::string& defaultPath) const;

  std::vector<CMediaSource> m_programSources;
  std::vector<CMediaSource> m_pictureSources;
  std::vector<CMediaSource> m_fileSources;
  std::vector<CMediaSource> m_musicSources;
  std::vector<CMediaSource> m_videoSources;
  std::vector<CMediaSource> m_gameSources;

  std::string m_defaultProgramSource;
  std::string m_defaultMusicSource;
  std::string m_defaultPictureSource;
  std::string m_defaultFileSource;
};
