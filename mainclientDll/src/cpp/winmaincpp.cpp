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

#include "base/fscapi.h"
#include "base/Log.h"
#include "base/util/utils.h"
#include "base/memTracker.h"

#include "winmaincpp.h"
#include "WindowsSyncSource.h"
#include "WindowsSyncClient.h"
#include "utils.h"
#include "HwndFunctions.h"

#include "outlook/ClientApplication.h"
#include "outlook/ClientException.h"
#include "SyncException.h"

// Listeners
#include "event/SetListener.h"
#include "event/ManageListener.h"
#include "event/OutlookSyncListener.h"
#include "event/OutlookSyncSourceListener.h"
#include "event/OutlookSyncStatusListener.h"
#include "event/OutlookSyncItemListener.h"
#include "event/OutlookTransportListener.h"

#include <string>
using namespace std;


HANDLE syncMutex  = NULL;
HANDLE syncThread = NULL;

bool isScheduledSync = false;
bool resetLog        = false;


/**
 * Returns a pointer to the OutlookConfig object (singleton).
 * It's used to access the whole configuration object from UI, to get/set plugin properties.
 * If configuration never instantiated, we need to initialize client first.
 * (config could not exist, log must be opened)
 */
__declspec(dllexport) OutlookConfig* getConfig() {

    if (!OutlookConfig::isInstantiated()) {
        initializeClient(false);
    }
    return OutlookConfig::getInstance();
}




/**
 * Initialize the client: open configuration, open LOG.
 * Configuration is a singleton object that MUST remain active during all 
 * program life. Will be released with close() method.
 *
 * @param isScheduled true if it's a scheduled sync
 * @return            0 if no errors
 */
int initializeClient(bool isScheduled) {

    int ret=0;
    char logText[512];
    logText[0] = 0;

    //
    // Initialize configuration.
    // -------------------------
    OutlookConfig* config = OutlookConfig::getInstance();
    //
    // 1. Generate configuration if error reading (may not exist if first time).
    //    Save, to ensure future calls to config.read().
    if (!config->read()) {
        config->createDefaultConfig();
        config->save();
        sprintf(logText, INFO_CONFIG_GENERATED);
    }
    //
    // 2. Check if sw version has changed (client may have been upgraded).
    //    If config upgraded, save it to disk.
    if (config->checkToUpgrade()) {
        config->upgradeConfig();
        config->save();

        // Execute actions to upgrade the plugin from old version.
        upgradePlugin(config->getOldSwv());
        sprintf(logText, INFO_SWV_UPGRADED, config->getClientConfig().getSwv());
    }

    // Simply save to public var.
    isScheduledSync = isScheduled;


    //
    // Initialize LOG.
    // ---------------
    if (initLog(isScheduled)) {
        safeMessageBox(ERR_INIT_LOG);
        ret = 1;
    }

    HwndFunctions::initHwnd();


    if (strlen(logText) > 0) {
        LOG.info(logText);
    }
    return ret;
}


/**
 * Initialize the LOG. Log File is placed under 'app data' directory for 
 * current user, if directories not found they will be created.
 * Log file is reset and set level with this call.
 * @param reset  if scheduled, we don't reset the log
 * @return       0 if no errors
 */
int initLog(bool isScheduled) {

    // Log path: get from config (under app data)
    OutlookConfig* config = OutlookConfig::getInstance();
    string logPath = config->getLogDir();
    logPath += "\\";
    logPath += OL_PLUGIN_LOG_NAME;

    // If first time, dirs don't exist under app data!
    if (writeToFile("\n", logPath, "a")) {
        if (makeDataDirs()) {               // <- warning: this assume we place the log under 'app data'
            safeMessageBox(getLastErrorMsg());
            return 1;
        }
    }

    //Log(0, config->getLogDir(), OL_PLUGIN_LOG_NAME);
    LOG.setLogPath(config->getLogDir());
    LOG.setLogName(OL_PLUGIN_LOG_NAME);
    LOG.setLevel(config->getClientConfig().getLogLevel());

    string title = "Outlook Sync Client opened";
    if (!isScheduled) {
        resetLog = true;    // Will reset when first sync starts
    }
    else {
        resetLog = false;   // Don't reset log on sched sync
        title += " *** Scheduled sync started ***";
    }
    LOG.info(title.c_str());

    return 0;
}




