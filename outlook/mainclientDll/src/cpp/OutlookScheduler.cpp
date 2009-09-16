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

#include <initguid.h>
#include <mstask.h>
#include <objidl.h>

#include "base/util/utils.h"
#include "base/stringUtils.h"
#include "utils.h"
#include "winmaincpp.h"
#include "OutlookConfig.h"
#include "customization.h"

#include <string>
using namespace std;


/**
 * Set a Task into task scheduler. It takes values to insert from Schedule form.
 * If task already exists, it is updated with passed parameters.
 * Starting time is always set to current time (task starts now).
 *
 * @param frequency : NEVER or EVERY_DAY
 * @param dayNum    : repeate the task every 'dayNum'
 * @param minNum    : repeate the task every 'minNum'
 * @return            0 if no errors.
 */
int setScheduleTask (const char* frequency, const int dayNum, const int minNum) {

    HRESULT hr = S_OK;
    bool newTask = false;
    ITaskScheduler* pITS = NULL;
    ITask*          pITask;
    IPersistFile*   pIPersistFile;
    wstring taskName, user;

    // Init COM library & create instance for Task scheduler.
    if ((pITS = initScheduleInstance()) == NULL) {
        goto error;
    }

    // Task is associated with current user
    if (getScheduledTaskName(taskName)) {
        goto error;
    }

    // We need the extended username "MACHINE\USER" to ensure that
    // the job can run also for users logged in with another domain (ref bug #307010).
    if (getWindowsUserEx(user)) {
        goto error;
    }

    //
    // Try to open EXISTING TASK.
    //
    LOG.debug("Try opening the Windows task: \"%ls\"", taskName.c_str());
    hr = pITS->Activate(taskName.c_str(),
                        IID_ITask,
                        (IUnknown**) &pITask);
    
    if (FAILED(hr)) {
        if (hr == E_OBJECT_NOT_FOUND) {
            //
            // NEW TASK -> create it.
            //
            LOG.debug(DBG_SCHED_TASK_NOT_FOUND);
            newTask = true;

            WCHAR path[512];
            OutlookConfig* config = OutlookConfig::getInstance();
            // Get current working dir (from config)
            const char* t = config->getWorkingDir();
            WCHAR* wt = toWideChar(t);
            wsprintf(path, TEXT("\"%s\\%s\""), wt, TEXT(PROGRAM_NAME_EXE));
            delete [] wt;  wt = NULL;

            hr = pITS->NewWorkItem(taskName.c_str(),       // Name of task
                                   CLSID_CTask,            // Class identifier
                                   IID_ITask,              // Interface identifier
                                   (IUnknown**)&pITask);   // Address of task interface
            if (FAILED(hr)) {
                char* msg = readSystemErrorMsg(hr);
                setErrorF(getLastErrorCode(), ERR_SCHED_NEWWORKITEM, hr, msg);
                delete [] msg;
                pITS->Release();
                goto error;
            }

            // Set path and comment on new task.
            pITask->SetApplicationName(path);
            pITask->SetComment(SCHED_COMMENT);
        }
        else {
            char* msg = readSystemErrorMsg(hr);
            setErrorF(getLastErrorCode(), ERR_SCHED_ACTIVATE, hr, msg);
            delete [] msg;
            pITS->Release();
            goto error;
        }
    }
    pITS->Release();


    //
    // Set task params: flags, account info, parameters
    if (strcmp(NEVER, frequency) == 0) {
        pITask->SetFlags(TASK_FLAG_DISABLED);
    } 
    else {
        pITask->SetAccountInformation(user.c_str(), NULL);
        pITask->SetFlags(TASK_FLAG_INTERACTIVE | TASK_FLAG_RUN_ONLY_IF_LOGGED_ON);
    }

    // ...so we have "OutlookPlugin.exe schedule"    
    pITask->SetParameters(SCHED_PARAM);


    //
    // Get/create the trigger object.
    //
    ITaskTrigger *pITaskTrigger;
    if (newTask) {
        WORD piNewTrigger;
        hr = pITask->CreateTrigger(&piNewTrigger, &pITaskTrigger);
        if (FAILED(hr)) {
            char* msg = readSystemErrorMsg(hr);
            setErrorF(getLastErrorCode(), ERR_SCHED_CREATE_TRIGGER, hr, msg);
            delete [] msg;
            goto error;
        }
    }
    else {
        hr = pITask->GetTrigger(0, &pITaskTrigger);
        if (FAILED(hr)) {
            char* msg = readSystemErrorMsg(hr);
            setErrorF(getLastErrorCode(), ERR_SCHED_GET_TRIGGER, hr, msg);
            delete [] msg;
            goto error;
        }
    }

    //////////////////////////////////////////////////////
    // Define TASK_TRIGGER structure.
    // Start time is derived from NOW.
    //////////////////////////////////////////////////////
    TASK_TRIGGER pTrigger;
    ZeroMemory(&pTrigger, sizeof (TASK_TRIGGER));

    time_t timer;
    time(&timer);
    struct tm* now = localtime(&timer);

    pTrigger.wBeginDay                = now->tm_mday;               // START = NOW.
    // TASK_TRIGGER uses wBeginMonth in interval (1,12), but tm_mon is in interval (0,11)
    pTrigger.wBeginMonth              = now->tm_mon + 1;
    pTrigger.wBeginYear               = now->tm_year + 1900;        // 'struct tm' has year from 1900...
    pTrigger.wStartHour               = now->tm_hour;
    pTrigger.wStartMinute             = now->tm_min - 1;            // -1 to avoid starting a sync right now!

    pTrigger.cbTriggerSize = sizeof (TASK_TRIGGER) ;
    pTrigger.MinutesDuration          = 1440;                       // Duration = fixed 1 day.
    pTrigger.MinutesInterval          = minNum;
    pTrigger.TriggerType              = TASK_TIME_TRIGGER_DAILY;    // Manage only DAILY schedules!
    pTrigger.Type.Daily.DaysInterval  = dayNum;
    //////////////////////////////////////////////////////

    //
    // Set trigger criteria.
    //
    hr = pITaskTrigger->SetTrigger(&pTrigger);
    if (FAILED(hr)) {
        if (hr == E_INVALIDARG) {
            setErrorF(getLastErrorCode(), ERR_SCHED_SET_TRIGGER, hr, ERR_SCHED_INVALID_PARAM);
        }
        else {
            char* msg = readSystemErrorMsg(hr);
            setErrorF(getLastErrorCode(), ERR_SCHED_SET_TRIGGER, hr, msg);
            delete [] msg;
        }
        goto error;
    }

    //
    // Call IUnknown::QueryInterface to get a pointer to IPersistFile.
    //
    hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    if (FAILED(hr)) {
        char* msg = readSystemErrorMsg(hr);
        setErrorF(getLastErrorCode(), ERR_SCHED_QUERY_INTERFACE, hr, msg);
        delete [] msg;
        goto error;
    }
    pITask->Release();
    pITaskTrigger->Release();

    //
    // Save the task to disk.
    //
    hr = pIPersistFile->Save(NULL, TRUE);
    if (FAILED(hr)) {
        char* msg = readSystemErrorMsg(hr);
        setErrorF(getLastErrorCode(), ERR_SCHED_SAVE, hr, msg);
        delete [] msg;
        goto error;
    }
    pIPersistFile->Release();

    LOG.info(INFO_SCHED_TASK_CREATED);
    CoUninitialize();
    return 0;


error:
    CoUninitialize();
    LOG.error(getLastErrorMsg());
    return 1;
}





