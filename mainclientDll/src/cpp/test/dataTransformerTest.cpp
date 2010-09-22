/*
 * Funambol is a mobile platform developed by Funambol, Inc.
 * Copyright (C) 2008 Funambol, Inc.
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

#include "WinUnit.h"
#include <stdio.h>
#include "base/util/utils.h"
#include "base/globalsdef.h"
#include "base/Log.h"
#include "base/stringUtils.h"
#include "winmaincpp.h"
#include "utils.h"
#include "vocl/VConverter.h"

#include "vocl/WinContact.h"
#include "vocl/WinEvent.h"
#include "vocl/WinTask.h"
#include "vocl/WinNote.h"
#include "vocl/WinContactSIF.h"
#include "vocl/WinEventSIF.h"
#include "vocl/WinTaskSIF.h"
#include "vocl/WinNoteSIF.h"

#include "outlook/utils.h"

#include "outlook/ClientItem.h"
#include "outlook/ClientTask.h"
#include "outlook/ClientAppointment.h"
#include "outlook/ClientContact.h"
#include "outlook/ClientNote.h"
#include "outlook/ClientAppException.h"
#include "outlook/ClientException.h"
#include "SIFFields.h"
#include "customization.h"
#include "spds/spdsutils.h"

#include "syncml\core\Property.h"
#include "syncml\core\PropParam.h"
#include <string>
#include "test/helper.h"

USE_FUNAMBOL_NAMESPACE

BEGIN_TEST(DummyTest)
{
    int i = 3;
    int k = 3;
    WIN_ASSERT_EQUAL(i, k);
}
END_TEST

BEGIN_TEST(StringBufferTest)
{
    StringBuffer first("string");
    StringBuffer second("string");
    WIN_ASSERT_EQUAL(first, second);
}
END_TEST


WinContact* updateOutlookItem(WindowsSyncSource* ss, SyncItem* item) {
    
    int ret = ss->updateItem(*item);
    WIN_ASSERT_EQUAL(ret, 200, TEXT("Update item is not correct"));
    wstring keyModified = item->getKey();

    SyncItem* itemTmp = ss->getItemFromId(keyModified.c_str());
    WIN_ASSERT_NOT_NULL(item, TEXT("The item retrieved from the backend is null."));

    WCHAR* wcon = toWideChar((const char*)itemTmp->getData());
    wstring wcontact(wcon);
    delete [] wcon;
   
    WinContact* winC = new WinContact(wcontact);
    return winC;

}

WindowsSyncSource* createContactWindowsSyncSource() {

    OutlookConfig* config = getConfig();
    config->getServerConfig().setNoFieldLevelReplace("card");

    WIN_ASSERT_NOT_NULL(config, TEXT("The config is null. Please verify the an Outlook client is already installed"));
    WindowsSyncSourceConfig* sc = config->getSyncSourceConfig(CONTACT_);
    WindowsSyncSource* ss = new WindowsSyncSource(CONTACT, sc);
    int ret = ss->beginSync();   
    WIN_ASSERT_ZERO(ret, TEXT("beginSync is not 0"));

    SyncSourceReport* ssReport = new SyncSourceReport();
    ssReport->setSourceName(sc->getName());
    ssReport->setState(SOURCE_ACTIVE);
    ss->setReport(ssReport);

    return ss;

}



/**
* The test must be executed with an already installed Outlook client
*/
BEGIN_TEST(WinContactRemoveBusinessTelTest)
{
    int ret = 0;
    SyncItem* item = NULL;
    WCHAR* internalKey = NULL;
    wstring propValue; 

    WindowsSyncSource* ss = createContactWindowsSyncSource();   
    WIN_ASSERT_NOT_NULL(ss, TEXT("The syncSource is null."));

    StringBuffer VCard;
    VCard.append("BEGIN:VCARD\r\n\r\n");
    VCard.append("VERSION:2.1\r\n");
    VCard.append("N:Jhon;sir;Doe;;\r\n");
    VCard.append("TEL;WORK;FAX:4444444444\r\n");
    VCard.append("TEL;VOICE;WORK:22222222222\r\n");
    VCard.append("TEL;VOICE;WORK:333333333333\r\n");
    VCard.append("TEL;CAR;VOICE:66666666666\r\n");
    VCard.append("END:VCARD");

    item = new SyncItem(TEXT("GUID"));
    item->setData(VCard.c_str(), VCard.length());
    
    // insert test item
    ret = ss->addItem(*item);
    WIN_ASSERT_EQUAL(ret, 201, TEXT("Adding item is not correct"));    
    internalKey = (WCHAR*)item->getKey();
    
    //
    // remove work and car tel number
    //
    VCard.reset();
    VCard.append("BEGIN:VCARD\r\n\r\n");
    VCard.append("VERSION:2.1\r\n");
    VCard.append("N:Marco;sir;Magistrali;;\r\n");
    VCard.append("TEL;VOICE;WORK:22222222222\r\n");
    VCard.append("TEL;VOICE;WORK:333333333333\r\n");
    VCard.append("END:VCARD");
    
    // update item
    item->setData(VCard.c_str(), VCard.length());
    WinContact* winC = updateOutlookItem(ss, item);
    WIN_ASSERT_NOT_NULL(winC, TEXT("The item retrieved from the backend is null."));
    
    // check modifications
    winC->getProperty(L"BusinessFaxNumber", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The TEL;WORK;FAX: is not null"));
    winC->getProperty(L"CarTelephoneNumber", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The TEL;WORK;FAX: is not null"));    
    delete winC;
    
    //
    // remove one work tel number
    //
    VCard.reset();
    VCard.append("BEGIN:VCARD\r\n\r\n");
    VCard.append("VERSION:2.1\r\n");
    VCard.append("N:Marco;sir;Magistrali;;\r\n");
    VCard.append("TEL;VOICE;WORK:333333333333\r\n");
    VCard.append("END:VCARD");

    item->setData(VCard.c_str(), VCard.length());
    winC = updateOutlookItem(ss, item);
    WIN_ASSERT_NOT_NULL(winC, TEXT("The item retrieved from the backend is null."));
    
    winC->getProperty(L"BusinessTelephoneNumber", propValue);
    WIN_ASSERT_EQUAL(propValue, L"333333333333", TEXT("The BusinessTelephoneNumber is not null"));
    winC->getProperty(L"Business2TelephoneNumber", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The Business2TelephoneNumber is not null"));
    delete winC;
    
    //
    // delete the test item
    // 
    ret = ss->deleteItem(*item);
    WIN_ASSERT_EQUAL(ret, 200, TEXT("delete item is not correct"));
    
    delete item;
    delete ss;
}
END_TEST


/**
* The test must be executed with an already installed Outlook client
*/
BEGIN_TEST(WinContactRemoveEmail)
{
    int ret = 0;
    SyncItem* item = NULL;
    WCHAR* internalKey = NULL;
    wstring propValue; 

    WindowsSyncSource* ss = createContactWindowsSyncSource();   
    WIN_ASSERT_NOT_NULL(ss, TEXT("The syncSource is null."));

    StringBuffer VCard;
    VCard.append("BEGIN:VCARD\r\n");
    VCard.append("VERSION:2.1\r\n");
    VCard.append("N:Paperino;Paolino;Middle;Mr.;suffix\r\n");
    VCard.append("BDAY:20100901\r\n");
    VCard.append("TEL;WORK;FAX:4444444444\r\n");
    VCard.append("TEL;VOICE;WORK:22222222222\r\n");
    VCard.append("TEL;VOICE;WORK:333333333333\r\n");
    VCard.append("TEL;CAR;VOICE:66666666666\r\n");
    VCard.append("EMAIL;INTERNET:paolino@doe.com\r\n");
    VCard.append("EMAIL;INTERNET;HOME:paolino2@dow.com\r\n");
    VCard.append("EMAIL;INTERNET;WORK:paolino3@dor.com\r\n");
    VCard.append("ADR;HOME:;;Via Riviera 55;pavia;pv;27100;Italy\r\n");
    VCard.append("ADR:;;Via Naples 23;milano;mi;3993;Italy\r\n");
    VCard.append("ADR;WORK:;;Via Roma;New york;NY;20111;Italy\r\n");
    VCard.append("END:VCARD");

    item = new SyncItem(TEXT("GUID"));
    item->setData(VCard.c_str(), VCard.length());
    
    // insert test item
    ret = ss->addItem(*item);
    WIN_ASSERT_EQUAL(ret, 201, TEXT("Adding item is not correct"));    
    internalKey = (WCHAR*)item->getKey();
    
    //
    // remove work and car tel number
    //
    VCard.reset();
    VCard.append("BEGIN:VCARD\r\n");
    VCard.append("VERSION:2.1\r\n");
    VCard.append("N:Paperino;Paolino;Middle;Mr.;suffix\r\n");
    VCard.append("TEL;WORK;FAX:4444444444\r\n");
    VCard.append("TEL;VOICE;WORK:22222222222\r\n");
    VCard.append("TEL;VOICE;WORK:333333333333\r\n");
    VCard.append("TEL;CAR;VOICE:66666666666\r\n");
    VCard.append("EMAIL;INTERNET;HOME:paolino2@dow.com\r\n");
    VCard.append("ADR;HOME:;;Via Riviera 55;pavia;pv;27100;Italy\r\n");
    VCard.append("ADR;WORK:;;Via Roma;New york;NY;20111;Italy\r\n");
    VCard.append("END:VCARD");
    
    // update item
    item->setData(VCard.c_str(), VCard.length());
    WinContact* winC = updateOutlookItem(ss, item);
    WIN_ASSERT_NOT_NULL(winC, TEXT("The item retrieved from the backend is null."));
    
    // check modifications
    // Mapping is:
    // Email1Address <-> EMAIL;INTERNET:
    // Email2Address <-> EMAIL;INTERNET;HOME:
    // Email3Address <-> EMAIL;INTERNET;WORK:

    winC->getProperty(L"Email1Address", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The Email2Address is not null"));
    winC->getProperty(L"Email3Address", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The Email3Address is not null"));  
    winC->getProperty(L"Birthday", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The Birthday is not null")); 
    winC->getProperty(L"OtherAddress", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The OtherAddress is not null")); 
    delete winC;
    
    //
    // remove one work tel number
    //
    VCard.reset();
    VCard.reset();
    VCard.append("BEGIN:VCARD\r\n");
    VCard.append("VERSION:2.1\r\n");
    VCard.append("N:Paperino;Paolino;Middle;Mr.;suffix\r\n");
    VCard.append("TEL;WORK;FAX:4444444444\r\n");
    VCard.append("TEL;VOICE;WORK:22222222222\r\n");
    VCard.append("TEL;VOICE;WORK:333333333333\r\n");
    VCard.append("TEL;CAR;VOICE:66666666666\r\n");
    VCard.append("EMAIL;INTERNET;HOME:paolino2@dow.com\r\n");
    VCard.append("END:VCARD");

    item->setData(VCard.c_str(), VCard.length());
    winC = updateOutlookItem(ss, item);
    WIN_ASSERT_NOT_NULL(winC, TEXT("The item retrieved from the backend is null."));
    
    winC->getProperty(L"HomeAddress", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The HomeAddress is not null")); 
    winC->getProperty(L"BusinessAddress", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The BusinessAddress is not null")); 
    delete winC;
    
    //
    // delete the test item
    // 
    ret = ss->deleteItem(*item);
    WIN_ASSERT_EQUAL(ret, 200, TEXT("delete item is not correct"));

    delete item;
    delete ss;
    
}
END_TEST

/**
* The test must be executed with an already installed Outlook client
*/
BEGIN_TEST(WinContactPhotoIsTheSame)
{
    int ret = 0;
    SyncItem* item = NULL;
    WCHAR* internalKey = NULL;
    wstring propValue; 

    WindowsSyncSource* ss = createContactWindowsSyncSource();   
    WIN_ASSERT_NOT_NULL(ss, TEXT("The syncSource is null."));

    StringBuffer VCard;
    VCard.append("BEGIN:VCARD\r\n");
    VCard.append("VERSION:2.1\r\n");
    VCard.append("N:Paperino;Paolino;Middle;Mr.;suffix\r\n");
    VCard.append("BDAY:20100901\r\n");
    VCard.append("NOTE:\r\n");
    VCard.append("TEL;WORK;FAX:4444444444\r\n");
    VCard.append("TEL;VOICE;WORK:22222222222\r\n");
    VCard.append("TEL;VOICE;WORK:333333333333\r\n");
    VCard.append("TEL;CAR;VOICE:66666666666\r\n");
    VCard.append("CATEGORIES:\r\n");
    VCard.append("TEL;WORK;PREF:777777777777\r\n");
    VCard.append("FN:Paperino, Paolino Middle\r\n");
    VCard.append("EMAIL;INTERNET:paolino@doe.com\r\n");
    VCard.append("EMAIL;INTERNET;HOME:paolino2@dow.com\r\n");
    VCard.append("EMAIL;INTERNET;WORK:paolino3@dor.com\r\n");
    VCard.append("TITLE:JobTitle\r\n");
    VCard.append("TEL;VOICE;HOME:8888888888888\r\n");
    VCard.append("TEL;VOICE;HOME:999999999999\r\n");
    VCard.append("TEL;HOME;FAX:000000000\r\n");
    VCard.append("URL;HOME:\r\n");
    VCard.append("TEL;CELL:1212121212121\r\n");
    VCard.append("NICKNAME:nickname\r\n");
    VCard.append("TEL;FAX:14141414141\r\n");
    VCard.append("TEL;VOICE:131313131313\r\n");
    VCard.append("TEL;PAGER:151515151\r\n");
    VCard.append("TEL;PREF;VOICE:16161616161\r\n");
    VCard.append("ROLE:profession\r\n");
    VCard.append("URL:www.web.com\r\n");
    VCard.append("ORG:Company;department;office\r\n");
    VCard.append("ADR;HOME:;;Via Riviera 55;pavia;pv;27100;Italy\r\n");
    VCard.append("ADR:;;Via Naples 23;milano;mi;3993;Italy\r\n");
    VCard.append("ADR;WORK:;;Via Roma;New york;NY;20111;Italy\r\n");    
    VCard.append(" /9j/4AAQSkZJRgABAQEAYABgAAD/2wBDAAYEBQYFBAYGBQYHBwYIChAKCgkJChQODwwQFxQY\r\n");
    VCard.append(" GBcUFhYaHSUfGhsjHBYWICwgIyYnKSopGR8tMC0oMCUoKSj/2wBDAQcHBwoIChMKChMoGhYa\r\n");
    VCard.append(" KCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCj/wAAR\r\n");
    VCard.append(" CAA2AEgDASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAA\r\n");
    VCard.append(" AgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkK\r\n");
    VCard.append(" FhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWG\r\n");
    VCard.append(" h4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl\r\n");
    VCard.append(" 5ufo6erx8vP09fb3+Pn6/8QAHwEAAwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREA\r\n");
    VCard.append(" AgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYk\r\n");
    VCard.append(" NOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOE\r\n");
    VCard.append(" hYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk\r\n");
    VCard.append(" 5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwCfxl4r1qy8S69Aut61bW9vqMysHmlCRxu7\r\n");
    VCard.append(" BSrKeFHbsBjpivOm8d+KAQreJtdSVE8uRW1CbnGSJFGemB+tbXxIef8A4THxCDaXAjF/dees\r\n");
    VCard.append(" t1tjuYhMxG0Z4xXnN9dwNAkTTJIAodFlRi6jkbAfoc14+HUuaTu3r/X9Py6n0s3FQjdJaLdW\r\n");
    VCard.append(" v99vw8007pnZaf418TSW0lxceK9eUjaVRb6U9eSuN3pjmki8beKhOF/4SnWAQcfNqExAx1zg\r\n");
    VCard.append(" +tcL9qMSxqc+WoABxt8zBwMep55zVyzlclwxJl+6Y2kC/MTnp3rrcZK7uYQ5FJJr+v8AhtvT\r\n");
    VCard.append(" RbHoFn4z8Qmf7NL4p1tVbBybyRmLHsDngZ7VqP4p8SxHYfEOsEoNzn7bIck9FHzVx8Yit7SL\r\n");
    VCard.append(" M8Ksko3nAZ93XgDqK1IyWRbi4iaNC2Y1I+82eGPevLrTle6bsfTYKEGuWpFX7WX9fgj0f4Z+\r\n");
    VCard.append(" JvEUt4H1PWLtrQnGLudmLk9hk9q91tpJpFBM0hyP7xr5d8MTeVqcVzK4Yq3yjzAQzemK+gdD\r\n");
    VCard.append(" 1rdax+YArY5Gc0YfFqM3GbPJzvDLn5qaXyRuam1wIG8ueVTjqrkUVXudQR4D0NFTipxlO6kz\r\n");
    VCard.append(" yKMZRjZo+WvippZi8c+JmW1VojcTytLdTEj5nJJQdsE4xXBXxaaCZ/tE5DwxmIvags4UjO0j\r\n");
    VCard.append(" oB616f43uGfxX4kgtrW5ucalcfajcSfucea2wY9B1Fcrb6LfraahdWs4lkgk+zs6yARpCwyy\r\n");
    VCard.append(" qOua0jX9m37R6p/13td/gep7HmpLk6r9H2tffzfmcdNZt5rSIXO9TLBJcDHnJ0IUdM5rRsra\r\n");
    VCard.append(" Ozk+z3Bt4wrhBBeptYbh1LD0qe408vYs9nZSXdhM4jiTBae2VTyfYHnH0rRsoEk3ix1KKW3N\r\n");
    VCard.append(" zGyJqEW5gMdc+ma6Z17x3/r8bf1YzpUXz2S/p3+9PTXW6tcdpVu0LL9k/siEnMRl3FiW9Rxk\r\n");
    VCard.append(" H0NbMiAW28CSWVsh5X65H931qIfZ7JmkkTSkZZyCYY9xX8PQ1oWn+mlfJikmbcWAb5Vj+ntX\r\n");
    VCard.append(" l1qjb5+n9dT6DCwVFa6eX/AsjS8MbIriF7i3jRzgRsyEkfX0r2rSbLzbdCWRmxztrhPDOm2U\r\n");
    VCard.append(" yhZoZI7t1w+GO0H2rfFvq2hvvtibi367T1FdGCnCrHmWp5+Omq0rRdn5nWNpDMuASPoaKqaR\r\n");
    VCard.append(" 4rgmwlwDFJ3Vxiiu7kpvc8Wf1iDs0eA/EMMvjTXZb65kNs+pzq1vFxuVZGxn9K5qNvJSIXu+\r\n");
    VCard.append(" DTZBJNDHGfmJ7Z716r460xpte1u6KIu28lC7x/DvOT+NcI2kXUrtdxx+bDGhTa3Rc+lcDqxb\r\n");
    VCard.append(" l2Tt218vPzPYpwXs1UjLay/K9328kUI9X1HTrm8acGKWdIpIxF91VxjJH40+U3Mas1pcW062\r\n");
    VCard.append(" bY5XDMj8nP0P8q0zp8ttfSK7ec4gCHcP4SOlX38LfZ7dJ4Y1+WMEn+9+FYynSi4tpJy28+nr\r\n");
    VCard.append(" tp8zoVNpqMpb3afXv6f8BGRFFPE2w2tnOEjwzIOSp5/MV19hokl3psN1bOHwmJI06g0umaEl\r\n");
    VCard.append(" 9BHPbRgMRtkUcV13hq2TS5mjePaH4J9az9lOp70d4vVF1a3IrxfvL0Mu0uHtliVt8ciDAY9f\r\n");
    VCard.append(" xrttC1qO5QRXON386XU9Dh1C33KAGxwwrjbizu9Mnw6ttB4YV2fVuR89LR/gedy0sTdrSR6P\r\n");
    VCard.append(" PolpdjeEUk98UVh+GdfLFYZzg9j60V0xrRt76szz6ixFKXKmc349zNq17HnG+7kB+gY0+Cwj\r\n");
    VCard.append(" t9EWNAP3h5OKKK4cVBLAysuv6nYm44KFjMnsEGtIGwQVCn3Fbt1pccKrCrHZ5fGaKKzx0I+x\r\n");
    VCard.append(" oytqmjbEzkvZ2ZmeGv8ARL+SEcrursrqySWLfwDiiiu+OlZ27E4ttVE0P0K9dJTbv8wHQ1t3\r\n");
    VCard.append(" enw3MeWUc+tFFdrSaPNxPuVLx0OcvdHht2LxYUj0ooorwsVJqpodMJykk2z/2Q==\r\n");
    VCard.append("\r\n");        
    VCard.append("X-ANNIVERSARY:20100902\r\n");
    VCard.append("X-FUNAMBOL-BILLINGINFO:\r\n");
    VCard.append("TEL;X-FUNAMBOL-CALLBACK:5555555555\r\n");
    VCard.append("X-FUNAMBOL-CHILDREN:\r\n");
    VCard.append("X-FUNAMBOL-COMPANIES:\r\n");
    VCard.append("X-FUNAMBOL-FOLDER:DEFAULT_FOLDER\r\n");
    VCard.append("X-FUNAMBOL-HOBBIES:\r\n");
    VCard.append("EMAIL;INTERNET;HOME;X-FUNAMBOL-INSTANTMESSENGER:IMaddress\r\n");
    VCard.append("X-FUNAMBOL-INITIALS:P.M.P.\r\n");
    VCard.append("X-FUNAMBOL-LANGUAGES:\r\n");
    VCard.append("X-MANAGER:managerName\r\n");
    VCard.append("X-FUNAMBOL-MILEAGE:\r\n");
    VCard.append("X-FUNAMBOL-ORGANIZATIONALID:\r\n");
    VCard.append("TEL;X-FUNAMBOL-RADIO:1717171717\r\n");
    VCard.append("X-SPOUSE:spuse\r\n");
    VCard.append("X-FUNAMBOL-SUBJECT:Paolino Middle Paperino suffix\r\n");
    VCard.append("TEL;X-FUNAMBOL-TELEX:1181818181818\r\n");
    VCard.append("X-FUNAMBOL-YOMICOMPANYNAME:\r\n");
    VCard.append("X-FUNAMBOL-YOMIFIRSTNAME:\r\n");
    VCard.append("X-FUNAMBOL-YOMILASTNAME:\r\n");
    VCard.append("END:VCARD");

    item = new SyncItem(TEXT("GUID"));
    item->setData(VCard.c_str(), VCard.length());
    
    // insert test item
    ret = ss->addItem(*item);
    WIN_ASSERT_EQUAL(ret, 201, TEXT("Adding item is not correct"));    
    internalKey = (WCHAR*)item->getKey();
    
    //
    // remove work and car tel number
    //
    VCard.reset();
    VCard.append("BEGIN:VCARD\r\n");
    VCard.append("VERSION:2.1\r\n");
    VCard.append("N:Paperino;Paolino;Middle;Mr.;suffix\r\n");    
    VCard.append("PHOTO;ENCODING=b;TYPE=JPEG:\r\n");
    VCard.append(" /9j/4AAQSkZJRgABAQEAYABgAAD/2wBDAAYEBQYFBAYGBQYHBwYIChAKCgkJChQODwwQFxQY\r\n");
    VCard.append(" GBcUFhYaHSUfGhsjHBYWICwgIyYnKSopGR8tMC0oMCUoKSj/2wBDAQcHBwoIChMKChMoGhYa\r\n");
    VCard.append(" KCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCj/wAAR\r\n");
    VCard.append(" CAA2AEgDASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAA\r\n");
    VCard.append(" AgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkK\r\n");
    VCard.append(" FhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWG\r\n");
    VCard.append(" h4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl\r\n");
    VCard.append(" 5ufo6erx8vP09fb3+Pn6/8QAHwEAAwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREA\r\n");
    VCard.append(" AgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYk\r\n");
    VCard.append(" NOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOE\r\n");
    VCard.append(" hYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk\r\n");
    VCard.append(" 5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwCfxl4r1qy8S69Aut61bW9vqMysHmlCRxu7\r\n");
    VCard.append(" BSrKeFHbsBjpivOm8d+KAQreJtdSVE8uRW1CbnGSJFGemB+tbXxIef8A4THxCDaXAjF/dees\r\n");
    VCard.append(" t1tjuYhMxG0Z4xXnN9dwNAkTTJIAodFlRi6jkbAfoc14+HUuaTu3r/X9Py6n0s3FQjdJaLdW\r\n");
    VCard.append(" v99vw8007pnZaf418TSW0lxceK9eUjaVRb6U9eSuN3pjmki8beKhOF/4SnWAQcfNqExAx1zg\r\n");
    VCard.append(" +tcL9qMSxqc+WoABxt8zBwMep55zVyzlclwxJl+6Y2kC/MTnp3rrcZK7uYQ5FJJr+v8AhtvT\r\n");
    VCard.append(" RbHoFn4z8Qmf7NL4p1tVbBybyRmLHsDngZ7VqP4p8SxHYfEOsEoNzn7bIck9FHzVx8Yit7SL\r\n");
    VCard.append(" M8Ksko3nAZ93XgDqK1IyWRbi4iaNC2Y1I+82eGPevLrTle6bsfTYKEGuWpFX7WX9fgj0f4Z+\r\n");
    VCard.append(" JvEUt4H1PWLtrQnGLudmLk9hk9q91tpJpFBM0hyP7xr5d8MTeVqcVzK4Yq3yjzAQzemK+gdD\r\n");
    VCard.append(" 1rdax+YArY5Gc0YfFqM3GbPJzvDLn5qaXyRuam1wIG8ueVTjqrkUVXudQR4D0NFTipxlO6kz\r\n");
    VCard.append(" yKMZRjZo+WvippZi8c+JmW1VojcTytLdTEj5nJJQdsE4xXBXxaaCZ/tE5DwxmIvags4UjO0j\r\n");
    VCard.append(" oB616f43uGfxX4kgtrW5ucalcfajcSfucea2wY9B1Fcrb6LfraahdWs4lkgk+zs6yARpCwyy\r\n");
    VCard.append(" qOua0jX9m37R6p/13td/gep7HmpLk6r9H2tffzfmcdNZt5rSIXO9TLBJcDHnJ0IUdM5rRsra\r\n");
    VCard.append(" Ozk+z3Bt4wrhBBeptYbh1LD0qe408vYs9nZSXdhM4jiTBae2VTyfYHnH0rRsoEk3ix1KKW3N\r\n");
    VCard.append(" zGyJqEW5gMdc+ma6Z17x3/r8bf1YzpUXz2S/p3+9PTXW6tcdpVu0LL9k/siEnMRl3FiW9Rxk\r\n");
    VCard.append(" H0NbMiAW28CSWVsh5X65H931qIfZ7JmkkTSkZZyCYY9xX8PQ1oWn+mlfJikmbcWAb5Vj+ntX\r\n");
    VCard.append(" l1qjb5+n9dT6DCwVFa6eX/AsjS8MbIriF7i3jRzgRsyEkfX0r2rSbLzbdCWRmxztrhPDOm2U\r\n");
    VCard.append(" yhZoZI7t1w+GO0H2rfFvq2hvvtibi367T1FdGCnCrHmWp5+Omq0rRdn5nWNpDMuASPoaKqaR\r\n");
    VCard.append(" 4rgmwlwDFJ3Vxiiu7kpvc8Wf1iDs0eA/EMMvjTXZb65kNs+pzq1vFxuVZGxn9K5qNvJSIXu+\r\n");
    VCard.append(" DTZBJNDHGfmJ7Z716r460xpte1u6KIu28lC7x/DvOT+NcI2kXUrtdxx+bDGhTa3Rc+lcDqxb\r\n");
    VCard.append(" l2Tt218vPzPYpwXs1UjLay/K9328kUI9X1HTrm8acGKWdIpIxF91VxjJH40+U3Mas1pcW062\r\n");
    VCard.append(" bY5XDMj8nP0P8q0zp8ttfSK7ec4gCHcP4SOlX38LfZ7dJ4Y1+WMEn+9+FYynSi4tpJy28+nr\r\n");
    VCard.append(" tp8zoVNpqMpb3afXv6f8BGRFFPE2w2tnOEjwzIOSp5/MV19hokl3psN1bOHwmJI06g0umaEl\r\n");
    VCard.append(" 9BHPbRgMRtkUcV13hq2TS5mjePaH4J9az9lOp70d4vVF1a3IrxfvL0Mu0uHtliVt8ciDAY9f\r\n");
    VCard.append(" xrttC1qO5QRXON386XU9Dh1C33KAGxwwrjbizu9Mnw6ttB4YV2fVuR89LR/gedy0sTdrSR6P\r\n");
    VCard.append(" PolpdjeEUk98UVh+GdfLFYZzg9j60V0xrRt76szz6ixFKXKmc349zNq17HnG+7kB+gY0+Cwj\r\n");
    VCard.append(" t9EWNAP3h5OKKK4cVBLAysuv6nYm44KFjMnsEGtIGwQVCn3Fbt1pccKrCrHZ5fGaKKzx0I+x\r\n");
    VCard.append(" oytqmjbEzkvZ2ZmeGv8ARL+SEcrursrqySWLfwDiiiu+OlZ27E4ttVE0P0K9dJTbv8wHQ1t3\r\n");
    VCard.append(" enw3MeWUc+tFFdrSaPNxPuVLx0OcvdHht2LxYUj0ooorwsVJqpodMJykk2z/2Q==\r\n");
    VCard.append("\r\n");        
    VCard.append("END:VCARD");
    
    // update item
    item->setData(VCard.c_str(), VCard.length());
    WinContact* winC = updateOutlookItem(ss, item);
    WIN_ASSERT_NOT_NULL(winC, TEXT("The item retrieved from the backend is null."));
    
    // check some modifications

    winC->getProperty(L"Email1Address", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The Email2Address is not null"));
    winC->getProperty(L"Email3Address", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The Email3Address is not null"));  
    winC->getProperty(L"Email2Address", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The Email2Address is not null"));
    winC->getProperty(L"Birthday", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The Birthday is not null")); 
    winC->getProperty(L"OtherAddress", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The OtherAddress is not null")); 
    winC->getProperty(L"MobileTelephoneNumber", propValue);
    WIN_ASSERT_TRUE(propValue.empty(), TEXT("The MobileTelephoneNumber is not null"));
    winC->getProperty(L"Photo", propValue);
    WIN_ASSERT_TRUE(!propValue.empty(), TEXT("The BusinessAddress is not null"));         

    delete winC;
   
    //
    // delete the test item
    // 
    ret = ss->deleteItem(*item);
    WIN_ASSERT_EQUAL(ret, 200, TEXT("delete item is not correct"));

    delete item;
    
}
END_TEST