/**
 ***************************************************
 * Entry point to start the synchronization process.
 ***************************************************
 *
 * @return   0  OK, no errors.
 *           1  generic error.
 *           2  aborted by user (soft termination).
 *           3  Outlook fatal exception.
 *           4  Thread terminated (hard termination).
 *           5  aborted by user to avoid full-sync.
 */
int startSync() {

    int ret           = 0;
    int sourcesActive = 0;
    int priority      = 0;
    WCHAR* wname      = NULL;
    SyncReport* report= NULL;
    string mutexName  = "";

    // Open current configuration: call initialize(0) if not called yet!
    // (reset abortSync flag)
    OutlookConfig* config = getConfig();
    config->setAbortSync(false);  

    // Check if log size is too big (>10MB).
    if (!resetLog && (LOG.getLogSize() > MAX_LOG_SIZE)) {
        resetLog = true;
    }

    // Reset log (only the first sync)
    if (resetLog) {
        string title = PROGRAM_NAME;
        title += " v. ";
        title += config->getClientConfig().getSwv();
        title += " - LOG file";
        if (isScheduledSync) title += " (scheduled sync)";
        LOG.reset(title.c_str());
        resetLog = false;
    }

    // Update log level: could be changed from initialize().
    LOG.setLevel(config->getClientConfig().getLogLevel());
    LOG.debug("Starting the Sync process...");

    // Manually disable notes if it's a portal build.
    // new 7.0.3: changed portal meaning
    //if (config->checkPortalBuild()) {
    //    config->getSyncSourceConfig(NOTE_)->setSync("none");
    //}

    // If here, this is the ONLY instance of sync process 
    // -> set the scheduled flag on win registry.
    config->setScheduledSync(isScheduledSync);


    // Reads timeStamps from registry -> update the config. 
    // (a scheduled sync could have completed with the UI open)
    config->readSourcesTimestamps();

    //
    // Set a low priority for this thread: different if normal/scheduled sync.
    //
    LOG.debug("Set a lower priority to the process");
    syncThread = GetCurrentThread();
    if (isScheduledSync)  priority = THREAD_PRIORITY_LOWEST;
    else                  priority = THREAD_PRIORITY_BELOW_NORMAL;
    if(!SetThreadPriority(syncThread, priority)) {
        DWORD code = GetLastError();
        char* msg = readSystemErrorMsg(code);
        setErrorF(getLastErrorCode(), ERR_THREAD_PRIORITY, code, msg);
        delete [] msg;
        LOG.error(getLastErrorMsg());
        return 1;
    }

    //
    // Set listeners
    //
    LOG.debug("Set listeners");
    OutlookSyncListener*       listener1 = new OutlookSyncListener      ();
    OutlookSyncSourceListener* listener2 = new OutlookSyncSourceListener();
    OutlookSyncStatusListener* listener3 = new OutlookSyncStatusListener();
    OutlookSyncItemListener*   listener4 = new OutlookSyncItemListener  ();
    OutlookTransportListener*  listener5 = new OutlookTransportListener ();

    setSyncListener      (listener1);
    setSyncSourceListener(listener2);
    setSyncStatusListener(listener3);
    setSyncItemListener  (listener4);
    setTransportListener (listener5);


    //
    // Create the array of SyncSources (only if syncMode != none)
    // ----------------------------------------------------------
    LOG.debug("Creating SyncSources...");
    int sourcesCount = config->getSyncSourceConfigsCount();
    WindowsSyncSource** sources = new WindowsSyncSource*[sourcesCount+1];

    // Sources order is important: 
    // 1. "Contacts"
    // 2. "Calendar"
    // 3. "Tasks"
    // 4. "Notes"
    int j=0;
    while (itemTypesUsed[j]) {
        for (int i=0; i<sourcesCount; i++) {
            const char* syncMode = config->getSyncSourceConfig(i)->getSync();
            if (syncMode && strcmp(syncMode, "none") && strcmp(syncMode, "")) {

                wname = toWideChar(config->getSyncSourceConfig(i)->getName());
                if (!wcscmp(wname, itemTypesUsed[j])) {
                    // Here the right WindowsSyncSourceConfig is passed to the SyncSource!
                    sources[sourcesActive] = new WindowsSyncSource(wname, config->getSyncSourceConfig(i));
                    sourcesActive++;
                }
                delete [] wname;
            }
        }
        j++;
    }
    sources[sourcesActive] = NULL;


    // Create the SyncClient passing pointer of SyncSources vector,  
    // used to check SyncMode in 'continueAfterPrepareSync()'.
    WindowsSyncClient winClient(sources);


    // Exit if no sources to sync
    if (sourcesActive == 0) {
        //safeMessageBox(MSGBOX_NO_SOURCES_TO_SYNC);
        setError(ERR_CODE_NO_SOURCES, ERR_NO_SOURCES_TO_SYNC);
        ret = ERR_CODE_NO_SOURCES;
        goto finally;
    }


    //
    // Create the mutex for sync process.
    // **********************************
    //
    // Refresh the 'beginSync' timestamp now, and save (only this value) to winreg.
    char buf[21];
    unsigned long timestamp = (unsigned long)time(NULL);
    timestampToAnchor(timestamp, buf);
    config->getAccessConfig().setBeginSync(timestamp);
    config->saveBeginSync();

    // - Use always a different mutex name, to avoid errors on pending mutexes (if sync drastically aborted).
    // - We need to know the mutex name from different plugin instances, so use the 'BeginSync' value
    //   that is re-written each time a sync process starts (write it here).
    mutexName = getSyncMutexName();
    LOG.debug("Creating the sync-mutex (\"%s\")", mutexName.c_str());
    syncMutex = CreateMutexA(NULL, TRUE, mutexName.c_str());
    if(!syncMutex){
        char* msg = readSystemErrorMsg();
        setErrorF(getLastErrorCode(), ERR_MUTEX_CREATE, msg);
        LOG.error(getLastErrorMsg());
        delete [] msg;
        ret = 1;
        goto finally;
    }
    if(GetLastError() == ERROR_ALREADY_EXISTS) {
        char* msg = readSystemErrorMsg();
        setErrorF(getLastErrorCode(), ERR_MUTEX_ALREADY_EXISTS, msg);
        LOG.error(getLastErrorMsg());
        delete [] msg;
        ret = 1;
        goto finally;
    }


    // --------------------------------------------------
    // Create the SyncClient object and kick off the sync
    //
    LOG.debug("Start SyncClient::Sync() with %d sources", sourcesActive);
    ret = winClient.sync(*config, (SyncSource**)sources);
    // --------------------------------------------------


    // Print sync results.
    report = winClient.getSyncReport();
    printReport(report, sources);


    //
    // Save configuration to win registry. (TBD: manage dirty flag!)
    // Note: source configs will not be saved if not successfull...
    //
    LOG.debug("Saving configuration to winRegistry");
    config->save(report);


finally:

    endSync();

    //
    // Send msg to UI to refresh status icon (source state)
    //
    int sourceID = -1;

    if(report){  // check if the sync report is valid

        // Update the errors from SyncReport
        setErrorF(report->getLastErrorCode(), "%s", report->getLastErrorMsg());
        ret = getLastErrorCode();

        int i=0;
        while(sources[i]){
            SyncSourceReport* ssReport = NULL;
            ssReport = sources[i]->getReport();

            if (ssReport){
                if (!strcmp(ssReport->getSourceName(), CONTACT_))
                    sourceID = SYNCSOURCE_CONTACTS;
                else if (!strcmp(ssReport->getSourceName(), APPOINTMENT_))
                    sourceID = SYNCSOURCE_CALENDAR;
                else if (!strcmp(ssReport->getSourceName(), TASK_))
                    sourceID = SYNCSOURCE_TASKS;
                else if (!strcmp(ssReport->getSourceName(), NOTE_))
                    sourceID = SYNCSOURCE_NOTES;

                if( (ssReport->getState() != SOURCE_ERROR) && sources[i]->getConfig().getIsSynced() )
                    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SOURCE_STATE, 
                                (WPARAM)sourceID, (LPARAM)SYNCSOURCE_STATE_OK );
                else
                    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SOURCE_STATE, 
                                (WPARAM)sourceID, (LPARAM)SYNCSOURCE_STATE_NOT_SYNCED );
            }
            i++;
        }
    }


    // Clean up SyncSources...
    LOG.debug("Delete SyncSources...");
    if (sources) {
        int i=0;
        while(sources[i]) {
            delete sources[i];
            i++;
        }
        sources = NULL;
    }