/**
 * Get task info from Task scheduler.
 * Returns 0 if task found and no errors, -1 if task not found.
 * If task found: frequency, dayNum, minNum and statusCode are set.
 *
 * @param active  : [OUT] true if task is active
 * @param dayNum  : [OUT] repeating every 'dayNum' days
 * @param minNum  : [OUT] repeating every 'minNum' minutes
 * @return          0 = task found, status correct.
 *                  1 = task found, status incorrect.
 *                  2 = task found but manually changed.
 *                 -1 = task not found.
 *                 -2 = errors occurred.
 */
int getScheduleTask(bool* active, int* dayNum, int* minNum) {

    int ret = 0;
    HRESULT hr = S_OK;
    ITaskScheduler* pITS = NULL;
    ITask*          pITask;
    wstring taskName;

    // Init COM library & create instance for Task scheduler.
    if ((pITS = initScheduleInstance()) == NULL) {
        goto error;
    }

    // Task is associated with current user
    if (getScheduledTaskName(taskName)) {
        goto error;
    }

    //
    // Open the task (fails if task not existing).
    //
    hr = pITS->Activate(taskName.c_str(),
                        IID_ITask,
                        (IUnknown**) &pITask);
    pITS->Release();
    if (FAILED(hr)) {
        goto notExisting;
    }


    //
    // Check task status
    //
    DWORD pdwExitCode;
    hr = pITask->GetExitCode(&pdwExitCode);
    if ((hr & SCHED_S_TASK_HAS_NOT_RUN) || (hr != S_OK)) {
        setErrorF(getLastErrorCode(), DBG_SCHED_LAST_EXECUTION);
        LOG.debug(getLastErrorMsg());
        ret = 1;
    }


    //
    // Check the flags -> active/disabled
    //
    DWORD flag = -1, triggerType = -1;
    pITask->GetFlags(&flag);
    if (flag & TASK_FLAG_DISABLED) {
        *active = false;
    } else {
        *active = true;
    }


    //
    // Get the trigger interface.
    //
    ITaskTrigger *pITaskTrigger;
    hr = pITask->GetTrigger(0, &pITaskTrigger);
    if (FAILED(hr)) {
        char* msg = readSystemErrorMsg(hr);
        setErrorF(getLastErrorCode(), ERR_SCHED_GET_TRIGGER, hr, msg);
        delete [] msg;
        goto error;
    }

    TASK_TRIGGER pTrigger;
    ZeroMemory(&pTrigger, sizeof(TASK_TRIGGER));
    hr = pITaskTrigger->GetTrigger(&pTrigger);
    if (FAILED(hr)) {
        char* msg = readSystemErrorMsg(hr);
        setErrorF(getLastErrorCode(), ERR_SCHED_GET_TRIGGER2, hr, msg);
        delete [] msg;
        goto error;
    }

    //
    // Get values
    //
    *minNum = pTrigger.MinutesInterval;
    *dayNum = pTrigger.Type.Daily.DaysInterval;

    // Check if trigger correct
    triggerType = pTrigger.TriggerType;
    if (triggerType != TASK_TIME_TRIGGER_DAILY ||
        (pTrigger.MinutesDuration != 0
         && pTrigger.MinutesInterval != 0
         && pTrigger.MinutesDuration != 1440) ) {
        ret = 2;
    }

    // Release the ITask interface.
    pITaskTrigger->Release();
    pITask->Release();

    CoUninitialize();
    return ret;

notExisting:
    LOG.debug(DBG_SCHED_TASK_NOT_FOUND);
    *active = false;
    *minNum = 0;
    *dayNum = 0;
    CoUninitialize();
    return -1;

error:
    LOG.error(getLastErrorMsg());
    CoUninitialize();
    return -2;
}




