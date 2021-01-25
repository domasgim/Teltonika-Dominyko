#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "smsprocess.h"

static gboolean on_handle_sms (SMSProcessGDBUS *interface, GDBusMethodInvocation *invocation, const gchar *phone_number, const gchar *SMS_message, gpointer user_data) {
    gchar *phone_numberr = g_strdup(phone_number);
    gchar *SMS_messagee = g_strdup(SMS_message);
    _g_utf8_make_valid(SMS_messagee);
    
    smsprocess_gdbus_complete_send_sms_information(interface, invocation, phone_numberr);

    g_print("Phone number: %s\n", phone_numberr);
    g_print("Message: %s\n", SMS_messagee);
    //g_print("Message: %s\n", g_utf8_);

    g_free(phone_numberr);
    g_free(SMS_messagee);
    return TRUE;
}

static void on_sms_received (GDBusConnection *connection, const gchar *phone_number, const gchar *SMS_message, gpointer user_data) {
    SMSProcessGDBUS *interface;
	GError *error;

    interface = smsprocess_gdbus_skeleton_new();
    g_signal_connect(interface, "handle-send-sms-information", G_CALLBACK(on_handle_sms), NULL);
    error = NULL;
    !g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON (interface), connection, "/com/smsprocess/GDBUS", &error);
}

int main() {
    GMainLoop *loop;

	loop = g_main_loop_new (NULL, FALSE);

	g_bus_own_name(G_BUS_TYPE_SESSION, "com.smsprocess", G_BUS_NAME_OWNER_FLAGS_NONE, NULL,
				on_sms_received, NULL, NULL, NULL);

	g_main_loop_run (loop);

	return 0;
}