#ifdef MALLOC_DEBUG
    printMemLeaks();
#endif

    if (ret == 0)
        LOG.info(INFO_SYNC_COMPLETED);
    else
        LOG.info(INFO_SYNC_COMPLETED_ERRORS, ret);

    return ret;
}


/**
 * Common operations to end the sync process.
 * - Close Outlook instance
 * - Release mutex on sync
 * - Unset Listeners
 */
void endSync() {

    // Close Outlook instance
    closeOutlook();

    // Release mutex on sync (use the same handle of startSync!)
    if (syncMutex) {
        LOG.debug("Releasing sync-mutex...");
        if (!ReleaseMutex(syncMutex)) {
            char* msg = readSystemErrorMsg();
            setErrorF(getLastErrorCode(), ERR_MUTEX_NOT_RELEASED, msg);
            LOG.error(getLastErrorMsg());
            delete [] msg;
        }
        LOG.debug("Sync-mutex released - closing handle.");
        CloseHandle(syncMutex);
        syncMutex = NULL;
    }

    // *** TODO: see if necessary ***
    //CoUninitialize();

    // Unset Listeners
    ManageListener::dispose();
}




/**
 * Closing operation before exiting client DLL.
 */
int closeClient() {

    // Try closing Outlook session if not yet done.
    closeOutlook();

    // Delete OutlookConfig instance.
    if (OutlookConfig::isInstantiated()) {
        OutlookConfig* config = OutlookConfig::getInstance();
        if (config) {
            LOG.debug("Deleting OutlookConfig instance");
            delete config;
        }
        LOG.debug(DBG_CONFIG_CLOSED);
    }

    LOG.info(INFO_EXIT);
    return 0;
}




