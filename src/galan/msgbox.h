/* -*- C++ -*- */
/* gAlan - Graphical Audio Language
 * Copyright (C) 1999 Tony Garnock-Jones
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MsgBox_H
#define MsgBox_H

GALAN_BEGIN_NAMESPACE

typedef enum MsgBoxResponse {
  MSGBOX_NONE = 0,
  MSGBOX_OK = 1,
  MSGBOX_ACCEPT = 2,
  MSGBOX_CANCEL = 4,
  MSGBOX_DISMISS = 8,
  MSGBOX_YES = 16,
  MSGBOX_NO = 32
} MsgBoxResponse;

typedef void (*MsgBoxResponseHandler)(MsgBoxResponse action_taken, gpointer userdata);

/**
 * Pops up a message box displaying the text. If def != MSGBOX_NONE,
 * the corresponding button is made the default selection. If
 * timeout_millis is non-zero, then after that length of time, the
 * default action will be taken. If no default is defined, the timeout
 * has no effect.
 *
 * @param title the title of the message box
 * @param buttons the buttons to display
 * @param timeout_millis timeout in milliseconds
 * @param def the default response to return if we time out
 * @param format a printf-style format string
 * @return the button selected, or 'def' if we time out
 **/
extern MsgBoxResponse popup_msgbox(char *title, MsgBoxResponse buttons,
				   gint timeout_millis, MsgBoxResponse def,
				   char *format, ...);

#if 0
// GTK-specific, here. May change to Qt? (Besides, shouldn't be used in the lib...?) %%%
// (What about popup_msgbox??) %%%
extern MsgBoxResponse popup_dialog(char *title, MsgBoxResponse buttons,
				   gint timeout_millis, MsgBoxResponse def,
				   GtkWidget *contents,
				   MsgBoxResponseHandler handler,
				   gpointer userdata);
#endif

GALAN_END_NAMESPACE

#endif
