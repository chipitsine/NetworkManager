// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2007 - 2008 Novell, Inc.
 * Copyright (C) 2007 - 2014 Red Hat, Inc.
 */

#ifndef __NM_MANAGER_H__
#define __NM_MANAGER_H__

#if !((NETWORKMANAGER_COMPILATION) & NM_NETWORKMANAGER_COMPILATION_WITH_LIBNM_PRIVATE)
#error Cannot use this header.
#endif

#include "nm-object.h"
#include "nm-client.h"
#include "nm-libnm-utils.h"

#define NM_TYPE_MANAGER            (nm_manager_get_type ())
#define NM_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NM_TYPE_MANAGER, NMManager))
#define NM_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NM_TYPE_MANAGER, NMManagerClass))
#define NM_IS_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NM_TYPE_MANAGER))
#define NM_IS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NM_TYPE_MANAGER))
#define NM_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NM_TYPE_MANAGER, NMManagerClass))

#define NM_MANAGER_VERSION "version"
#define NM_MANAGER_STATE "state"
#define NM_MANAGER_STARTUP "startup"
#define NM_MANAGER_NETWORKING_ENABLED "networking-enabled"

_NM_DEPRECATED_SYNC_WRITABLE_PROPERTY_INTERNAL
#define NM_MANAGER_WIRELESS_ENABLED "wireless-enabled"

_NM_DEPRECATED_SYNC_WRITABLE_PROPERTY_INTERNAL
#define NM_MANAGER_WWAN_ENABLED "wwan-enabled"

_NM_DEPRECATED_SYNC_WRITABLE_PROPERTY_INTERNAL
#define NM_MANAGER_WIMAX_ENABLED "wimax-enabled"

#define NM_MANAGER_WIRELESS_HARDWARE_ENABLED "wireless-hardware-enabled"
#define NM_MANAGER_WWAN_HARDWARE_ENABLED "wwan-hardware-enabled"
#define NM_MANAGER_WIMAX_HARDWARE_ENABLED "wimax-hardware-enabled"
#define NM_MANAGER_ACTIVE_CONNECTIONS "active-connections"
#define NM_MANAGER_CONNECTIVITY "connectivity"
#define NM_MANAGER_CONNECTIVITY_CHECK_AVAILABLE "connectivity-check-available"

_NM_DEPRECATED_SYNC_WRITABLE_PROPERTY_INTERNAL
#define NM_MANAGER_CONNECTIVITY_CHECK_ENABLED "connectivity-check-enabled"

#define NM_MANAGER_PRIMARY_CONNECTION "primary-connection"
#define NM_MANAGER_ACTIVATING_CONNECTION "activating-connection"
#define NM_MANAGER_DEVICES "devices"
#define NM_MANAGER_CHECKPOINTS "checkpoints"
#define NM_MANAGER_METERED "metered"
#define NM_MANAGER_ALL_DEVICES "all-devices"

/**
 * NMManager:
 */
typedef struct {
	NMObject parent;
} NMManager;

typedef struct {
	NMObjectClass parent;

	/* Signals */
	void (*device_added) (NMManager *manager, NMDevice *device);
	void (*device_removed) (NMManager *manager, NMDevice *device);
	void (*active_connection_added) (NMManager *manager, NMActiveConnection *ac);
	void (*active_connection_removed) (NMManager *manager, NMActiveConnection *ac);
	void (*checkpoint_added) (NMManager *manager, NMCheckpoint *checkpoint);
	void (*checkpoint_removed) (NMManager *manager, NMCheckpoint *checkpoint);
	void (*permission_changed) (NMManager *manager,
	                            NMClientPermission permission,
	                            NMClientPermissionResult result);
} NMManagerClass;

GType nm_manager_get_type (void);

const char *nm_manager_get_version        (NMManager *manager);
NMState   nm_manager_get_state            (NMManager *manager);
gboolean  nm_manager_get_startup          (NMManager *manager);

gboolean  nm_manager_networking_get_enabled (NMManager *manager);

_NM_DEPRECATED_SYNC_METHOD_INTERNAL
gboolean _nm_manager_networking_set_enabled (GDBusConnection *dbus_connection,
                                             const char *name_owner,
                                             gboolean enable,
                                             GError **error);

gboolean  nm_manager_wireless_get_enabled (NMManager *manager);

_NM_DEPRECATED_SYNC_METHOD_INTERNAL
void      nm_manager_wireless_set_enabled (NMManager *manager, gboolean enabled);

gboolean  nm_manager_wireless_hardware_get_enabled (NMManager *manager);

gboolean  nm_manager_wwan_get_enabled (NMManager *manager);
void      nm_manager_wwan_set_enabled (NMManager *manager, gboolean enabled);
gboolean  nm_manager_wwan_hardware_get_enabled (NMManager *manager);

gboolean  nm_manager_wimax_get_enabled (NMManager *manager);
void      nm_manager_wimax_set_enabled (NMManager *manager, gboolean enabled);
gboolean  nm_manager_wimax_hardware_get_enabled (NMManager *manager);

gboolean  nm_manager_connectivity_check_get_available (NMManager *manager);

gboolean  nm_manager_connectivity_check_get_enabled (NMManager *manager);

void      nm_manager_connectivity_check_set_enabled (NMManager *manager,
                                                     gboolean enabled);

const char *nm_manager_connectivity_check_get_uri (NMManager *manager);

NMClientPermissionResult nm_manager_get_permission_result (NMManager *manager,
                                                           NMClientPermission permission);

NMConnectivityState nm_manager_get_connectivity          (NMManager *manager);

void _nm_manager_set_connectivity_hack (NMManager *manager,
                                        guint32 connectivity);

/* Devices */

const GPtrArray *nm_manager_get_devices    (NMManager *manager);
const GPtrArray *nm_manager_get_all_devices(NMManager *manager);
NMDevice *nm_manager_get_device_by_path    (NMManager *manager, const char *object_path);
NMDevice *nm_manager_get_device_by_iface   (NMManager *manager, const char *iface);

/* Active Connections */

const GPtrArray *nm_manager_get_active_connections (NMManager *manager);

NMActiveConnection *nm_manager_get_primary_connection (NMManager *manager);
NMActiveConnection *nm_manager_get_activating_connection (NMManager *manager);

void nm_manager_wait_for_active_connection (NMManager *self,
                                            const char *active_path,
                                            const char *connection_path,
                                            GVariant *add_and_activate_output_take,
                                            GTask *task_take);

const GPtrArray *nm_manager_get_checkpoints (NMManager *manager);

void nm_manager_wait_for_checkpoint (NMManager *self,
                                     const char *checkpoint_path,
                                     GTask *task_take);

/*****************************************************************************/

typedef struct {
	NMActiveConnection *active;
	GVariant *add_and_activate_output;
} _NMActivateResult;

_NMActivateResult *_nm_activate_result_new (NMActiveConnection *active,
                                            GVariant *add_and_activate_output);

void _nm_activate_result_free (_NMActivateResult *result);

NM_AUTO_DEFINE_FCN0 (_NMActivateResult *, _nm_auto_free_activate_result, _nm_activate_result_free)
#define nm_auto_free_activate_result nm_auto(_nm_auto_free_activate_result)

/*****************************************************************************/

#endif /* __NM_MANAGER_H__ */
