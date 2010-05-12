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

#include "outlook/ClientRecipient.h"
#include "outlook/ClientApplication.h"

ClientRecipient::ClientRecipient() {
}

ClientRecipient::ClientRecipient(const std::wstring & namedEmail) {
    parseNamedEmail(namedEmail);
    pSafeRecipient = NULL;
}

ClientRecipient::ClientRecipient(const std::wstring & name, const std::wstring & email) {
    setName(name);
    setEmail(email);
    pSafeRecipient = NULL;
}

ClientRecipient::ClientRecipient(Redemption::ISafeRecipientPtr recPtr) {
    pSafeRecipient = recPtr;
    read();
}

std::wstring ClientRecipient::getName() const {
    return name;
}

std::wstring ClientRecipient::getEmail() const {
    return email;
}

std::wstring ClientRecipient::getNamedEmail() const {
    if (name.length() > 0) {
        return name + L" <" + email + L">";
    } else {
        return email;
    }
}

int ClientRecipient::getStatus() const {
    return status;
}

void ClientRecipient::setName(const std::wstring & value) {
    name = value;
}

void ClientRecipient::setEmail(const std::wstring & value) {
    email = value;
}

bool ClientRecipient::read() {
    name = L"";
    email = L"";
    bool success = false;

    if (pSafeRecipient)
    {
        // The '!(!' is not a mistake.  operator! is overloaded, but bool conversion is not
        if (!(!pSafeRecipient->GetName()) && wcscmp((WCHAR*)pSafeRecipient->GetName(), L"Unknown"))
        {
            if (!(!pSafeRecipient->GetAddress())) {

                if (!(!pSafeRecipient->GetName())) {
                    name = (WCHAR*)pSafeRecipient->GetName();
                }
                email = (WCHAR*)pSafeRecipient->GetAddress();
                success = true;

            } else if (!(!pSafeRecipient->GetName())) {

                // The name can sometimes be the combination of name and email
                std::wstring value = (WCHAR*)pSafeRecipient->GetName();
                success = parseNamedEmail(value);

            }
        }

        // TODO : Needs testing
        /*
        if (success) {
            fixExchangeAddress();
        }
        */

        //status = pSafeRecipient->MeetingResponseStatus;
        //status = pSafeRecipient->GetMeetingResponseStatus();
        //status = pSafeRecipient->GetTrackingStatus();
    }

    return success;
}

bool ClientRecipient::parseNamedEmail(const std::wstring & namedEmail) {

    bool success = false;

    size_t npos = std::wstring::npos;
    size_t leftParen = namedEmail.find_last_of(L'(');
    size_t rightParen = namedEmail.find_last_of(L')');

    // If its not formatted in the way we expect, we cant handle it
    if (leftParen != npos && rightParen != npos && rightParen == namedEmail.size()-1) {
        name = namedEmail.substr(0, leftParen);
        email = namedEmail.substr(leftParen+1,rightParen-leftParen-1);
        success = true;
    } else {
        size_t leftParen = namedEmail.find_last_of(L'<');
        size_t rightParen = namedEmail.find_last_of(L'>');
        // If its not formatted in the way we expect, we cant handle it
        if (leftParen != npos && rightParen != npos && rightParen == namedEmail.size()-1) {
            name = namedEmail.substr(0, leftParen);
            email = namedEmail.substr(leftParen+1,rightParen-leftParen-1);
            success = true;
        }
    }

    return success;
}

void ClientRecipient::fixExchangeAddress() {

    std::wstring result = email;

    ClientApplication * app = ClientApplication::getInstance(false);
    if (app != NULL) {
        std::wstring temp = app->getSMTPfromEX(email);
        if (temp.compare(L"") != 0) {
            email = temp;
        }
    }
}
