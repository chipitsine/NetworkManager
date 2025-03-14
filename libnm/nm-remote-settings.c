// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2008 Novell, Inc.
 * Copyright (C) 2009 - 2012 Red Hat, Inc.
 */

#include "nm-default.h"

#include "nm-remote-settings.h"

#include "c-list/src/c-list.h"
#include "nm-dbus-interface.h"
#include "nm-connection.h"
#include "nm-client.h"
#include "nm-remote-connection.h"
#include "nm-remote-connection-private.h"
#include "nm-object-private.h"
#include "nm-dbus-helpers.h"
#include "nm-core-internal.h"

#include "introspection/org.freedesktop.NetworkManager.Settings.h"

G_DEFINE_TYPE (NMRemoteSettings, nm_remote_settings, NM_TYPE_OBJECT)

#define NM_REMOTE_SETTINGS_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), NM_TYPE_REMOTE_SETTINGS, NMRemoteSettingsPrivate))

typedef struct {
	NMDBusSettings *proxy;
	GPtrArray *all_connections;
	GPtrArray *visible_connections;

	/* AddConnectionInfo objects that are waiting for the connection to become initialized */
	CList add_lst_head;

	char *hostname;
	gboolean can_modify;
} NMRemoteSettingsPrivate;

enum {
	PROP_0,
	PROP_CONNECTIONS,
	PROP_HOSTNAME,
	PROP_CAN_MODIFY,

	LAST_PROP
};

/* Signals */
enum {
	CONNECTION_ADDED,
	CONNECTION_REMOVED,

	LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

/*****************************************************************************/

typedef struct {
	CList add_lst;
	NMRemoteSettings *self;
	GTask *task;
	char *connection_path;
	GVariant *extra_results;
	gulong cancellable_id;
} AddConnectionInfo;

static AddConnectionInfo *
_add_connection_info_find (NMRemoteSettings *self, const char *connection_path)
{
	NMRemoteSettingsPrivate *priv = NM_REMOTE_SETTINGS_GET_PRIVATE (self);
	AddConnectionInfo *info;

	c_list_for_each_entry (info, &priv->add_lst_head, add_lst) {
		if (nm_streq (info->connection_path, connection_path))
			return info;
	}
	return NULL;
}

static void
_add_connection_info_complete (AddConnectionInfo *info,
                               NMRemoteConnection *connection,
                               GError *error_take)
{
	nm_assert (info);

	c_list_unlink_stale (&info->add_lst);

	nm_clear_g_signal_handler (g_task_get_cancellable (info->task), &info->cancellable_id);

	if (error_take)
		g_task_return_error (info->task, error_take);
	else {
		NMAddConnectionResultData *result_info;

		result_info = g_slice_new (NMAddConnectionResultData);
		*result_info = (NMAddConnectionResultData) {
			.connection    = g_object_ref (connection),
			.extra_results = g_steal_pointer (&info->extra_results),
		};
		g_task_return_pointer (info->task, result_info, (GDestroyNotify) nm_add_connection_result_data_free);
	}

	g_object_unref (info->task);
	g_object_unref (info->self);
	g_free (info->connection_path);
	nm_g_variant_unref (info->extra_results);
	nm_g_slice_free (info);
}

static void
_wait_for_connection_cancelled_cb (GCancellable *cancellable,
                                   AddConnectionInfo *info)
{
	_add_connection_info_complete (info,
	                               NULL,
	                               g_error_new_literal (G_IO_ERROR,
	                                                    G_IO_ERROR_CANCELLED,
	                                                    "Operation was cancelled"));
}

typedef const char * (*ConnectionStringGetter) (NMConnection *);

static NMRemoteConnection *
get_connection_by_string (NMRemoteSettings *settings,
                          const char *string,
                          ConnectionStringGetter get_comparison_string)
{
	NMRemoteSettingsPrivate *priv;
	NMConnection *candidate;
	int i;

	priv = NM_REMOTE_SETTINGS_GET_PRIVATE (settings);

	for (i = 0; i < priv->visible_connections->len; i++) {
		candidate = priv->visible_connections->pdata[i];
		if (!g_strcmp0 (string, get_comparison_string (candidate)))
			return NM_REMOTE_CONNECTION (candidate);
	}

	return NULL;
}

NMRemoteConnection *
nm_remote_settings_get_connection_by_id (NMRemoteSettings *settings, const char *id)
{
	g_return_val_if_fail (NM_IS_REMOTE_SETTINGS (settings), NULL);
	g_return_val_if_fail (id != NULL, NULL);

	return get_connection_by_string (settings, id, nm_connection_get_id);
}

NMRemoteConnection *
nm_remote_settings_get_connection_by_path (NMRemoteSettings *settings, const char *path)
{
	g_return_val_if_fail (NM_IS_REMOTE_SETTINGS (settings), NULL);
	g_return_val_if_fail (path != NULL, NULL);

	return get_connection_by_string (settings, path, nm_connection_get_path);
}

NMRemoteConnection *
nm_remote_settings_get_connection_by_uuid (NMRemoteSettings *settings, const char *uuid)
{
	g_return_val_if_fail (NM_IS_REMOTE_SETTINGS (settings), NULL);
	g_return_val_if_fail (uuid != NULL, NULL);

	return get_connection_by_string (settings, uuid, nm_connection_get_uuid);
}

static void
connection_visible_changed (GObject *object,
                            GParamSpec *pspec,
                            gpointer user_data)
{
	NMRemoteConnection *connection = NM_REMOTE_CONNECTION (object);
	NMRemoteSettings *self = NM_REMOTE_SETTINGS (user_data);

	if (nm_remote_connection_get_visible (connection))
		g_signal_emit (self, signals[CONNECTION_ADDED], 0, connection);
	else
		g_signal_emit (self, signals[CONNECTION_REMOVED], 0, connection);
}

static void
cleanup_connection (NMRemoteSettings *self,
                    NMRemoteConnection *remote)
{
	g_signal_handlers_disconnect_by_func (remote, G_CALLBACK (connection_visible_changed), self);
}

static void
connection_removed (NMRemoteSettings *self,
                    NMRemoteConnection *remote)
{
	NMRemoteSettingsPrivate *priv = NM_REMOTE_SETTINGS_GET_PRIVATE (self);
	gboolean still_exists = FALSE;
	int i;

	/* Check if the connection was actually removed or if it just turned invisible. */
	for (i = 0; i < priv->all_connections->len; i++) {
		if (remote == priv->all_connections->pdata[i]) {
			still_exists = TRUE;
			break;
		}
	}

	if (!still_exists)
		cleanup_connection (self, remote);

	/* Allow the signal to propagate if and only if @remote was in visible_connections */
	if (!g_ptr_array_remove (priv->visible_connections, remote))
		g_signal_stop_emission (self, signals[CONNECTION_REMOVED], 0);
}

static void
connection_added (NMRemoteSettings *self,
                  NMRemoteConnection *remote)
{
	NMRemoteSettingsPrivate *priv = NM_REMOTE_SETTINGS_GET_PRIVATE (self);
	AddConnectionInfo *info;
	const char *path;

	if (!g_signal_handler_find (remote, G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA, 0, 0, NULL,
	                            G_CALLBACK (connection_visible_changed), self)) {
		g_signal_connect (remote,
		                  "notify::" NM_REMOTE_CONNECTION_VISIBLE,
		                  G_CALLBACK (connection_visible_changed),
		                  self);
	}

	if (nm_remote_connection_get_visible (remote))
		g_ptr_array_add (priv->visible_connections, remote);
	else
		g_signal_stop_emission (self, signals[CONNECTION_ADDED], 0);

	/* FIXME: this doesn't look right. Why does it not care about whether the
	 * connection is visible? Anyway, this will be reworked. */
	path = nm_connection_get_path (NM_CONNECTION (remote));
	info =   path
	       ? _add_connection_info_find (self, path)
	       : NULL;
	if (info)
		_add_connection_info_complete (info, remote, NULL);
}

static void
object_creation_failed (NMObject *object,
                        const char *failed_path)
{
	NMRemoteSettings *self = NM_REMOTE_SETTINGS (object);
	AddConnectionInfo *info;

	info = _add_connection_info_find (self, failed_path);
	if (!info)
		return;

	_add_connection_info_complete (info,
	                               NULL,
	                               g_error_new_literal (NM_CLIENT_ERROR,
	                                                    NM_CLIENT_ERROR_OBJECT_CREATION_FAILED,
	                                                    _("Connection removed before it was initialized")));
}

const GPtrArray *
nm_remote_settings_get_connections (NMRemoteSettings *settings)
{
	g_return_val_if_fail (NM_IS_REMOTE_SETTINGS (settings), NULL);

	return NM_REMOTE_SETTINGS_GET_PRIVATE (settings)->visible_connections;
}

void
nm_remote_settings_wait_for_connection (NMRemoteSettings *self,
                                        const char *connection_path,
                                        GVariant *extra_results_take,
                                        GTask *task_take)
{
	NMRemoteSettingsPrivate *priv;
	gs_unref_object GTask *task = task_take;
	gs_unref_variant GVariant *extra_results = extra_results_take;
	GCancellable *cancellable;
	AddConnectionInfo *info;

	priv = NM_REMOTE_SETTINGS_GET_PRIVATE (self);

	/* FIXME: there is no timeout for how long we wait. But this entire
	 * code will be reworked, also that we have a suitable GMainContext
	 * where we can schedule the timeout (we shouldn't use g_main_context_default()). */

	info = g_slice_new (AddConnectionInfo);
	*info = (AddConnectionInfo) {
		.self            = g_object_ref (self),
		.connection_path = g_strdup (connection_path),
		.task            = g_steal_pointer (&task),
		.extra_results   = g_steal_pointer (&extra_results),
	};
	c_list_link_tail (&priv->add_lst_head, &info->add_lst);

	cancellable = g_task_get_cancellable (info->task);
	/* On success, we still have to wait until the connection is fully
	 * initialized before calling the callback.
	 */
	if (cancellable) {
		gulong id;

		id = g_cancellable_connect (cancellable,
		                            G_CALLBACK (_wait_for_connection_cancelled_cb),
		                            info,
		                            NULL);
		if (id == 0) {
			/* the callback was invoked synchronously, which destroyed @info.
			 * We must not touch @info anymore. */
		} else
			info->cancellable_id = id;
	}

	/* FIXME: OK, we just assume the the connection is here, and that we are bound
	 * to get the suitable signal when the connection is fully initalized (or failed).
	 * Obviously, that needs reworking. */
}

/*****************************************************************************/

static void
nm_remote_settings_init (NMRemoteSettings *self)
{
	NMRemoteSettingsPrivate *priv = NM_REMOTE_SETTINGS_GET_PRIVATE (self);

	c_list_init (&priv->add_lst_head);
	priv->all_connections = g_ptr_array_new ();
	priv->visible_connections = g_ptr_array_new ();
}

static void
init_dbus (NMObject *object)
{
	NMRemoteSettingsPrivate *priv = NM_REMOTE_SETTINGS_GET_PRIVATE (object);
	const NMPropertiesInfo property_info[] = {
		{ NM_REMOTE_SETTINGS_CONNECTIONS,      &priv->all_connections, NULL, NM_TYPE_REMOTE_CONNECTION, "connection" },
		{ NM_REMOTE_SETTINGS_HOSTNAME,         &priv->hostname },
		{ NM_REMOTE_SETTINGS_CAN_MODIFY,       &priv->can_modify },
		{ NULL },
	};

	NM_OBJECT_CLASS (nm_remote_settings_parent_class)->init_dbus (object);

	priv->proxy = NMDBUS_SETTINGS (_nm_object_get_proxy (object, NM_DBUS_INTERFACE_SETTINGS));
	_nm_object_register_properties (object,
	                                NM_DBUS_INTERFACE_SETTINGS,
	                                property_info);
}

static GObject *
constructor (GType type,
             guint n_construct_params,
             GObjectConstructParam *construct_params)
{
	guint i;
	const char *dbus_path;

	/* Fill in the right D-Bus path if none was specified */
	for (i = 0; i < n_construct_params; i++) {
		if (strcmp (construct_params[i].pspec->name, NM_OBJECT_PATH) == 0) {
			dbus_path = g_value_get_string (construct_params[i].value);
			if (dbus_path == NULL) {
				g_value_set_static_string (construct_params[i].value, NM_DBUS_PATH_SETTINGS);
			} else {
				if (!g_variant_is_object_path (dbus_path)) {
					g_warning ("Passed D-Bus object path '%s' is invalid; using default '%s' instead",
					           dbus_path, NM_DBUS_PATH);
					g_value_set_static_string (construct_params[i].value, NM_DBUS_PATH_SETTINGS);
				}
			}
			break;
		}
	}

	return G_OBJECT_CLASS (nm_remote_settings_parent_class)->constructor (type,
	                                                                      n_construct_params,
	                                                                      construct_params);
}

static void
dispose (GObject *object)
{
	NMRemoteSettings *self = NM_REMOTE_SETTINGS (object);
	NMRemoteSettingsPrivate *priv = NM_REMOTE_SETTINGS_GET_PRIVATE (self);
	guint i;

	if (priv->all_connections) {
		for (i = 0; i < priv->all_connections->len; i++)
			cleanup_connection (self, priv->all_connections->pdata[i]);
		g_clear_pointer (&priv->all_connections, g_ptr_array_unref);
	}

	g_clear_pointer (&priv->visible_connections, g_ptr_array_unref);
	g_clear_pointer (&priv->hostname, g_free);
	g_clear_object (&priv->proxy);

	G_OBJECT_CLASS (nm_remote_settings_parent_class)->dispose (object);
}

static void
get_property (GObject *object, guint prop_id,
              GValue *value, GParamSpec *pspec)
{
	NMRemoteSettingsPrivate *priv = NM_REMOTE_SETTINGS_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_CONNECTIONS:
		g_value_take_boxed (value, _nm_utils_copy_object_array (priv->visible_connections));
		break;
	case PROP_HOSTNAME:
		g_value_set_string (value, priv->hostname);
		break;
	case PROP_CAN_MODIFY:
		g_value_set_boolean (value, priv->can_modify);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
nm_remote_settings_class_init (NMRemoteSettingsClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	NMObjectClass *nm_object_class = NM_OBJECT_CLASS (class);

	g_type_class_add_private (class, sizeof (NMRemoteSettingsPrivate));

	/* Virtual methods */
	object_class->constructor = constructor;
	object_class->get_property = get_property;
	object_class->dispose = dispose;

	nm_object_class->init_dbus = init_dbus;
	nm_object_class->object_creation_failed = object_creation_failed;

	class->connection_added = connection_added;
	class->connection_removed = connection_removed;

	/* Properties */

	g_object_class_install_property
		(object_class, PROP_CONNECTIONS,
		 g_param_spec_boxed (NM_REMOTE_SETTINGS_CONNECTIONS, "", "",
		                     G_TYPE_PTR_ARRAY,
		                     G_PARAM_READABLE |
		                     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property
		(object_class, PROP_HOSTNAME,
		 g_param_spec_string (NM_REMOTE_SETTINGS_HOSTNAME, "", "",
		                      NULL,
		                      G_PARAM_READABLE |
		                      G_PARAM_STATIC_STRINGS));

	g_object_class_install_property
		(object_class, PROP_CAN_MODIFY,
		 g_param_spec_boolean (NM_REMOTE_SETTINGS_CAN_MODIFY, "", "",
		                       FALSE,
		                       G_PARAM_READABLE |
		                       G_PARAM_STATIC_STRINGS));

	/* Signals */
	signals[CONNECTION_ADDED] =
		g_signal_new (NM_REMOTE_SETTINGS_CONNECTION_ADDED,
		              G_OBJECT_CLASS_TYPE (object_class),
		              G_SIGNAL_RUN_FIRST,
		              G_STRUCT_OFFSET (NMRemoteSettingsClass, connection_added),
		              NULL, NULL, NULL,
		              G_TYPE_NONE, 1,
		              NM_TYPE_REMOTE_CONNECTION);

	signals[CONNECTION_REMOVED] =
		g_signal_new (NM_REMOTE_SETTINGS_CONNECTION_REMOVED,
		              G_OBJECT_CLASS_TYPE (object_class),
		              G_SIGNAL_RUN_FIRST,
		              G_STRUCT_OFFSET (NMRemoteSettingsClass, connection_removed),
		              NULL, NULL, NULL,
		              G_TYPE_NONE, 1,
		              NM_TYPE_REMOTE_CONNECTION);
}
