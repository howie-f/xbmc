/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GUIDialogPVRRadioRDSInfo.h"

#include "GUIUserMessages.h"
#include "ServiceBroker.h"
#include "guilib/GUIMessage.h"
#include "guilib/GUISpinControl.h"
#include "guilib/GUITextBox.h"
#include "guilib/LocalizeStrings.h"
#include "pvr/PVRManager.h"
#include "pvr/PVRPlaybackState.h"
#include "pvr/channels/PVRChannel.h"
#include "pvr/channels/PVRRadioRDSInfoTag.h"

using namespace PVR;

namespace
{
constexpr unsigned int CONTROL_BTN_OK = 10;
constexpr unsigned int SPIN_CONTROL_INFO = 21;
constexpr unsigned int TEXT_INFO = 22;
constexpr unsigned int CONTROL_INFO_LIST = 70;

constexpr unsigned int INFO_NEWS = 1;
constexpr unsigned int INFO_NEWS_LOCAL = 2;
constexpr unsigned int INFO_SPORT = 3;
constexpr unsigned int INFO_WEATHER = 4;
constexpr unsigned int INFO_LOTTERY = 5;
constexpr unsigned int INFO_STOCK = 6;
constexpr unsigned int INFO_OTHER = 7;
constexpr unsigned int INFO_CINEMA = 8;
constexpr unsigned int INFO_HOROSCOPE = 9;

} // unnamed namespace

CGUIDialogPVRRadioRDSInfo::CGUIDialogPVRRadioRDSInfo()
  : CGUIDialog(WINDOW_DIALOG_PVR_RADIO_RDS_INFO, "DialogPVRRadioRDSInfo.xml")
  , m_InfoNews(29916, INFO_NEWS)
  , m_InfoNewsLocal(29917, INFO_NEWS_LOCAL)
  , m_InfoSport(29918, INFO_SPORT)
  , m_InfoWeather(400, INFO_WEATHER)
  , m_InfoLottery(29919, INFO_LOTTERY)
  , m_InfoStock(29920, INFO_STOCK)
  , m_InfoOther(29921, INFO_OTHER)
  , m_InfoCinema(19602, INFO_CINEMA)
  , m_InfoHoroscope(29922, INFO_HOROSCOPE)
{
}

bool CGUIDialogPVRRadioRDSInfo::OnMessage(CGUIMessage& message)
{
  if (message.GetMessage() == GUI_MSG_CLICKED)
  {
    int iControl = message.GetSenderId();

    if (iControl == CONTROL_BTN_OK)
    {
      Close();
      return true;
    }
    else if (iControl == SPIN_CONTROL_INFO)
    {
      const std::shared_ptr<const CPVRChannel> channel =
          CServiceBroker::GetPVRManager().PlaybackState()->GetPlayingChannel();
      if (!channel)
        return false;

      const std::shared_ptr<const CPVRRadioRDSInfoTag> currentRDS = channel->GetRadioRDSInfoTag();
      if (!currentRDS)
        return false;

      const auto* spin{static_cast<CGUISpinControl*>(GetControl(SPIN_CONTROL_INFO))};
      if (!spin)
        return false;

      auto* textbox{static_cast<CGUITextBox*>(GetControl(TEXT_INFO))};
      if (!textbox)
        return false;

      std::string text;
      switch (spin->GetValue())
      {
        case INFO_NEWS:
          text = currentRDS->GetInfoNews();
          break;
        case INFO_NEWS_LOCAL:
          text = currentRDS->GetInfoNewsLocal();
          break;
        case INFO_SPORT:
          text = currentRDS->GetInfoSport();
          break;
        case INFO_WEATHER:
          text = currentRDS->GetInfoWeather();
          break;
        case INFO_LOTTERY:
          text = currentRDS->GetInfoLottery();
          break;
        case INFO_STOCK:
          text = currentRDS->GetInfoStock();
          break;
        case INFO_OTHER:
          text = currentRDS->GetInfoOther();
          break;
        case INFO_CINEMA:
          text = currentRDS->GetInfoCinema();
          break;
        case INFO_HOROSCOPE:
          text = currentRDS->GetInfoHoroscope();
          break;
        default:
          break;
      }

      if (!text.empty())
        textbox->SetInfo(KODI::GUILIB::GUIINFO::CGUIInfoLabel{text});

      SET_CONTROL_VISIBLE(CONTROL_INFO_LIST);
    }
  }
  else if (message.GetMessage() == GUI_MSG_NOTIFY_ALL)
  {
    if (message.GetParam1() == GUI_MSG_UPDATE_RADIOTEXT && IsActive())
    {
      UpdateInfoControls();
    }
  }

  return CGUIDialog::OnMessage(message);
}

