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

#ifndef INCL_SIF_FIELDS
#define INCL_SIF_FIELDS

/** @cond OLPLUGIN */
/** @addtogroup SIFFields */
/** @{ */

#include "base/fscapi.h"
#include "spdm/constants.h"


//////////////////////////////////////////////////////////////
/// SIF APPOINTMENT PROPERTIES
//////////////////////////////////////////////////////////////
/// @note
/// - "AllDayEvent" MUST be set AFTER the "Start" and "End" fields to be correctly managed
/// - RecPattern properties MUST be set AFTER the other properties
///
static WCHAR* appointmentFields[] = {

    {L"Start"                              },                // Returns or sets the starting date and time for the appointment or journal entry. Use only in calendar
    {L"End"                                },                // Returns or sets the end date and time of an appointment or journal entry. Use only on calendar
    {L"AllDayEvent"                        },                // True if the appointment is an all-day event (as opposed to a specified time). Corresponds to the All day event check box on the Appointment page of an AppointmentItem - MUST be set AFTER "Start" and "End" fields.
    {L"BillingInformation"                 },                // Returns or sets the billing information associated with the Outlook item. This is a free-form text field
    {L"Body"                               },                // Returns or sets the clear-text body of the Outlook item.
    {L"BusyStatus"                         },                // Returns or sets the busy status of the user for the appointment. Can be one of the following OlBusyStatus constants: olBusy(2), olFree(0), olOutOfOffice(3), or olTentative(1). 
    {L"Categories"                         },                // Returns or sets the categories assigned to the Outlook item. 
    {L"Companies"                          },                // Returns or sets the names of the companies associated with the Outlook item. This is a free-form text field
    {L"Importance"                         },                // Returns or sets the relative importance level for the Outlook item. Can be one of the following OlImportance constants: olImportanceHigh(2), olImportanceLow(0), or olImportanceNormal(1). This property corresponds to the MAPI property PR_IMPORTANCE. 
    {L"IsRecurring"                        },                // True if the appointment or task is a recurring appointment or task. When the GetRecurrencePattern method is used with an AppointmentItem or TaskItem object, this property is set to True
    {L"Location"                           },                // Returns or sets the specific office location (for example, Building 1 Room 1 or Suite 123) for the appointment. This property corresponds to the MAPI property PR_OFFICE_LOCATION
    {L"MeetingStatus"                      },                // OlRemoteStatus can be one of these OlRemoteStatus constants.
    {L"Mileage"                            },                // Returns or sets a String representing the mileage for an item. This is a free-form string field and can be used to store mileage information associated with the item (for example, 100 miles documented for an appointment, contact, or task) for purposes of reimbursement.
    {L"NoAging"                            },    // need?    // True to not age the Outlook item. 
//  {L"OptionalAttendees"                  },    // need?    // Returns or sets a String representing the display string of optional attendees names for the appointment. This property corresponds to the MAPI property PR_DISPLAY_CC. Read/write
    {L"ReminderMinutesBeforeStart"         },                // Returns or sets the number of minutes the reminder should occur prior to the start of the appointment
    {L"ReminderSet"                        },                // True if a reminder has been set for this appointment, mail item or task. 
    {L"ReminderSoundFile"                  },                // Returns or sets the path and filename of the sound file to play when the reminder occurs for the appointment or task. This property is only valid if the ReminderOverrideDefault and ReminderPlaySound properties are set to True
    {L"ReplyTime"                          },                // Returns or sets a Date indicating the reply time for the appointment. Read/write
    {L"Sensitivity"                        },                // Returns or sets the sensitivity for the Outlook item. Can be one of the following OlSensitivity constants: olConfidential(3), olNormal(0), olPersonal(1), or olPrivate(2). This property corresponds to the MAPI property PR_SENSITIVITY
    {L"Subject"                            },                // Returns or sets the subject for the Outlook item. This property corresponds to the MAPI property PR_SUBJECT. The Subject property is the default property for Outlook items.
//  {L"UnRead"                             },    // need?    // True if the Outlook item has not been opened (read).                             // 
    {NULL}

};


////////////////////////////////////
/// RECURRENCE FIELDS
////////////////////////////////////
static WCHAR* recurrenceFields[] = {

    {L"RecurrenceType"                     },                // Returns or set a RecurrenceType. values are orRecursDaily...
    {L"Interval"                           },                // Is the interval of the recurrence. If RecurrenceType is olRecursDaily, event occurs every <Interval> day. If olRecursWeekly, event occurs every <inteval> week..
    {L"MonthOfYear"                        },                // Returns or sets the month of year.
    {L"DayOfMonth"                         },                // Returns or sets the single day of the month from 1 to 31.
    {L"DayOfWeekMask"                      },                // The combination days of the week constants (i. e. event recurring on Monday and Wednesday. The DayOfWeekMask should be olMonday + olWednesday)
    {L"Instance"                           },                // Returns or sets the ordinal number of the day, week, month.
    {L"PatternStartDate"                   },                // Returns or sets the start date of the recurrence.
    {L"NoEndDate"                          },                // True if there is no end date. True is 1.
    {L"PatternEndDate"                     },                // Returns or sets the end date of the recurrence.
    {L"Occurrences"                        },                // Return or sets the number of the occurrences of the recurrence.
    {NULL}

};


////////////////////////////////////
/// APPOINTMENT EXCEPTION PROPERTIES (fields inside <ExAppointent>)
/// "OriginalDate" is inside <Exception> but outside <ExAppointment>
////////////////////////////////////
static WCHAR* exAppointmentFields[] = {

    {L"Subject"                            },                // The modified Subject
    {L"Body"                               },                // The modified Body
    {L"Location"                           },                // The modified Location
    {L"Start"                              },                // The modified Start date of appointment  (UTC)
    {L"End"                                },                // The modified End date of appointment    (UTC)
    {L"AllDayEvent"                        },                // The modified AllDayEvent flag           (0 - 1)
    {L"BusyStatus"                         },                // The modified BusyStatus                 (olFree 0 - olTentative 1 - olBusy 2 - olOutOfOffice 3)
    {L"ReminderSet"                        },                // The modified ReminderSet                (only Outlook)
    {L"ReminderMinutesBeforeStart"         },                // The modified ReminderMinutesBeforeStart (only Outlook)
    {L"Importance"                         },                // The modified Importance                 (only Outlook)
    {NULL}

};




