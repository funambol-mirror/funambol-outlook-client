/*
 * Funambol is a mobile platform developed by Funambol, Inc. 
 * Copyright (C) 2003 - 2007 Funambol, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License version 3 as published by
 * the Free Software Foundation with the addition of the following permission 
 * added to Section 15 as permitted in Section 7(a): FOR ANY PART OF THE COVERED
 * WORK IN WHICH THE COPYRIGHT IS OWNED BY FUNAMBOL, FUNAMBOL DISCLAIMS THE 
 * WARRANTY OF NON INFRINGEMENT  OF THIRD PARTY RIGHTS.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Affero General Public License 
 * along with this program; if not, see http://www.gnu.org/licenses or write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 * 
 * You can contact Funambol, Inc. headquarters at 643 Bair Island Road, Suite 
 * 305, Redwood City, CA 94063, USA, or at email address info@funambol.com.
 * 
 * The interactive user interfaces in modified source and object code versions
 * of this program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU Affero General Public License version 3.
 * 
 * In accordance with Section 7(b) of the GNU Affero General Public License
 * version 3, these Appropriate Legal Notices must retain the display of the
 * "Powered by Funambol" logo. If the display of the logo is not reasonably 
 * feasible for technical reasons, the Appropriate Legal Notices must display
 * the words "Powered by Funambol".
 */

#ifndef INCL_DEF_WIN_CONFIG
#define INCL_DEF_WIN_CONFIG

/** @cond OLPLUGIN */
/** @addtogroup ClientDLL */
/** @{ */

#include "base/fscapi.h"
#include "base/Log.h"
#include "spds/AccessConfig.h"
#include "spds/DeviceConfig.h"
#include "spds/SyncSourceConfig.h"
#include "spds/SyncManagerConfig.h"
#include "spds/DefaultConfigFactory.h"

#include <string>

/**
 * This class is a factory of default configurations.
 * It can be inherited to define client specific Config parameters.
 */
class DefaultWinConfigFactory : public DefaultConfigFactory {

    public:

        DefaultWinConfigFactory();
        ~DefaultWinConfigFactory();

        /**
         * Returns a default generated AccessConfig for Win32.
         * @return  AccessConfig pointer allocated new, so it must
         *          be freed by the caller.
         */
        static AccessConfig*  getAccessConfig();

        /**
         * Returns a default generated DeviceConfig for Win32.
         * @return  DeviceConfig pointer allocated new, so it must
         *          be freed by the caller.
         */
        static DeviceConfig*  getDeviceConfig();

        /**
         * Returns a default generated SyncSourceConfig for Win32 (common props of SyncSourceConfig).
         * @param name : the name of syncsource desired
         * @return       WindowsSyncSourceConfig pointer allocated new, so it must
         *               be freed by the caller.
         */
        static SyncSourceConfig* getSyncSourceConfig(const std::wstring& name);

        /**
         * Returns a default generated WindowsSyncSourceConfig for Win32 (specific client props).
         * @param name : the name of syncsource desired
         * @param sc   : pointer to the SyncSourceConfig object where all common properties are stored (mandatory not NULL)
         * @return       WindowsSyncSourceConfig pointer allocated new, so it must
         *               be freed by the caller.
         */
        static WindowsSyncSourceConfig* getWinSyncSourceConfig(const std::wstring& name, SyncSourceConfig* sc);
};

/** @} */
/** @endcond */
#endif