/**
 * If Outlook session is active, close it and clean-up shared objects.
 */
void closeOutlook() {

    if ( ClientApplication::isInstantiated() ) {

        LOG.debug("Closing Outlook...");
        ClientApplication* outlook;
        try {
            outlook = ClientApplication::getInstance();
            if (outlook) {
                LOG.debug("Deleting ClientApplication instance");
                delete outlook;

                // Close COM library for current thread
                LOG.debug("Closing COM library...");
                CoUninitialize();
            }
        }
        catch (ClientException* e) {
            manageClientException(e);
            LOG.error(ERR_CLOSE_OUTLOOK);
        }
        // *** To catch unexpected exceptions... ***
        catch (...) {
            LOG.error(ERR_CLOSE_OUTLOOK);
        }
    }
}



/**
 * Check if a synchronization process is already running.
 * This is obtained checking the correspondent mutex, which is created
 * during the sync process.
 * @return  true if a sync is already running.
 */
bool checkSyncInProgress() {

    // ALWAYS get exact name from 'BeginSync' property.
    string mutexName = getSyncMutexName();

    //
    // Try opening the mutex:
    //
    HANDLE hMutex = syncMutex;
    if (!hMutex) {
        hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexName.c_str());
    }
    
    // OpenMutex failed.
    if (hMutex == NULL) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            // *** mutex not found *** -> no sync in progress.
            return false;
        }
        // Some other error
        char* msg = readSystemErrorMsg();
        setErrorF(getLastErrorCode(), ERR_MUTEX_OPEN, msg);
        LOG.error(getLastErrorMsg());
        delete [] msg;
        return false;
    }

    // *** mutex found *** -> sync in progress OR pending mutex?
    else {

        //
        // Try getting access to the mutex found:
        //
        DWORD dwWaitResult = WaitForSingleObject(hMutex, 0L);

        switch (dwWaitResult) {

            // This thread has obtained access: mutex free or previous thread was killed
            // -> release the mutex, no sync in progress.
            case WAIT_ABANDONED:
                LOG.debug(DBG_LAST_SYNC_ABORTED);
            case WAIT_OBJECT_0: {

                // ***TODO*** This has been commented cause not sure if correct to 
                //            release a pending mutex (owner could try to release it)
                //if (!ReleaseMutex(hMutex)) {
                //    char* msg = readSystemErrorMsg();
                //    setErrorF(getLastErrorCode(), ERR_MUTEX_NOT_RELEASED, msg);
                //    LOG.error(getLastErrorMsg());
                //    delete [] msg;
                //}
                // ***TODO*** verify this: probably there's no need to close handle here
                //            (can lead to 2 close handle, if owner closes it just after this)
                //CloseHandle(hMutex);

                ReleaseMutex(hMutex);
                return false;
            }

            // Cannot get mutex ownership due to time-out (sync already in progress)
            case WAIT_TIMEOUT: {
                return true;
            }

            // Some error occurred (case WAIT_FAILED)
            default: {
                char* msg = readSystemErrorMsg();
                setError(getLastErrorCode(), msg);
                LOG.error(getLastErrorMsg());
                delete [] msg;
                return true;
            }
        }
    }
}