//////////////////////////////////////////////////////////////
/// SIF CONTACT PROPERTIES
//////////////////////////////////////////////////////////////
/// "FileAs" MUST be set after name fields, otherwise Outlook will 
/// automatically fill it with a name field. So it's placed in last position.
static WCHAR* contactFields[] = {

    {L"Anniversary"                        },                // Returns or sets the anniversary date for the contact
    {L"AssistantName"                      },                // Returns or sets the name of the person who is the assistant for the contact. Corresponds to the Assistant's name: box on the Details page of a ContactItem.
    {L"AssistantTelephoneNumber"           },                // Returns or sets the telephone number of the person who is the assistant for the contact
    {L"BillingInformation"                 },                // Returns or sets the billing information associated with the Outlook item. This is a free-form text field
    {L"Birthday"                           },                // Returns or sets the birthday for the contact.Corresponds to the Birthday: field on the Details page of a ContactItem. 
    {L"Body"                               },                // Returns or sets the clear-text body of the Outlook item.
    {L"Business2TelephoneNumber"           },                // Returns or sets the second business telephone number for the contact. 
    {L"BusinessAddressCity"                },                // Returns or sets the city name portion of the business address for the contact
    {L"BusinessAddressCountry"             },                // Returns or sets the country code portion of the business address for the contact
    {L"BusinessAddressPostalCode"          },                // Returns or sets the postal code (zip code) portion of the business address for the contact
    {L"BusinessAddressPostOfficeBox"       },                // Returns or sets the post office box number portion of the business address for the contact
    {L"BusinessAddressState"               },                // Returns or sets the state code portion of the business address for the contact
    {L"BusinessAddressStreet"              },                // Returns or sets the street address portion of the business address for the contact
    {L"BusinessFaxNumber"                  },                // Returns or sets the business fax number for the contact
    {L"BusinessTelephoneNumber"            },                // Returns or sets the first business telephone number for the contact
//  {L"BusinessWebPage"                    },    // Duplicated: inside Outlook it's the same of "WebPage" -> removed since 6.5.2
    {L"CallbackTelephoneNumber"            },                // Returns or sets the callback telephone number for the contact
    {L"CarTelephoneNumber"                 },                // Returns or sets the car telephone number for the contact
    {L"Categories"                         },                // Returns or sets the categories assigned to the Outlook item. 
    {L"Children"                           },                // Returns or sets the names of the children of the contact
    {L"Companies"                          },                // Returns or sets the names of the companies associated with the Outlook item. This is a free-form text field
    {L"CompanyMainTelephoneNumber"         },                // Returns or sets the company main telephone number for the contact
    {L"CompanyName"                        },                // Returns or sets the company name for the contact
    {L"ComputerNetworkName"                },                // Returns or sets the name of the computer network for the contact
    {L"Department"                         },                // Returns or sets the department name for the contact
    {L"Email1Address"                      },                // Returns or sets a String representing the e-mail address of the first e-mail entry for the contact.
    {L"Email1AddressType"                  },                // Returns or sets a String representing the address type (such as EX or SMTP) of the first e-mail entry for the contact. This is a free-form text field, but it must match the actual type of an existing mail transport.
    {L"Email2Address"                      },                // Returns or sets the e-mail address of the second e-mail entry for the contact
    {L"Email2AddressType"                  },                // Returns or sets a String representing the address type (such as EX or SMTP) of the second e-mail entry for the contact. This is a free-form text field, but it must match the actual type of an existing mail transport.
    {L"Email3Address"                      },                // Returns or sets the e-mail address of the third e-mail entry for the contact
    {L"Email3AddressType"                  },                // Returns or sets a String representing the address type (such as EX or SMTP) of the third e-mail entry for the contact. This is a free-form text field, but it must match the actual type of an existing mail transport.
    {L"FirstName"                          },                // Returns or sets the first name for the contact.
    {L"Gender"                             },                // Returns or sets the gender of the contact. Can be one of the following OlGender constants: olFemale(1), olMale(2), or olUnspecified(0). 
    {L"Hobby"                              },                // Returns or sets the hobby for the contact
    {L"Home2TelephoneNumber"               },                // Returns or sets the second home telephone number for the contact
    {L"HomeAddressCity"                    },                // Returns or sets the city portion of the home address for the contact
    {L"HomeAddressCountry"                 },                // Returns or sets the country portion of the home address for the contact
    {L"HomeAddressPostalCode"              },                // Returns or sets the postal code portion of the home address for the contact
    {L"HomeAddressPostOfficeBox"           },                // Returns or sets the post office box number portion of the home address for the contact
    {L"HomeAddressState"                   },                // Returns or sets the state portion of the home address for the contact
    {L"HomeAddressStreet"                  },                // Returns or sets the street portion of the home address for the contact
    {L"HomeFaxNumber"                      },                // Returns or sets the home fax number for the contact
    {L"HomeTelephoneNumber"                },                // Returns or sets the first home telephone number for the contact
    {L"HomeWebPage"                        },                // Returns or sets the URL of the Home Web page for the contact
    {L"IMAddress"                          },                // Returns or sets a String that represents a contact's Microsoft Instant Messenger address.
    {L"Importance"                         },                // Returns or sets the relative importance level for the Outlook item. Can be one of the following OlImportance constants: olImportanceHigh(2), olImportanceLow(0), or olImportanceNormal(1). This property corresponds to the MAPI property PR_IMPORTANCE. 
    {L"Initials"                           },                // Returns or sets the initials for the contact
    {L"JobTitle"                           },                // Returns or sets the job title for the contact
    {L"Language"                           },                // Returns or sets the language for the contact
    {L"LastName"                           },                // Returns or sets the last name for the contact
    {L"MailingAddress"                     },                // Returns or sets the full, unparsed selected mailing address for the contact
    {L"ManagerName"                        },                // Returns or sets the manager name for the contact
    {L"MiddleName"                         },                // Returns or sets a String representing the middle name for the contact.This property is parsed from the FullName property, but may be changed or entered independently should it be parsed incorrectly. Note that any such changes or entries to this property will be overwritten by any subsequent changes of entries to FullName.
    {L"Mileage"                            },                // Returns or sets a String representing the mileage for an item. This is a free-form string field and can be used to store mileage information associated with the item (for example, 100 miles documented for an appointment, contact, or task) for purposes of reimbursement.
    {L"MobileTelephoneNumber"              },                // Returns or sets a String representing the mobile telephone number for the contact.
    {L"NickName"                           },                // Returns or sets a String representing the nickname for the contact.
    {L"OfficeLocation"                     },                // Returns or sets a String specifying the specific office location (for example, Building 1 Room 1 or Suite 123) for the contact. This property corresponds to the MAPI property PR_OFFICE_LOCATION.
    {L"OrganizationalIDNumber"             },                // Returns or sets the organizational ID number for the contact
    {L"OtherAddressCity"                   },                // Returns or sets the city portion of the other address for the contact
    {L"OtherAddressCountry"                },                // Returns or sets the country portion of the other address for the contact
    {L"OtherAddressPostalCode"             },                // Returns or sets the postal code portion of the other address for the contact
    {L"OtherAddressPostOfficeBox"          },                // Returns or sets the post office box portion of the other address for the contact
    {L"OtherAddressState"                  },                // Returns or sets the state portion of the other address for the contact
    {L"OtherAddressStreet"                 },                // Returns or sets the street portion of the other address for the contact
    {L"OtherFaxNumber"                     },                // Returns or sets the other fax number for the contact
    {L"OtherTelephoneNumber"               },                // Returns or sets the other telephone number for the contact
    {L"PagerNumber"                        },                // Returns or sets the pager number for the contact
    {L"Photo"                              },                // The contact's picture (b64)
    {L"PrimaryTelephoneNumber"             },                // Returns or sets the primary telephone number for the contact
    {L"Profession"                         },                // Returns or sets the profession for the contact
    {L"RadioTelephoneNumber"               },                // Returns or sets the radio telephone number for the contact
    {L"Sensitivity"                        },                // Returns or sets the sensitivity for the Outlook item. Can be one of the following OlSensitivity constants: olConfidential(3), olNormal(0), olPersonal(1), or olPrivate(2). This property corresponds to the MAPI property PR_SENSITIVITY
    {L"Spouse"                             },                // Returns or sets the spouse name entry for the contact
    {L"Subject"                            },                // Returns or sets the subject for the Outlook item. This property corresponds to the MAPI property PR_SUBJECT. The Subject property is the default property for Outlook items.
    {L"Suffix"                             },                // Returns or sets the name suffix (such as Jr., III, or Ph.D.) for the contact
    {L"TelexNumber"                        },                // Returns or sets the telex number for the contact
    {L"Title"                              },                // Returns or sets the title for the contact
    {L"WebPage"                            },                // Returns or sets the URL of the Web page for the contact
    {L"YomiCompanyName"                    },                // Returns or sets a String indicating the Japanese phonetic rendering (yomigana) of the company name for the contact
    {L"YomiFirstName"                      },                // Returns or sets a String indicating the Japanese phonetic rendering (yomigana) of the first name for the contact
    {L"YomiLastName"                       },                // Returns or sets a String indicating the Japanese phonetic rendering (yomigana) of the last name for the contact
    {L"FileAs"                             },                // Returns or sets the default keyword string assigned to the contact when it is filed
    {NULL}

};