/**
 * Delete the schedule task.
 * @return 0 if no errors
 */
int deleteScheduleTask() {

    HRESULT hr = S_OK;
    ITaskScheduler* pITS = NULL;
    wstring taskName;

    // Init COM library & create instance for Task scheduler.
    if ((pITS = initScheduleInstance()) == NULL) {
        goto error;
    }

    // Task is associated with current user
    if (getScheduledTaskName(taskName)) {
        goto error;
    }

    //
    // Delete the task (if exists).
    //
    hr = pITS->Delete(taskName.c_str());
    pITS->Release();
    if (FAILED(hr)) {
        if (hr == E_OBJECT_NOT_FOUND) {
            // Not found -> no need to notify.
        }
        else {
            char* msg = readSystemErrorMsg(hr);
            setErrorF(getLastErrorCode(), ERR_SCHED_DELETE, hr, msg);
            delete [] msg;
            goto error;
        }
    }

    LOG.info(INFO_SCHED_TASK_DELETED);
    CoUninitialize();
    return 0;

error:
    LOG.error(getLastErrorMsg());
    CoUninitialize();
    return 1;
}



/**
 * Call CoInitialize to initialize the COM library and then
 * CoCreateInstance to get the Task Scheduler object.
 * Sets the lastErrorMsg in case of errors.
 * @param pITS: pointer to ITaskScheduler interface
 * @return      0 if no errors
 */
ITaskScheduler* initScheduleInstance() {

    ITaskScheduler* pITS;
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_CTaskScheduler,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ITaskScheduler,
                              (void**) &pITS);
        if (FAILED(hr)) {
            setErrorF(getLastErrorCode(), "%s %s", ERR_SCHED_INIT_TASK, ERR_COM_CREATE_INSTANCE);
            return NULL;
        }
    }
    else {
        setErrorF(getLastErrorCode(), "%s %s", ERR_SCHED_INIT_TASK, ERR_COM_INITIALIZE);
        return NULL;
    }

    return pITS;
}


/**
 * Used to create the scheduled task name: 
 * "Funambol Outlook Sync Client - <UserName>".
 * 'Username' is the current Windows user, so each user has a different task.
 *
 * @param  taskName  [OUT] the name of task
 * @return           0 if no errors
 */
int getScheduledTaskName(wstring& taskName) {

    taskName = WPROGRAM_NAME;

    wstring user;
    if (getWindowsUser(user)) {
        return 1;
    }
    taskName += L" - ";
    taskName += user;
    return 0;
}