/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef STATE_SERVER_H
#define STATE_SERVER_H

#include "ccnx-wrapper.h"
#include "object-manager.h"
#include "object-db.h"
#include "action-log.h"
#include <set>
#include <map>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include "scheduler.h"

/**
 * @brief Class serving state information from ChronoShare
 *
 * Eventually, the same info/actions can be made available via a global scope prefix
 *
 * Information available:
 *
 * For now serving only locally (using <PREFIX> = /localhost/<user's-device-name>/"chronoshare"/"info")
 *
 * - state: get list of SyncNodes, their sequence numbers, and forwarding hint (almost the same as RECOVERY interest)
 *
 *   <PREFIX_INFO>/"state"/<nonce>   (nonce should probably be the authentification code or authentication code should in addition somewhere)
 *
 * - action
 *
 *   <PREFIX_INFO>/"actions"/"all"/<nonce>/<segment>      get list of all actions
 *   <PREFIX_INFO>/"actions"/"file"/<nonce>/<segment>     get list of actions for a file
 *
 *   Actions are ordered in decreasing order (latest will go first).
 *   Each data packet contains up to 100 actions.  If there are more, they will be segmented. Data packet always
 *   contains a segment number (even if there are less than 100 actions available).
 *
 *   Number of segments is indicated in FinalBlockID of the first data packet (in <PREFIX>/"action"/"all"/<nonce>/%00)
 *
 * - file
 *
 *   <PREFIX_INFO>/"filestate"/"all"/<nonce>/<segment>
 *
 *   Each Data packets contains a list of up to 100 files, the rest is published in other segments.
 *   Number of segments is indicated in FinalBlockID of the first data packet (in <PREFIX>/"file"/"all"/<nonce>/%00).
 *
 * Commands available:
 *
 * For now serving only locally (using <PREFIX_CMD> = /localhost/<user's-device-name>/"chronoshare"/"cmd")
 *
 * - restore version of the file
 *
 *   <PREFIX_CMD>/"restore"/"file"/<one-component-relative-file-name>/<version>/<file-hash>
 *
 * - clean state log
 *   (this may not need to be here, if we implement periodic cleaning)
 * - ? flatten action log (should be supported eventually, but not supported now)
 */
class StateServer
{
public:
  StateServer(Ccnx::CcnxWrapperPtr ccnx, ActionLogPtr actionLog, const boost::filesystem::path &rootDir,
              const Ccnx::Name &userName, const std::string &sharedFolderName, const std::string &appName,
              ObjectManager &objectManager,
              int freshness = -1);
  ~StateServer();

private:
  void
  info_actions_all (const Ccnx::Name &interest);

  void
  info_actions_all_Execute (const Ccnx::Name &interest);

  void
  cmd_restore_file (const Ccnx::Name &interest);

  void
  cmd_restore_file_Execute (const Ccnx::Name &interest);

private:
  void
  registerPrefixes ();

  void
  deregisterPrefixes ();

private:
  Ccnx::CcnxWrapperPtr m_ccnx;
  ActionLogPtr m_actionLog;
  ObjectManager &m_objectManager;

  Ccnx::Name m_PREFIX_INFO;
  Ccnx::Name m_PREFIX_CMD;

  boost::filesystem::path m_rootDir;
  int m_freshness;

  SchedulerPtr     m_scheduler;

  Ccnx::Name  m_userName;
  std::string m_sharedFolderName;
  std::string m_appName;
};
#endif // CONTENT_SERVER_H