/**
 * This function is called to terminate the sync session (soft way).
 * We first try to "soft" terminate the thread, setting the correspondent
 * flag inside OutlookConfig. This is to correctly close the session.
 */
void softTerminateSync() {

    OutlookConfig* config = getConfig();
    config->setAbortSync(true);
    LOG.debug(DBG_SYNC_ABORT_REQUEST);
}




/**
 * This function is called to terminate the sync session (hard way).
 * If softTerminateSync() did not work, use this function to terminate
 * the thread of synchronization.
 *
 * @param hSyncThread   the handle of sync thread to terminate
 * @return              0 if sync aborted successfully.
 *                      1 if could not terminate the sync thread.
 */
int hardTerminateSync(HANDLE hSyncThread) {

    // (code 4 = thread terminated)
    if (!TerminateThread(hSyncThread, 4)) {
        char* msg = readSystemErrorMsg();
        setErrorF(4, ERR_THREAD_NOT_TERMINATED, msg);
        LOG.error(getLastErrorMsg());
        delete [] msg;
        return 1;
    }

    setErrorF(2, INFO_SYNC_ABORTED_BY_USER);    // code 2 = sync aborted by the user
    LOG.info(getLastErrorMsg());
    LOG.debug(DBG_THREAD_TERMINATED);
    return 0;
}



/**
 * This function is called to terminate the sync thread.
 * Like 'hardTerminateSync' but used internally by client
 * to terminate the syncThread (HANDLE set during startSync).
 *
 * @param code   the exit code for the sync thread
 * @return       0 if sync thread terminated successfully.
 *               1 if could not terminate the sync thread.
 */
int exitSyncThread(int code) {

    if (syncThread) {
        if (!TerminateThread(syncThread, code)) {
            char* msg = readSystemErrorMsg();
            setErrorF(getLastErrorCode(), ERR_THREAD_NOT_TERMINATED, msg);
            LOG.error(getLastErrorMsg());
            delete [] msg;
            return 1;
        }
        return 0;
    }
    return 1;
}



/**
 * Check if synchronization session has been intentionally aborted.
 * A flag 'abortSync' inside OutlookConfig singleton object is used to
 * indicate that the user wants to abort the sync.
 * The client periodically checks this flag, using this function.
 * @note  this is important to correctly close Outlook session, as the logoff must
 *        be done by the same thread that logged in... otherwise Outlook may become
 *        instable or could not respond.
 */
void checkAbortedSync() {

    OutlookConfig* config = getConfig();
    
    if (config->getAbortSync()) {
        LOG.info(INFO_SYNC_ABORTING);

        endSync();

        // Throw SyncException with code 2 (sync aborted by user)
        setErrorF(getLastErrorCode(), INFO_SYNC_ABORTED_BY_USER);
        LOG.info(getLastErrorMsg());
        config->setAbortSync(false);
        throwSyncException(getLastErrorMsg(), 2);
    }
}




/**
 * Returns the full Outlook path of the default folder, given the desired 'itemType'.
 *
 * @note  to correctly manage COM library, we delete the ClientApplication instance
 *        at the end of this method (it's called from UI config window, Details).
 *
 * @param itemType   the type of items for the folder to select (CONTACT/TASK/...)
 * @return           path of default folder (empty string if errors).
 */