void CGUIDialogPVRRadioRDSInfo::OnInitWindow()
{
  CGUIDialog::OnInitWindow();

  InitInfoControls();
}

void CGUIDialogPVRRadioRDSInfo::InitInfoControls()
{
  SET_CONTROL_HIDDEN(CONTROL_INFO_LIST);

  auto* spin{static_cast<CGUISpinControl*>(GetControl(SPIN_CONTROL_INFO))};
  if (spin)
    spin->Clear();

  auto* textbox{static_cast<CGUITextBox*>(GetControl(TEXT_INFO))};

  m_InfoNews.Init(spin, textbox);
  m_InfoNewsLocal.Init(spin, textbox);
  m_InfoSport.Init(spin, textbox);
  m_InfoWeather.Init(spin, textbox);
  m_InfoLottery.Init(spin, textbox);
  m_InfoStock.Init(spin, textbox);
  m_InfoOther.Init(spin, textbox);
  m_InfoCinema.Init(spin, textbox);
  m_InfoHoroscope.Init(spin, textbox);

  if (spin && textbox)
    UpdateInfoControls();
}

void CGUIDialogPVRRadioRDSInfo::UpdateInfoControls()
{
  const std::shared_ptr<const CPVRChannel> channel =
      CServiceBroker::GetPVRManager().PlaybackState()->GetPlayingChannel();
  if (!channel)
    return;

  const std::shared_ptr<const CPVRRadioRDSInfoTag> currentRDS = channel->GetRadioRDSInfoTag();
  if (!currentRDS)
    return;

  bool bInfoPresent = m_InfoNews.Update(currentRDS->GetInfoNews());
  bInfoPresent |= m_InfoNewsLocal.Update(currentRDS->GetInfoNewsLocal());
  bInfoPresent |= m_InfoSport.Update(currentRDS->GetInfoSport());
  bInfoPresent |= m_InfoWeather.Update(currentRDS->GetInfoWeather());
  bInfoPresent |= m_InfoLottery.Update(currentRDS->GetInfoLottery());
  bInfoPresent |= m_InfoStock.Update(currentRDS->GetInfoStock());
  bInfoPresent |= m_InfoOther.Update(currentRDS->GetInfoOther());
  bInfoPresent |= m_InfoCinema.Update(currentRDS->GetInfoCinema());
  bInfoPresent |= m_InfoHoroscope.Update(currentRDS->GetInfoHoroscope());

  if (bInfoPresent)
    SET_CONTROL_VISIBLE(CONTROL_INFO_LIST);
}

CGUIDialogPVRRadioRDSInfo::InfoControl::InfoControl(uint32_t iSpinLabelId, uint32_t iSpinControlId)
: m_iSpinLabelId(iSpinLabelId),
  m_iSpinControlId(iSpinControlId)
{
}

void CGUIDialogPVRRadioRDSInfo::InfoControl::Init(CGUISpinControl* spin, CGUITextBox* textbox)
{
  m_spinControl = spin;
  m_textbox = textbox;
  m_bSpinLabelPresent = false;
  m_textboxValue.clear();
}

bool CGUIDialogPVRRadioRDSInfo::InfoControl::Update(const std::string& textboxValue)
{
  if (m_spinControl && m_textbox && !textboxValue.empty())
  {
    if (!m_bSpinLabelPresent)
    {
      m_spinControl->AddLabel(g_localizeStrings.Get(m_iSpinLabelId), m_iSpinControlId);
      m_bSpinLabelPresent = true;
    }

    if (m_textboxValue != textboxValue)
    {
      m_spinControl->SetValue(m_iSpinControlId);
      m_textboxValue = textboxValue;
      m_textbox->SetInfo(KODI::GUILIB::GUIINFO::CGUIInfoLabel{textboxValue});
      return true;
    }
  }
  return false;
}