//////////////////////////////////////////////////////////////
/// SIF MAIL PROPERTIES
//////////////////////////////////////////////////////////////
//static WCHAR* mailFields[] = {
//
//    {L"BCC"                                },                // Returns the display list of blind carbon copy (BCC) names for a MailItem. This property contains the display names only. The Recipients collection should be used to modify the BCC recipients
//    {L"BillingInformation"                 },                // Returns or sets the billing information associated with the Outlook item. This is a free-form text field
//    {L"Body"                               },                // Returns or sets the clear-text body of the Outlook item.
//    {L"Categories"                         },                // Returns or sets the categories assigned to the Outlook item. 
//    {L"CC"                                 },                // Returns the display list of carbon copy (CC) names for a MailItem. This property contains the display names only. The Recipients collection should be used to modify the CC recipients
//    {L"Companies"                          },                // Returns or sets the names of the companies associated with the Outlook item. This is a free-form text field
//    {L"DeferredDeliveryTime"               },                // Returns or sets the date and time the mail message is to be delivered. This property corresponds to the MAPI property PR_DEFERRED_DELIVERY_TIME
//    {L"ExpiryTime"                         },                // Returns or sets the date and time at which the item becomes invalid and can be deleted
//    {L"FlagDueBy"                          },                // Returns or sets the date by which this mail message is due. This property is only valid if the FlagStatus property is also set for the message. This property corresponds to the MAPI property PR_REPLY_TIME
//    {L"FlagRequest"                        },                // Returns or sets the requested action for the mail message. This is a free-form text field. This property is only valid if the FlagStatus property is also set for the message
//    {L"FlagStatus"                         },                // Returns or sets the flag status for the mail message. Can be one of the following OlFlagStatus constants: olFlagComplete(1), olFlagMarked(2), or olNoFlag(0).
//    {L"HTMLBody"                           },                // Returns or sets a String representing the HTML body of the specified item. The HTMLBody property should be an HTML syntax string.Setting the HTMLBody property sets the EditorType property of the item's Inspector to olEditorHTML.Setting the HTMLBody property will always update the Body property immediately.Setting the Body property will clear the contents of the HTMLBody property on HTML aware stores.The EditorType property is not affected when you merely access the Body property of the item (as in MsgBox myItem.Body), but when you reset the Body property (as in myItem.Body = "This is a new body"), the EditorType reverts back to the user's default editor.
//    {L"Importance"                         },                // Returns or sets the relative importance level for the Outlook item. Can be one of the following OlImportance constants: olImportanceHigh(2), olImportanceLow(0), or olImportanceNormal(1). This property corresponds to the MAPI property PR_IMPORTANCE. 
//    {L"Mileage"                            },                // Returns or sets a String representing the mileage for an item. This is a free-form string field and can be used to store mileage information associated with the item (for example, 100 miles documented for an appointment, contact, or task) for purposes of reimbursement.
//    {L"NoAging"                            },                // True to not age the Outlook item. 
//    {L"ReadReceiptRequested"               },                // True if a read receipt has been requested by the sender. This property corresponds to the MAPI property PR_READ_RECEIPT_REQUESTED.
//    {L"ReminderSet"                        },                // True if a reminder has been set for this appointment, mail item or task. 
//    {L"ReminderTime"                       },                // Returns or sets the date and time at which the reminder should occur for this item. 
//    {L"Sensitivity"                        },                // Returns or sets the sensitivity for the Outlook item. Can be one of the following OlSensitivity constants: olConfidential(3), olNormal(0), olPersonal(1), or olPrivate(2). This property corresponds to the MAPI property PR_SENSITIVITY
//    {L"SentOnBehalfOfName"                 },                // Returns the display name for the intended sender of the mail message. This property corresponds to the MAPI property PR_SENT_REPRESENTING_NAME
//    {L"Subject"                            },                // Returns or sets the subject for the Outlook item. This property corresponds to the MAPI property PR_SUBJECT. The Subject property is the default property for Outlook items.
//    {L"To"                                 },                // Returns or sets the semicolon-delimited list of display names for the To recipients for the Outlook item. This property contains the display names only. The To property corresponds to the MAPI property PR_DISPLAY_TO. The Recipients collection should be used to modify this property.
//    {L"UnRead"                             },                // True if the Outlook item has not been opened (read). 
//    {L"VotingOptions"                      },                // Returns or sets a String specifying a delimited string containing the voting options for the mail message.
//    {L"VotingResponse"                     },                // Returns or sets a String specifying the voting response for the mail message. This property is usually set to one of the delimited values returned by the VotingOptions property on a reply to the original message.
//    {NULL}
//};




//////////////////////////////////////////////////////////////
/// SIF TASK PROPERTIES
//////////////////////////////////////////////////////////////
/// @note  "StartDate" property MUST be set before "DueDate", so it's 
///        placed in first position. When setting "StartDate" property for an
///        incoming item, "DueDate" is internally shifted by Outlook if existing
///        (it is done internally in a silent way) and it could lead to errors.
///        So "StartDate" is the first property to be set.
static WCHAR* taskFields[] = {

    {L"StartDate"                          },                // Returns or sets the starting date and time for the task
    {L"ActualWork"                         },                // Returns or sets the actual effort (in minutes) spent on the task
    {L"BillingInformation"                 },                // Returns or sets the billing information associated with the Outlook item. This is a free-form text field
    {L"Body"                               },                // Returns or sets the clear-text body of the Outlook item.
    {L"Categories"                         },                // Returns or sets the categories assigned to the Outlook item. 
    {L"Companies"                          },                // Returns or sets the names of the companies associated with the Outlook item. This is a free-form text field
    {L"Complete"                           },                // True if the task is completed
    {L"DateCompleted"                      },                // Returns or sets the completion date of the task
    {L"DueDate"                            },                // Returns or sets a Date indicating the due date for the task.
    {L"Importance"                         },                // Returns or sets the relative importance level for the Outlook item. Can be one of the following OlImportance constants: olImportanceHigh(2), olImportanceLow(0), or olImportanceNormal(1). This property corresponds to the MAPI property PR_IMPORTANCE. 
    {L"IsRecurring"                        },                // True if the appointment or task is a recurring appointment or task. When the GetRecurrencePattern method is used with an AppointmentItem or TaskItem object, this property is set to True
    {L"Mileage"                            },                // Returns or sets a String representing the mileage for an item. This is a free-form string field and can be used to store mileage information associated with the item (for example, 100 miles documented for an appointment, contact, or task) for purposes of reimbursement.
    {L"PercentComplete"                    },                // Returns or sets the percentage of the task completed at the current date and time
    {L"ReminderSet"                        },                // True if a reminder has been set for this appointment, mail item or task. 
    {L"ReminderSoundFile"                  },                // Returns or sets the path and filename of the sound file to play when the reminder occurs for the appointment or task. This property is only valid if the ReminderOverrideDefault and ReminderPlaySound properties are set to True
    {L"ReminderTime"                       },                // Returns or sets the date and time at which the reminder should occur for this item. 
    {L"Sensitivity"                        },                // Returns or sets the sensitivity for the Outlook item. Can be one of the following OlSensitivity constants: olConfidential(3), olNormal(0), olPersonal(1), or olPrivate(2). This property corresponds to the MAPI property PR_SENSITIVITY
    {L"Status"                             },                // Returns or sets the status for the task. Can be one of the following OlTaskStatus constants: olTaskComplete(2), olTaskDeferred(4), olTaskInProgress(1), olTaskNotStarted(0), or olTaskWaiting(3). 
    {L"Subject"                            },                // Returns or sets the subject for the Outlook item. This property corresponds to the MAPI property PR_SUBJECT. The Subject property is the default property for Outlook items.
    {L"TeamTask"                           },                // True if the task is a team task
    {L"TotalWork"                          },                // Returns or sets the total work for the task
    {NULL}

};




