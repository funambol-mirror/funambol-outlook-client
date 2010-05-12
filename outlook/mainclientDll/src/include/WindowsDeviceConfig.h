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
#ifndef INCL_WINDOWS_DEVICE_CONFIG
#define INCL_WINDOWS_DEVICE_CONFIG

/** @cond DEV */

#include "spds/DeviceConfig.h"
#include "defs.h"

using namespace Funambol;

class WindowsDeviceConfig : public DeviceConfig
{
private:
    bool attach;
    unsigned int logNum;
    unsigned int logSize;

    DeviceConfig & dc;

public:
    WindowsDeviceConfig(const WindowsDeviceConfig& wdc, DeviceConfig & dc);
    WindowsDeviceConfig(const WindowsDeviceConfig& wdc);
    WindowsDeviceConfig(DeviceConfig& dc);
    ~WindowsDeviceConfig();

    void setLogNum(unsigned int v)
    { logNum = min(max(v,MIN_LOG_FILE_SIZE),MAX_LOG_FILE_SIZE);     }
    const unsigned int getLogNum() const
    { return min(max(logNum,MIN_LOG_FILE_NUM),MAX_LOG_FILE_NUM);    }

    void setLogSize(unsigned int v)
    { logSize = min(max(v,MIN_LOG_FILE_SIZE),MAX_LOG_FILE_SIZE);    }
    const unsigned int getLogSize() const
    { return min(max(logSize,MIN_LOG_FILE_SIZE),MAX_LOG_FILE_SIZE); }

    WindowsDeviceConfig& operator = (const WindowsDeviceConfig& dc) {
        assign(dc);
        return *this;
    }

    _declspec(dllexport) void setAttach       (const  bool v)
    { attach = v;    }
    _declspec(dllexport) const bool getAttach()         const
    { return attach; }

    void assign(const WindowsDeviceConfig& s) {
        dc.assign(s.dc);
        attach  = s.attach;
        logNum  = s.logNum;
        logSize = s.logSize;
    }

    DeviceConfig & getCommonConfig() const
    {
        return dc;
    }

    const char*  getMan() const            { return dc.getMan();       }
    void setMan(const char*  v)            { dc.setMan(v);             }

    const char*  getMod() const            { return dc.getMod();       }
    void setMod(const char*  v)            { dc.setMod(v);             }

    const char*  getOem() const            { return dc.getOem();       }
    void setOem(const char*  v)            { dc.setOem(v);             }

    const char*  getFwv() const            { return dc.getFwv();       }
    void setFwv(const char*  v)            { dc.setFwv(v);             }

    const char*  getSwv() const            { return dc.getSwv();       }
    void setSwv(const char*  v)            { dc.setSwv(v);             }

    const char*  getHwv() const            { return dc.getHwv();       }
    void setHwv(const char*  v)            { dc.setHwv(v);             }

    const char*  getDevID() const          { return dc.getDevID();     }
    void setDevID(const char*  v)          { dc.setDevID(v);           }

    const char*  getDevType() const        { return dc.getDevType();   }
    void setDevType(const char*  v)        { dc.setDevType(v);         }

    const char*  getDsV() const            { return dc.getDsV();       }
    void setDsV(const char*  v)            { dc.setDsV(v);             }

    bool getUtc() const                    { return dc.getUtc();       }
    void setUtc(bool v)                    { dc.setUtc(v);             }

    bool getLoSupport() const              { return dc.getLoSupport(); }
    void setLoSupport(bool v)              { dc.setLoSupport(v);       }

    bool getNocSupport() const             { return dc.getNocSupport();}
    void setNocSupport(bool v)             { dc.setNocSupport(v);      }

    LogLevel getLogLevel() const           { return dc.getLogLevel();  }
    void setLogLevel(LogLevel v)           { dc.setLogLevel(v);        }

    unsigned int getMaxObjSize() const     { return dc.getMaxObjSize();}
    void setMaxObjSize(unsigned int v)     { dc.setMaxObjSize(v);      }

    const char*  getDevInfHash() const     { return dc.getDevInfHash();}
    void setDevInfHash(const char *v)      { dc.setDevInfHash(v);      }
};

/** @endcond */

#endif INCL_WINDOWS_DEVICE_CONFIG