wstring getDefaultFolderPath(const wstring& itemType) {

    wstring pathSelected = EMPTY_WSTRING;
    ClientApplication* outlook = NULL;
    ClientFolder*      folder  = NULL;
    
    try {
        outlook = ClientApplication::getInstance();
        if (outlook) {
            folder = outlook->getDefaultFolder(itemType);
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
        pathSelected = EMPTY_WSTRING;
        goto finally;
    }

    if (folder) {
        pathSelected = folder->getPath();
    }

finally:
    // Delete ClientApplication: this is called from UI config, so we should
    // release the COM library to be correctly used by next thread.
    try {
        if (outlook) {
            delete outlook; 
            outlook = NULL;
        }
    } 
    catch (ClientException* e) {
        manageClientException(e);
    }
    return pathSelected;
}



/**
 * This function displays an Outlook window on desktop, to browse for a
 * specific Outlook folder. If passed 'itemType' is not an empty string,
 * the user will have to select a folder of the correct type - otherwise
 * a warning+retry will be displayed.
 * The full path of folder will be returned.
 *
 * @note  to correctly manage COM library, we delete the ClientApplication instance
 *        at the end of this method (it's called from UI config window, Details).
 *
 * @param itemType   the type of items for the folder to select (CONTACT/TASK/...)
 * @return           path of folder selected (empty string if not selected).
 */
wstring pickOutlookFolder(const wstring& itemType) {

    wstring pathSelected = EMPTY_WSTRING;
    ClientApplication* outlook = NULL;
    ClientFolder*      folder  = NULL;

    
    // Pick the desired folder.
    try {
        outlook = ClientApplication::getInstance();
        if (outlook) {
            folder = outlook->pickFolder(itemType);
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
        pathSelected = EMPTY_WSTRING;
        goto finally;
    }

    // Get folder's path.
    if (folder) {
        pathSelected = folder->getPath();
    }

finally:
    // Delete ClientApplication: this is called from UI config, so we should
    // release the COM library to be correctly used by next thread.
    try {
        if (outlook) {
            delete outlook; 
            outlook = NULL;
        }
    } 
    catch (ClientException* e) {
        manageClientException(e);
    }

    return pathSelected;
}



/**
 * Set the scheduled task of plugin.
 * @param enable    true  = activate the scheduler
 *                  false = disable the scheduler (delete the task)
 * @param minutes   the repeating minutes of task
 * @return          0 if no errors
 */
int setScheduler(const bool enable, const int minutes) {

    //
    // Activate Windows scheduler: create or update the task.
    //
    if (enable) {

        int dayNum = SCHED_DURATION_DAYS;           // Fixed: task duration = 1 day.
        int minNum = SCHED_DEFAULT_REPEAT_MINS;     // Default every 15 min.
        if (minutes > 0) {
            minNum = minutes;
        }

        return setScheduleTask(EVERY_DAY, dayNum, minNum);
    }

    //
    // Delete the task (if any).
    //
    else {
        return deleteScheduleTask();
    }
}


/**
 * Get information about scheduled task of plugin. 
 * @param minutes   [OUT] the repeating minutes of task
 * @return          true if task is active.
 *                  false if task is disabled or not existing.
 */
bool getScheduler(int* minutes) {

    bool active = false;
    int dayNum  = 0;
    int minNum  = 0;

    int ret = getScheduleTask(&active, &dayNum, &minNum);

    if (ret < 0) {
        // Not found OR errors
        return false;
    }

    if ((dayNum != SCHED_DURATION_DAYS) || (ret == 2)) {
        LOG.debug(DBG_SCHED_TASK_MANUALLY_CHANGED);
    }

    if (active == false) {
        // Task is disabled
        return false;
    }

    // If here, task found and active.
    *minutes = minNum;
    return true;
}



const char* getClientLastErrorMsg() {
    return getLastErrorMsg();
}

const int getClientLastErrorCode() {
    return getLastErrorCode();
}



/**
 * Operations to upgrade the plugin from 'oldVersion' to this version.
 * Should be called only once during plugin first start.
 */
void upgradePlugin(int oldVersion) {

    if (oldVersion < 60000) {
        return;
    }

    // Old version < 6.0.10: remove the old scheduled task (now per-user jobs)
    if (oldVersion < 60010) {
        char tmp[MAX_PATH_LENGTH];
        GetWindowsDirectoryA(tmp, MAX_PATH_LENGTH);

        // usually: "C:\Windows\Tasks\Funambol Outlook Plug-in.job"
        string jobPath = tmp;
        jobPath += "\\Tasks\\Funambol Outlook Plug-in.job";
        remove(jobPath.c_str());
    }

    // Old version < 6.0.9: delete the old logfile (now .txt)
    if (oldVersion < 60009) {
        OutlookConfig* config = OutlookConfig::getInstance();
        string oldLogPath = config->getLogDir();
        oldLogPath += "\\OLPlugin.log";
        remove(oldLogPath.c_str());
    }


    // Old version < 7.1.4: Client name has changed, was "Outlook Plug-in"
    //   1. move cache files and delete old folder
    //   2. remove old cache dir
    //   3. rename scheduler task if existing
    if (oldVersion < 70104) {

        makeDataDirs();

        // Get 'application data' folder for current user.
        WCHAR appDataPath[MAX_PATH_LENGTH];
        if ( FAILED(SHGetFolderPath(NULL, 
                                    CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                                    NULL,
                                    0,
                                    appDataPath)) ) {
            DWORD code = GetLastError();
            char* msg = readSystemErrorMsg(code);
            LOG.error(ERR_APPDATA_PATH, code, msg);
            delete [] msg;
            return;
        }

        wstring oldDataPath(appDataPath);
        oldDataPath += TEXT("\\");
        oldDataPath += FUNAMBOL_DIR_NAME;
        oldDataPath += TEXT("\\");
        oldDataPath += TEXT("Outlook Plug-in");

        wstring newDataPath(appDataPath);
        newDataPath += TEXT("\\");
        newDataPath += FUNAMBOL_DIR_NAME;
        newDataPath += TEXT("\\");
        newDataPath += OLPLUGIN_DIR_NAME;

        // List of possible cache files to copy
        list<wstring> fileNames;
        fileNames.clear();
        fileNames.push_back(TEXT("\\appointment.db"));
        fileNames.push_back(TEXT("\\appointment_modified.db"));
        fileNames.push_back(TEXT("\\contact.db"));
        fileNames.push_back(TEXT("\\note.db"));
        fileNames.push_back(TEXT("\\task.db"));

        // Copy ALL cache files (*.db) to new location.
        wstring oldName, newName;
        list<wstring>::iterator it;
        for (it = fileNames.begin(); it != fileNames.end(); it++) {
            oldName = oldDataPath;  oldName += *it;
            newName = newDataPath;  newName += *it;
            CopyFile(oldName.c_str(), newName.c_str(), FALSE);
        }

        // TODO: better use this method.
        //char* buf = toMultibyte(oldDataPath.c_str());
        //StringBuffer oldPath(buf);
        //delete [] buf;
        //ArrayList filesToCopy = getAllFilesInDir(oldPath, "*.db");  <-- add in API!


        // 2. Now we can remove the old cache dir with all its content.
        char* oldDir = toMultibyte(oldDataPath.c_str());
        removeFileInDir(oldDir);
        delete [] oldDir;
        RemoveDirectory(oldDataPath.c_str());


        // 3. Rename the scheduled task for this user only (we don't have more permissions)
        // was: "C:\Windows\Tasks\Funambol Outlook Plug-in - <username>.job"
        wstring user;
        getWindowsUser(user);
        WCHAR winDir[MAX_PATH_LENGTH];
        GetWindowsDirectory(winDir, MAX_PATH_LENGTH);

        wstring oldTaskName(winDir);
        oldTaskName += TEXT("\\Tasks\\Funambol Outlook Plug-in - ");
        oldTaskName += user;
        oldTaskName += TEXT(".job");

        wstring name;
        getScheduledTaskName(name);     // Using this function, so it's always the correct name
        wstring newTaskName(winDir);
        newTaskName += TEXT("\\Tasks\\");
        newTaskName += name;
        newTaskName += TEXT(".job");

        _wrename(oldTaskName.c_str(), newTaskName.c_str());
    }
}