//////////////////////////////////////////////////////////////
/// SIF NOTE PROPERTIES
//////////////////////////////////////////////////////////////
static WCHAR* noteFields[] = {

    {L"Body"                               },                // Returns or sets the clear-text body of the Outlook item.
    {L"Categories"                         },                // Returns or sets the categories assigned to the Outlook item. 
    //{L"CreationTime"                     },  // need?      // Returns the creation time for the Outlook item. This property corresponds to the MAPI property PR_CREATION_TIME
    //{L"Date"                             },  // not used   // Returns the time that the Outlook item was last modified. This property corresponds to the MAPI property LastModificationTime
    {L"Subject"                            },                // Returns or sets the subject for the Outlook item. This property corresponds to the MAPI property PR_SUBJECT. The Subject property is the default property for Outlook items.
    {L"Color"                              },                // Color of note
    {L"Height"                             },                // Height of note
    {L"Width"                              },                // Width of note
    {L"Left"                               },                // Left position of note
    {L"Top"                                },                // Top position of note
    {NULL}
};





//////////////////////////////////////////////////////////////
// ALL PROPERTIES - (supported and not, useful to add future fields...)
//////////////////////////////////////////////////////////////
//
//static WCHAR* allFields[] = {
//
//    {L"Duration"                           },                // olFullItem The entire item has been downloaded.
//    {L"AllDayEvent"                        },                // True if the appointment is an all-day event (as opposed to a specified time). Corresponds to the All day event check box on the Appointment page of an AppointmentItem.
//    {L"Start"                              },                // Returns or sets the starting date and time for the appointment or journal entry. Use only in calendar
//    {L"End"                                },                // Returns or sets the end date and time of an appointment or journal entry. Use only on calendar
//    {L"Account"                            },                // Returns or sets the account for the contact
//    {L"Actions"                            },                // Returns an Actions collection that represents all the available actions for the Outlook item. 
//    {L"ActualWork"                         },                // Returns or sets the actual effort (in minutes) spent on the task
//    {L"AlternateRecipientAllowed"          },                // True if the mail message can be forwarded.
//    {L"Anniversary"                        },                // Returns or sets the anniversary date for the contact
//    {L"Application"                        },                // Returns an Application object that represents the parent application (Microsoft Outlook) for an object
//    {L"AssistantName"                      },                // Returns or sets the name of the person who is the assistant for the contact. Corresponds to the Assistant's name: box on the Details page of a ContactItem.
//    {L"AssistantTelephoneNumber"           },                // Returns or sets the telephone number of the person who is the assistant for the contact
//    {L"Attachments"                        },                // Returns an Attachments object that represents all the attachments for the item.
//    {L"AutoForwarded"                      },                // True if the mail message was automatically forwarded.
//    {L"BCC"                                },                // Returns the display list of blind carbon copy (BCC) names for a MailItem. This property contains the display names only. The Recipients collection should be used to modify the BCC recipients
//    {L"BillingInformation"                 },                // Returns or sets the billing information associated with the Outlook item. This is a free-form text field
//    {L"Birthday"                           },                // Returns or sets the birthday for the contact.Corresponds to the Birthday: field on the Details page of a ContactItem. 
//    {L"Body"                               },                // Returns or sets the clear-text body of the Outlook item.
//    {L"BodyFormat"                         },                // Returns or sets an OlBodyFormat constant indicating the format of the body text. The body text format determines the standard used to display the text of the message. Microsoft Outlook provides three body text format options: Plain Text, Rich Text and HTML.
//    {L"Business2TelephoneNumber"           },                // Returns or sets the second business telephone number for the contact. 
//    {L"BusinessAddress"                    },                // Returns or sets the whole, unparsed business address for the contact. 
//    {L"BusinessAddressCity"                },                // Returns or sets the city name portion of the business address for the contact
//    {L"BusinessAddressCountry"             },                // Returns or sets the country code portion of the business address for the contact
//    {L"BusinessAddressPostalCode"          },                // Returns or sets the postal code (zip code) portion of the business address for the contact
//    {L"BusinessAddressPostOfficeBox"       },                // Returns or sets the post office box number portion of the business address for the contact
//    {L"BusinessAddressState"               },                // Returns or sets the state code portion of the business address for the contact
//    {L"BusinessAddressStreet"              },                // Returns or sets the street address portion of the business address for the contact
//    {L"BusinessFaxNumber"                  },                // Returns or sets the business fax number for the contact
//    {L"BusinessWebPage"                    },                // Returns or sets the URL of the business Web page for the contact
//    {L"BusinessTelephoneNumber"            },                // Returns or sets the first business telephone number for the contact
//    {L"BusyStatus"                         },                // Returns or sets the busy status of the user for the appointment. Can be one of the following OlBusyStatus constants: olBusy(2), olFree(0), olOutOfOffice(3), or olTentative(1). 
//    {L"CallbackTelephoneNumber"            },                // Returns or sets the callback telephone number for the contact
//    {L"CardData"                           },                // Returns or sets a String representing the text of the card data for the task.
//    {L"CarTelephoneNumber"                 },                // Returns or sets the car telephone number for the contact
//    {L"Categories"                         },                // Returns or sets the categories assigned to the Outlook item. 
//    {L"CC"                                 },                // Returns the display list of carbon copy (CC) names for a MailItem. This property contains the display names only. The Recipients collection should be used to modify the CC recipients
//    {L"Children"                           },                // Returns or sets the names of the children of the contact
//    {L"Class"                              },                // Returns an OlObjectClass constant indicating the object's class. Read-only
//    {L"Companies"                          },                // Returns or sets the names of the companies associated with the Outlook item. This is a free-form text field
//    {L"CompanyAndFullName"                 },                // Returns a String representing the concatenated company name and full name for the contact
//    {L"CompanyLastFirstNoSpace"            },                // Returns a String representing the company name for the contact followed by the concatenated last name, first name, and middle name with no space between the last and first names. This property is parsed from the CompanyName, LastName, FirstName and MiddleName properties.
//    {L"CompanyLastFirstSpaceOnly"          },                // Returns a String representing the company name for the contact followed by the concatenated last name, first name, and middle name with spaces between the last, first, and middle names. This property is parsed from the CompanyName, LastName, FirstName and MiddleName properties.
//    {L"CompanyMainTelephoneNumber"         },                // Returns or sets the company main telephone number for the contact
//    {L"CompanyName"                        },                // Returns or sets the company name for the contact
//    {L"Complete"                           },                // True if the task is completed
//    {L"ComputerNetworkName"                },                // Returns or sets the name of the computer network for the contact
//    {L"ConferenceServerAllowExternal"      },                // Reserved for future use
//    {L"ConferenceServerPassword"           },                // Reserved for future use.
//    {L"ContactNames"                       },                // Returns a String representing the contact names associated with the journal entry. This property contains the display names for the contacts only. Use the Recipients object to modify the contents of this string.
//    {L"ConversationIndex"                  },                // Returns a String representing the index of the conversation thread of the item. Read-only.
//    {L"ConversationTopic"                  },                // Returns the topic of the conversation thread of the item.
//    {L"CreationTime"                       },                // Returns the creation time for the Outlook item. This property corresponds to the MAPI property PR_CREATION_TIME
//    {L"CustomerID"                         },                // Returns or sets the customer ID for the contact
//    {L"DateCompleted"                      },                // Returns or sets the completion date of the task
//    {L"DeferredDeliveryTime"               },                // Returns or sets the date and time the mail message is to be delivered. This property corresponds to the MAPI property PR_DEFERRED_DELIVERY_TIME
//    {L"DelegationState"                    },                // Returns the delegation state of the task. Can be one of the following OlTaskDelegationState constants: olTaskDelegationAccepted(2), olTaskDelegationDeclined(3), olTaskDelegationUnknown(1), or olTaskNotDelegated(0)
//    {L"Delegator"                          },                // Returns a String representing the display name of the delegator for the task.
//    {L"DeleteAfterSubmit"                  },                // True if a copy of the mail message is not saved upon being sent. False if a copy is saved.
//    {L"Department"                         },                // Returns or sets the department name for the contact
//    {L"DownloadState"                      },                // Returns or sets an OlDownloadState constant indicating the download state of the item. Read-only OlDownloadState.
//    {L"DueDate"                            },                // Returns or sets a Date indicating the due date for the task.
//    {L"Email1Address"                      },                // Returns or sets a String representing the e-mail address of the first e-mail entry for the contact.
//    {L"Email1AddressType"                  },                // Returns or sets a String representing the address type (such as EX or SMTP) of the first e-mail entry for the contact. This is a free-form text field, but it must match the actual type of an existing mail transport.
//    {L"Email1DisplayName"                  },                // Returns a String representing the display name of the first e-mail address for the contact. This property is set to the value of the FullName property by default.
//    {L"Email1EntryID"                      },                // Returns a String representing the entry ID of the first e-mail address for the contact.
//    {L"Email2Address"                      },                // Returns or sets the e-mail address of the second e-mail entry for the contact
//    {L"Email2AddressType"                  },                // Returns or sets a String representing the address type (such as EX or SMTP) of the second e-mail entry for the contact. This is a free-form text field, but it must match the actual type of an existing mail transport.
//    {L"Email2DisplayName"                  },                // Returns a String representing the display name of the second e-mail entry for the contact. This property is set to the value of the FullName property by default.
//    {L"Email2EntryID"                      },                // Returns a String representing the entry ID of the second e-mail entry for the contact.
//    {L"Email3Address"                      },                // Returns or sets the e-mail address of the third e-mail entry for the contact
//    {L"Email3AddressType"                  },                // Returns or sets a String representing the address type (such as EX or SMTP) of the third e-mail entry for the contact. This is a free-form text field, but it must match the actual type of an existing mail transport.
//    {L"Email3DisplayName"                  },                // Returns a String representing the display name of the third e-mail entry for the contact. This property is set to the value of the FullName property by default.
//    {L"Email3EntryID"                      },                // Returns a String representing the entry ID of the third e-mail entry for the contact.
//    {L"EntryID"                            },                // Returns a String representing the unique entry ID of the object. This property corresponds to the MAPI property PR_ENTRYID. MAPI systems assign a permanent, unique ID string when an object is created that does not change from one MAPI session to another. The EntryID property is not set for an Outlook item until it is saved or sent. Also, the EntryID changes when an item is moved into another folder. Read-only.
//    {L"ExpiryTime"                         },                // Returns or sets the date and time at which the item becomes invalid and can be deleted
//    {L"FileAs"                             },                // Returns or sets the default keyword string assigned to the contact when it is filed
//    {L"FirstName"                          },                // Returns or sets the first name for the contact.
//    {L"FlagDueBy"                          },                // Returns or sets the date by which this mail message is due. This property is only valid if the FlagStatus property is also set for the message. This property corresponds to the MAPI property PR_REPLY_TIME
//    {L"FlagRequest"                        },                // Returns or sets the requested action for the mail message. This is a free-form text field. This property is only valid if the FlagStatus property is also set for the message
//    {L"FlagStatus"                         },                // Returns or sets the flag status for the mail message. Can be one of the following OlFlagStatus constants: olFlagComplete(1), olFlagMarked(2), or olNoFlag(0).
//    {L"FormDescription"                    },                // Returns the FormDescription object that represents the form description for the specified Microsoft Outlook item. 
//    {L"FTPSite"                            },                // Returns the FTP site entry for the contact
//    {L"FullName"                           },                // Returns or sets the whole, unparsed full name for the contact
//    {L"FullNameAndCompany"                 },                // Returns a String representing the full name and company of the contact by concatenating the values of the FullName and CompanyName properties.
//    {L"Gender"                             },                // Returns or sets the gender of the contact. Can be one of the following OlGender constants: olFemale(1), olMale(2), or olUnspecified(0). 
//    {L"GetInspector"                       },                // Returns an Inspector object that represents an inspector initialized to contain the specified item. This property is useful for returning a new Inspector object in which to display the item, as opposed to using the ActiveInspector method and setting the CurrentItem property
//    {L"GovernmentIDNumber"                 },                // Returns or sets the government ID number for the contact
//    {L"Hobby"                              },                // Returns or sets the hobby for the contact
//    {L"Home2TelephoneNumber"               },                // Returns or sets the second home telephone number for the contact
//    {L"HomeAddress"                        },                // Returns or sets the full, unparsed text of the home address for the contact
//    {L"HomeAddressCity"                    },                // Returns or sets the city portion of the home address for the contact
//    {L"HomeAddressCountry"                 },                // Returns or sets the country portion of the home address for the contact
//    {L"HomeAddressPostalCode"              },                // Returns or sets the postal code portion of the home address for the contact
//    {L"HomeAddressPostOfficeBox"           },                // Returns or sets the post office box number portion of the home address for the contact
//    {L"HomeAddressState"                   },                // Returns or sets the state portion of the home address for the contact
//    {L"HomeAddressStreet"                  },                // Returns or sets the street portion of the home address for the contact
//    {L"HomeFaxNumber"                      },                // Returns or sets the home fax number for the contact
//    {L"HomeTelephoneNumber"                },                // Returns or sets the first home telephone number for the contact
//    {L"HTMLBody"                           },                // Returns or sets a String representing the HTML body of the specified item. The HTMLBody property should be an HTML syntax string.Setting the HTMLBody property sets the EditorType property of the item's Inspector to olEditorHTML.Setting the HTMLBody property will always update the Body property immediately.Setting the Body property will clear the contents of the HTMLBody property on HTML aware stores.The EditorType property is not affected when you merely access the Body property of the item (as in MsgBox myItem.Body), but when you reset the Body property (as in myItem.Body = "This is a new body"), the EditorType reverts back to the user's default editor.
//    {L"IMAddress"                          },                // Returns or sets a String that represents a contact's Microsoft Instant Messenger address.
//    {L"Importance"                         },                // Returns or sets the relative importance level for the Outlook item. Can be one of the following OlImportance constants: olImportanceHigh(2), olImportanceLow(0), or olImportanceNormal(1). This property corresponds to the MAPI property PR_IMPORTANCE. 
//    {L"Initials"                           },                // Returns or sets the initials for the contact
//    {L"InternetCodepage"                   },                // Returns or sets a Long that determines the Internet code page used by the item. The Internet code page defines the text encoding scheme used by the item. Read/write
//    {L"InternetFreeBusyAddress"            },                // Returns or sets a String corresponding to the Address box on the Details tab for a contact. This box can contain the URL location of the user's free-busy information in vCard Free-Busy standard format.
//    {L"IsConflict"                         },                // Returns a Boolean that determines if the item is in conflict. Whether or not an item is in conflict is determined by the state of the application. For example, when a user is offline and tries to access an online folder the action will fail. In this scenario, the IsConflict property will return True. Read-only.
//    {L"ISDNNumber"                         },                // Returns or sets the ISDN number for the contact
//    {L"IsOnlineMeeting"                    },                // True if this is an online meeting. Read/write Boolean
//    {L"IsRecurring"                        },                // True if the appointment or task is a recurring appointment or task. When the GetRecurrencePattern method is used with an AppointmentItem or TaskItem object, this property is set to True
//    {L"ItemProperties"                     },                // Returns an ItemProperties collection that represents all properties associated with an item.
//    {L"JobTitle"                           },                // Returns or sets the job title for the contact
//    {L"Journal"                            },                // True if the transaction of the contact will be journalized. The default value is False
//    {L"Language"                           },                // Returns or sets the language for the contact
//    {L"LastFirstAndSuffix"                 },                // Returns a String representing the last name, first name, middle name, and suffix of the contact. There is a comma between the last and first names and spaces between all the names and the suffix. This property is parsed from the LastName, FirstName, MiddleName and Suffix properties.
//    {L"LastFirstNoSpace"                   },                // Returns a String representing the concatenated last name, first name, and middle name of the contact with no space between the last name and the first name. This property is parsed from the LastName, FirstName and MiddleName properties.
//    {L"LastFirstNoSpaceAndSuffix"          },                // Returns the last name, first name, and suffix of the user without a space.
//    {L"LastFirstNoSpaceCompany"            },                // Returns a String representing the concatenated last name, first name, and middle name of the contact with no space between the last name and the first name. The company name for the contact is included after the middle name. This property is parsed from the LastName, FirstName, MiddleName, and CompanyName properties.
//    {L"LastFirstSpaceOnly"                 },                // Returns a String representing the concatenated last name, first name, and middle name of the contact with spaces between them. This property is parsed from the LastName, FirstName and MiddleName properties.
//    {L"LastFirstSpaceOnlyCompany"          },                // Returns a String representing the concatenated last name, first name, and middle name of the contact with spaces between them. The company name for the contact is after the middle name. This property is parsed from the LastName, FirstName, MiddleName, and CompanyName properties.
//    {L"Date"                               },                // Returns the time that the Outlook item was last modified. This property corresponds to the MAPI property PR_LAST_MODIFICATION_TIME (Ex LastModificationTime)
//    {L"LastName"                           },                // Returns or sets the last name for the contact
//    {L"LastNameAndFirstName"               },                // Returns a String representing the concatenated last name and first name for the contact.
//    {L"Links"                              },                // Returns a collection of Link objects that represent the contacts to which the item is linked
//    {L"Location"                           },                // Returns or sets the specific office location (for example, Building 1 Room 1 or Suite 123) for the appointment. This property corresponds to the MAPI property PR_OFFICE_LOCATION
//    {L"MailingAddress"                     },                // Returns or sets the full, unparsed selected mailing address for the contact
//    {L"MailingAddressCity"                 },                // Returns or sets a String representing the city name portion of the selected mailing address of the contact.
//    {L"MailingAddressCountry"              },                // Returns or sets a String representing the country/region code portion of the selected mailing address of the contact.
//    {L"MailingAddressPostalCode"           },                // Returns or sets a String representing the postal code (zip code) portion of the selected mailing address of the contact.
//    {L"MailingAddressPostOfficeBox"        },                // Returns or sets a String representing the post office box number portion of the selected mailing address of the contact.
//    {L"MailingAddressState"                },                // Returns or sets a String representing the state code portion for the selected mailing address of the contact
//    {L"MailingAddressStreet"               },                // Returns or sets a String representing the street address portion of the selected mailing address of the contact
//    {L"ManagerName"                        },                // Returns or sets the manager name for the contact
//    {L"MarkForDownload"                    },                // Returns or sets an OlRemoteStatus constant that determines the status of an item once it is received by a remote user. This property gives remote users with less-than-ideal data-transfer capabilities increased messaging flexibility. Read/write.
//    {L"MeetingStatus"                      },                // OlRemoteStatus can be one of these OlRemoteStatus constants.
//    {L"MessageClass"                       },                // Returns or sets a String representing the message class for the Microsoft Outlook item or Action. This property corresponds to the MAPI property PR_MESSAGE_CLASS. The MessageClass property links the item to the form on which it is based. When an item is selected, Outlook uses the message class to locate the form and expose its properties, such as Reply commands.
//    {L"MiddleName"                         },                // Returns or sets a String representing the middle name for the contact.This property is parsed from the FullName property, but may be changed or entered independently should it be parsed incorrectly. Note that any such changes or entries to this property will be overwritten by any subsequent changes of entries to FullName.
//    {L"Mileage"                            },                // Returns or sets a String representing the mileage for an item. This is a free-form string field and can be used to store mileage information associated with the item (for example, 100 miles documented for an appointment, contact, or task) for purposes of reimbursement.
//    {L"MobileTelephoneNumber"              },                // Returns or sets a String representing the mobile telephone number for the contact.
//    {L"NetMeetingAlias"                    },                // Returns or sets a String indicating the user's Microsoft NetMeeting ID, or alias.
//    {L"NetMeetingAutoStart"                },                // True if this online meeting starts automatically. Read/write Boolean
//    {L"NetMeetingDocPathName"              },                // Returns or sets a String representing the full path to the Microsoft Office document specified for a Microsoft NetMeeting online meeting. Read/write
//    {L"NetMeetingOrganizerAlias"           },                // Returns or sets a String representing the alias of the meeting organizer, if this is an online meeting. Read/write.
//    {L"NetMeetingServer"                   },                // Returns or sets a String specifying the name of the Microsoft NetMeeting server being used for an online meeting. Read/write
//    {L"NetMeetingType"                     },                // Sets or returns an OlNetMeetingType constant specifying the type of Microsoft NetMeeting. Read/write.
//    {L"NetShowURL"                         },                // OlNetMeetingType can be one of these OlNetMeetingType constants.
//    {L"NickName"                           },                // Returns or sets a String representing the nickname for the contact.
//    {L"NoAging"                            },                // True to not age the Outlook item. 
//    {L"OfficeLocation"                     },                // Returns or sets a String specifying the specific office location (for example, Building 1 Room 1 or Suite 123) for the contact. This property corresponds to the MAPI property PR_OFFICE_LOCATION.
//    {L"OptionalAttendees"                  },                // Returns or sets a String representing the display string of optional attendees names for the appointment. This property corresponds to the MAPI property PR_DISPLAY_CC. Read/write
//    {L"Ordinal"                            },                // Returns or sets a Long specifying the position in the view (ordinal) for the task.
//    {L"OrganizationalIDNumber"             },                // Returns or sets the organizational ID number for the contact
//    {L"Organizer"                          },                // Returns the name of the organizer of the appointment
//    {L"OriginatorDeliveryReportRequested"  },               // Returns or sets a Boolean value that determines whether the originator of the meeting item or mail message will receive a delivery report. Each transport provider that handles your message sends you a single delivery notification containing the names and addresses of each recipient to whom it was delivered. Note that delivery does not imply that the message has been read. The OriginatorDeliveryReportRequested property corresponds to the MAPI property PR_ORIGINATOR_DELIVERY_REPORT_REQUESTED. True if the originator requested a delivery receipt on the message.
//    {L"OtherAddress"                       },                // Returns or sets the other address for the contact
//    {L"OtherAddressCity"                   },                // Returns or sets the city portion of the other address for the contact
//    {L"OtherAddressCountry"                },                // Returns or sets the country portion of the other address for the contact
//    {L"OtherAddressPostalCode"             },                // Returns or sets the postal code portion of the other address for the contact
//    {L"OtherAddressPostOfficeBox"          },                // Returns or sets the post office box portion of the other address for the contact
//    {L"OtherAddressState"                  },                // Returns or sets the state portion of the other address for the contact
//    {L"OtherAddressStreet"                 },                // Returns or sets the street portion of the other address for the contact
//    {L"OtherFaxNumber"                     },                // Returns or sets the other fax number for the contact
//    {L"OtherTelephoneNumber"               },                // Returns or sets the other telephone number for the contact
//    {L"OutlookInternalVersion"             },                // Returns the build number of the Outlook application for an Outlook item. 
//    {L"OutlookVersion"                     },                // Returns the major and minor version number of the Outlook application for an Outlook item. 
//    {L"Owner"                              },                // Returns or sets the owner for the task. This is a free-form string field. Setting this property to someone other than the current user does not have the effect of delegating the task
//    {L"Ownership"                          },                // Returns an OlTaskOwnership specifying the ownership state of the task.
//    {L"PagerNumber"                        },                // Returns or sets the pager number for the contact
//    {L"Parent"                             },                // Returns the parent object of the specified object
//    {L"PercentComplete"                    },                // Returns or sets the percentage of the task completed at the current date and time
//    {L"PersonalHomePage"                   },                // Returns or sets the URL of the personal Web page for the contact
//    {L"PrimaryTelephoneNumber"             },                // Returns or sets the primary telephone number for the contact
//    {L"Profession"                         },                // Returns or sets the profession for the contact
//    {L"RadioTelephoneNumber"               },                // Returns or sets the radio telephone number for the contact
//    {L"ReadReceiptRequested"               },                // True if a read receipt has been requested by the sender. This property corresponds to the MAPI property PR_READ_RECEIPT_REQUESTED.
//    {L"ReceivedByEntryID"                  },                // Returns a String representing the EntryID for the true recipient as set by the transport provider delivering the mail message. This property corresponds to the MAPI property PR_RECEIVED_BY_ENTRYID.
//    {L"ReceivedByName"                     },                // Returns a String representing the display name of the true recipient for the mail message. This property corresponds to the MAPI property PR_RECEIVED_BY_NAME.
//    {L"ReceivedOnBehalfOfEntryID"          },                // Returns a String representing the EntryID of the user delegated to represent the recipient for the mail message. This property corresponds to the MAPI property PR_RCVD_REPRESENTING_ENTRYID.
//    {L"ReceivedOnBehalfOfName"             },                // Returns a String representing the display name of the user delegated to represent the recipient for the mail message. This property corresponds to the MAPI property PR_RCVD_REPRESENTING_NAME.
//    {L"ReceivedTime"                       },                // Returns the date and time at which the mail message, meeting item, or post was received
//    {L"RecipientReassignmentProhibited"    },                // True if the recipient cannot forward the mail message.
//    {L"Recipients"                         },                // Returns a Recipients collection that represents all the recipients for the Outlook item. Read-only
//    {L"RecurrenceState"                    },                // Returns an OlRecurrenceState constant indicating the recurrence property of the specified object. Read-only
//    {L"ReferredBy"                         },                // Returns or sets the referral name entry for the contact
//    {L"ReminderMinutesBeforeStart"         },                // Returns or sets the number of minutes the reminder should occur prior to the start of the appointment
//    {L"ReminderOverrideDefault"            },                // True if the reminder overrides the default reminder behavior for the appointment, mail item, or task.
//    {L"ReminderPlaySound"                  },                // True if the reminder should play a sound when it occurs for this appointment or task
//    {L"ReminderSet"                        },                // True if a reminder has been set for this appointment, mail item or task. 
//    {L"ReminderSoundFile"                  },                // Returns or sets the path and filename of the sound file to play when the reminder occurs for the appointment, mail message, or task. This property is only valid if the ReminderOverrideDefault and ReminderPlaySound properties are set to True
//    {L"ReminderTime"                       },                // Returns or sets the date and time at which the reminder should occur for this item. 
//    {L"RemoteStatus"                       },                // Returns or sets the remote status of the mail message. Can be one of the following OlRemoteStatus constants: olMarkedForCopy(3), olMarkedForDelete(4), olMarkedForDownload(2), olRemoteStatusNone(0), or olUnMarked(1). 
//    {L"ReplyRecipientNames"                },                // Returns the semicolon-delimited list of reply recipients for the mail message. This property only contains the display names for the reply recipients. The reply recipients list should be set by using the ReplyRecipients collection
//    {L"ReplyRecipients"                    },                // Returns a Recipients collection that represents all the reply recipient objects for the mail message.
//    {L"ReplyTime"                          },                // Returns or sets a Date indicating the reply time for the appointment. Read/write
//    {L"RequiredAttendees"                  },                // Returns the semicolon-delimited string of required attendee names for the meeting appointment. This property only contains the display names for the required attendees. The attendee list should be set by using the Recipients collection. 
//    {L"Resources"                          },                // Returns the semicolon-delimited string of resource names for the meeting. This property contains the display names only. The Recipients collection should be used to modify the resource recipients. Resources are added as BCC recipients to the collection
//    {L"ResponseRequested"                  },                // True if the sender would like a response to the meeting request for the appointment
//    {L"ResponseState"                      },                // Returns or sets an OlTaskResponse constant indicating the overall status of the response to the specified task request.
//    {L"ResponseStatus"                     },                // Returns an OlResponseStatus constant indicating the overall status of the meeting for the current user for the appointment. Read-only
//    {L"Role"                               },                // Returns or sets the free-form text string associating the owner of a task with a role for the task
//    {L"Saved"                              },                // True if the Microsoft Outlook item has not been modified since the last save. Read-only Boolean
//    {L"SaveSentMessageFolder"              },                // Returns a MAPIFolder object that represents the folder in which a copy of the mail message will be saved upon being sent.
//    {L"SchedulePlusPriority"               },                // Returns or sets the Microsoft Schedule+ priority for the task. Can be 1 through 9, A through Z, or A1 through Z9. Priority 1 is the highest
//    {L"SelectedMailingAddress"             },                // Returns or sets an OlMailingAdress constant indicating the type of the mailing address for the contact
//    {L"SenderName"                         },                // Returns a String indicating the display name of the sender for the mail message, meeting item or post. This property corresponds to the MAPI property PR_SENDER_NAME.
//    {L"Sensitivity"                        },                // Returns or sets the sensitivity for the Outlook item. Can be one of the following OlSensitivity constants: olConfidential(3), olNormal(0), olPersonal(1), or olPrivate(2). This property corresponds to the MAPI property PR_SENSITIVITY
//    {L"Sent"                               },                // Returns a Boolean value that indicates if a message has been sent. True if sent, False if not sent. Read-only.In general, there are three different kinds of messages: sent, posted, and saved. Sent messages are traditional e-mail messages or meeting items sent to a recipient or public folder. Posted messages are created in a public folder. Saved messages are created and saved without either sending or posting.
//    {L"SentOn"                             },                // Returns the date and time on which the mail message, meeting item, or post was sent. This property corresponds to the MAPI property PR_CLIENT_SUBMIT_TIME. When you send a meeting request item using the object's Send method, the transport provider sets the ReceivedTime and SentOn properties for you
//    {L"SentOnBehalfOfName"                 },                // Returns the display name for the intended sender of the mail message. This property corresponds to the MAPI property PR_SENT_REPRESENTING_NAME
//    {L"Session"                            },                // Returns the NameSpace object for the current session
//    {L"Size"                               },                // Returns the size (in bytes) of the Outlook item. 
//    {L"Spouse"                             },                // Returns or sets the spouse name entry for the contact
//    {L"StartDate"                          },                // Returns or sets the starting date and time for the task
//    {L"Status"                             },                // Returns or sets the status for the task. Can be one of the following OlTaskStatus constants: olTaskComplete(2), olTaskDeferred(4), olTaskInProgress(1), olTaskNotStarted(0), or olTaskWaiting(3). 
//    {L"StatusOnCompletionRecipients"       },                // Returns or sets a semicolon-delimited String of display names for recipients who will receive status upon completion of the task. This property is calculated from the Recipients property. Recipients returned by the StatusOnCompletionRecipients property correspond to BCC recipients in the Recipients collection.
//    {L"StatusUpdateRecipients"             },                // Returns a semicolon-delimited String of display names for recipients who receive status updates for the task. This property is calculated from the Recipients property. Recipients returned by the StatusUpdateRecipients property correspond to CC recipients in the Recipients collection.
//    {L"Subject"                            },                // Returns or sets the subject for the Outlook item. This property corresponds to the MAPI property PR_SUBJECT. The Subject property is the default property for Outlook items.
//    {L"Submitted"                          },                // Returns a Boolean value that indicates if the item has been submitted. True if the item has been submitted. A message is always created and submitted in a folder, usually the Outbox.
//    {L"Suffix"                             },                // Returns or sets the name suffix (such as Jr., III, or Ph.D.) for the contact
//    {L"TeamTask"                           },                // True if the task is a team task
//    {L"TelexNumber"                        },                // Returns or sets the telex number for the contact
//    {L"Title"                              },                // Returns or sets the title for the contact
//    {L"To"                                 },                // Returns or sets the semicolon-delimited list of display names for the To recipients for the Outlook item. This property contains the display names only. The To property corresponds to the MAPI property PR_DISPLAY_TO. The Recipients collection should be used to modify this property.
//    {L"TotalWork"                          },                // Returns or sets the total work for the task
//    {L"TTYTDDTelephoneNumber"              },                // Returns or sets the TTY/TDD telephone number for the contact
//    {L"UnRead"                             },                // True if the Outlook item has not been opened (read). 
//    {L"User1"                              },                // Returns or sets the first Microsoft Schedule+ user for the contact.
//    {L"User2"                              },                // Returns or sets the second Microsoft Schedule+ user for the contact
//    {L"User3"                              },                // Returns or sets the third Microsoft Schedule+ user for the contact.
//    {L"User4"                              },                // Returns or sets the fourth Microsoft Schedule+ user for the contact.
//    {L"UserCertificate"                    },                // Returns or sets a String containing the user's authentication certificate for the contact
//    {L"UserProperties"                     },                // Returns the UserProperties collection that represents all the user properties for the Outlook item
//    {L"VotingOptions"                      },                // Returns or sets a String specifying a delimited string containing the voting options for the mail message.
//    {L"VotingResponse"                     },                // Returns or sets a String specifying the voting response for the mail message. This property is usually set to one of the delimited values returned by the VotingOptions property on a reply to the original message.
//    {L"WebPage"                            },                // Returns or sets the URL of the Web page for the contact
//    {L"YomiCompanyName"                    },                // Returns or sets a String indicating the Japanese phonetic rendering (yomigana) of the company name for the contact
//    {L"YomiFirstName"                      },                // Returns or sets a String indicating the Japanese phonetic rendering (yomigana) of the first name for the contact
//    {L"YomiLastName"                       },                // Returns or sets a String indicating the Japanese phonetic rendering (yomigana) of the last name for the contact
//    {L"RecurrenceType"                     },                // Returns or set a RecurrenceType. values are orRecursDaily...
//    {L"Interval"                           },                // Is the interval of the recurrence. If RecurrenceType is olRecursDaily, event occurs every <Interval> day. If olRecursWeekly, event occurs every <inteval> week..
//    {L"MonthOfYear"                        },                // Returns or sets the month of year.
//    {L"DayOfMonth"                         },                // Returns or sets the single day of the month from 1 to 31.
//    {L"DayOfWeekMask"                      },                // The combination days of the week constants (i. e. event recurring on Monday and Wednesday. The DayOfWeekMask should be olMonday + olWednesday)
//    {L"Instance"                           },                // Returns or sets the ordinal number of the day, week, month.
//    {L"PatternStartDate"                   },                // Returns or sets the start date of the recurrence.
//    {L"NoEndDate"                          },                // True if there is no end date. True is 1.
//    {L"PatternEndDate"                     },                // Returns or sets the end date of the recurrence.
//    {L"Occurrences"                        },                // Return or sets the number of the occurrences of the recurrence.
//    {L"Color"                              },                // Color of note
//    {L"Height"                             },                // Height of note
//    {L"Width"                              },                // Width of note
//    {L"Left"                               },                // Width of note
//    {L"Top"                                },                // Width of note
//    {L"HomeWebPage"                        },                // Returns or sets the URL of the Home Web page for the contact
//    {NULL}
//};


/** @} */
/** @endcond */
#endif
