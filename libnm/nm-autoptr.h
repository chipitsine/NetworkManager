// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright 2018 Red Hat, Inc.
 */

#ifndef __NM_AUTOPTR_H__
#define __NM_AUTOPTR_H__

/*
 * Note that you might use this header with older versions of libnm
 * that do not yet ship this header. In that case, copy the header
 * into your source tree.
 */

#include <glib.h>
#include <NetworkManager.h>

#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC

G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMClient, g_object_unref)

G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMConnection, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSetting, g_object_unref)

G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSetting6Lowpan, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSetting8021x, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingAdsl, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingBluetooth, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingBond, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingBridge, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingBridgePort, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingCdma, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingConnection, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingDcb, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingDummy, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingEthtool, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingGeneric, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingGsm, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingIP4Config, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingIP6Config, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingIPConfig, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingIPTunnel, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingInfiniband, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingMacsec, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingMacvlan, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingMatch, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingOlpcMesh, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingOvsBridge, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingOvsInterface, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingOvsPatch, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingOvsPort, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingPpp, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingPppoe, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingProxy, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingSerial, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingSriov, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingTCConfig, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingTeam, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingTeamPort, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingTun, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingUser, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingVlan, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingVpn, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingVxlan, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingWifiP2P, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingWimax, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingWired, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingWireGuard, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingWireless, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingWirelessSecurity, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMSettingWpan, g_object_unref)

G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMVpnEditor, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMVpnEditorPlugin, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMVpnPluginInfo, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (NMVpnServicePlugin, g_object_unref)

#endif

#endif /* __NM_AUTOPTR_H__ */
