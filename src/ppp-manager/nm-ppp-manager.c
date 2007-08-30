/* -*- Mode: C; tab-width: 5; indent-tabs-mode: t; c-basic-offset: 5 -*- */

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "nm-ppp-manager.h"
#include "nm-dbus-manager.h"
#include "nm-utils.h"
#include "dbus-dict-helpers.h"
#include "nm-ppp-marshal.h"

#define NM_PPPD_PLUGIN LIBDIR "/nm-pppd-plugin.so"
#define NM_PPP_WAIT_PPPD 10000 /* 10 seconds */

typedef struct {
	GPid pid;
	NMDBusManager *dbus_manager;

	guint32 signal_handler;
	guint32 ppp_timeout_handler;
	guint32 name_owner_changed_handler;
} NMPPPManagerPrivate;

#define NM_PPP_MANAGER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), NM_TYPE_PPP_MANAGER, NMPPPManagerPrivate))

G_DEFINE_TYPE (NMPPPManager, nm_ppp_manager, G_TYPE_OBJECT)

enum {
	STATE_CHANGED,
	IP4_CONFIG,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

GQuark
nm_ppp_manager_error_quark (void)
{
	static GQuark quark;

	if (!quark)
		quark = g_quark_from_static_string ("nm_ppp_manager_error");

	return quark;
}

static void
nm_ppp_manager_init (NMPPPManager *manager)
{
}

static void
finalize (GObject *object)
{
	nm_ppp_manager_stop (NM_PPP_MANAGER (object));

	G_OBJECT_CLASS (nm_ppp_manager_parent_class)->finalize (object);
}

static void
nm_ppp_manager_class_init (NMPPPManagerClass *manager_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (manager_class);

	g_type_class_add_private (manager_class, sizeof (NMPPPManagerPrivate));

	object_class->finalize = finalize;

	/* signals */
	signals[STATE_CHANGED] =
		g_signal_new ("state-changed",
				    G_OBJECT_CLASS_TYPE (object_class),
				    G_SIGNAL_RUN_FIRST,
				    G_STRUCT_OFFSET (NMPPPManagerClass, state_changed),
				    NULL, NULL,
				    g_cclosure_marshal_VOID__UINT,
				    G_TYPE_NONE, 1,
				    G_TYPE_UINT);

	signals[IP4_CONFIG] =
		g_signal_new ("ip4-config",
				    G_OBJECT_CLASS_TYPE (object_class),
				    G_SIGNAL_RUN_FIRST,
				    G_STRUCT_OFFSET (NMPPPManagerClass, ip4_config),
				    NULL, NULL,
				    nm_ppp_marshal_VOID__STRING_OBJECT,
				    G_TYPE_NONE, 2,
				    G_TYPE_STRING,
				    G_TYPE_OBJECT);
}

NMPPPManager *
nm_ppp_manager_new (void)
{
	return (NMPPPManager *) g_object_new (NM_TYPE_PPP_MANAGER, NULL);
}

/*******************************************/

typedef struct {
	GPtrArray *array;
	GStringChunk *chunk;
} NMCmdLine;

static NMCmdLine *
nm_cmd_line_new (void)
{
	NMCmdLine *cmd;

	cmd = g_slice_new (NMCmdLine);
	cmd->array = g_ptr_array_new ();
	cmd->chunk = g_string_chunk_new (1024);

	return cmd;
}

static void
nm_cmd_line_destroy (NMCmdLine *cmd)
{
	g_ptr_array_free (cmd->array, TRUE);
	g_string_chunk_free (cmd->chunk);
	g_slice_free (NMCmdLine, cmd);
}

static char *
nm_cmd_line_to_str (NMCmdLine *cmd)
{
	char *str;

	g_ptr_array_add (cmd->array, NULL);
	str = g_strjoinv (" ", (gchar **) cmd->array->pdata);
	g_ptr_array_remove_index (cmd->array, cmd->array->len - 1);

	return str;
}

static void
nm_cmd_line_add_string (NMCmdLine *cmd, const char *str)
{
	g_ptr_array_add (cmd->array, g_string_chunk_insert (cmd->chunk, str));
}

static void
nm_cmd_line_add_int (NMCmdLine *cmd, int i)
{
	char *str;

	str = g_strdup_printf ("%d", i);
	nm_cmd_line_add_string (cmd, str);
	g_free (str);
}

/*******************************************/

static inline const char *nm_find_pppd (void)
{
	static const char *pppd_binary_paths[] =
		{
			"/usr/local/sbin/pppd",
			"/usr/sbin/pppd",
			"/sbin/pppd",
			NULL
		};

	const char  **pppd_binary = pppd_binary_paths;

	while (*pppd_binary != NULL) {
		if (g_file_test (*pppd_binary, G_FILE_TEST_EXISTS))
			break;
		pppd_binary++;
	}

	return *pppd_binary;
}

static void
ppp_exit_code (guint pppd_exit_status)
{
	const char *msg;

	switch (pppd_exit_status) {
	case  1: 
		msg = "Fatal pppd error"; 
		break;
	case  2: 
		msg = "pppd options error"; 
		break;
	case  3: 
		msg = "No root priv error"; 
		break;
	case  4: 
		msg = "No ppp module error"; 
		break;
	case  5: 
		msg = "pppd received a signal"; 
		break;
	case  6: 
		msg = "Serial port lock failed"; 
		break;
	case  7: 
		msg = "Serial port open failed"; 
		break;
	case  8: 
		msg = "Connect script failed"; 
		break;
	case  9: 
		msg = "Pty program error"; 
		break;
	case 10: 
		msg = "PPP negotiation failed"; 
		break;
	case 11: 
		msg = "Peer didn't authenticatie itself"; 
		break;
	case 12: 
		msg = "Link idle: Idle Seconds reached."; 
		break;
	case 13: 
		msg = "Connect time limit reached."; 
		break;
	case 14: 
		msg = "Callback negotiated, call should come back.";
		break;
	case 15: 
		msg = "Lack of LCP echo responses"; 
		break;
	case 16: 
		msg = "A modem hung up the phone"; 
		break;
	case 17: 
		msg = "Loopback detected"; 
		break;
	case 18: 
		msg = "The init script failed"; 
		break;
	case 19: 
		msg = "Authentication error.\n"
			"We failed to authenticate ourselves to the peer.\n"
			"Maybe bad account or password?";
		break;
	default:
		msg = "Unknown error";
	}

	g_warning ("pppd exited with error: %s", msg);
}

static void
ppp_watch_cb (GPid pid, gint status, gpointer user_data)
{
	NMPPPManagerPrivate *priv = NM_PPP_MANAGER_GET_PRIVATE (user_data);
	guint err;

	if (WIFEXITED (status)) {
		err = WEXITSTATUS (status);
		if (err != 0)
			ppp_exit_code (err);
	} else if (WIFSTOPPED (status))
		g_warning ("ppp stopped unexpectedly with signal %d", WSTOPSIG (status));
	else if (WIFSIGNALED (status))
		g_warning ("ppp died with signal %d", WTERMSIG (status));
	else
		g_warning ("ppp died from an unknown cause");
  
	/* Reap child if needed. */
	waitpid (pid, NULL, WNOHANG);

	priv->pid = 0;
}

#define HANDLE_DICT_ITEM(in_key, in_type, in_ary_type, op)			  \
	if (!strcmp (entry.key, in_key)) {							  \
		if (entry.type != in_type) {							  \
			nm_warning (in_key "had invalid type in PPP IP Config message."); \
		} else {											  \
			if (in_type == DBUS_TYPE_ARRAY && entry.array_type != in_ary_type) { \
				nm_warning (in_key "had invalid type in PPP IP Config message."); \
			} else {										  \
				op										  \
					}									  \
		}												  \
		goto next;										  \
	}

static gboolean
parse_ip4_config (DBusMessage *message, char **interface, NMIP4Config **config)
{
	DBusMessageIter	iter;
	DBusMessageIter	iter_dict;
	NMUDictEntry entry;
	gboolean success = FALSE;

	dbus_message_iter_init (message, &iter);
	if (!nmu_dbus_dict_open_read (&iter, &iter_dict)) {
		nm_warning ("Warning: couldn't get config dictionary from PPP IP Config message.");
		goto out;
	}

	*config = nm_ip4_config_new ();

	while (nmu_dbus_dict_has_dict_entry (&iter_dict)) {
		int i;

		if (!nmu_dbus_dict_get_entry (&iter_dict, &entry)) {
			nm_warning ("Error: couldn't read dict entryfrom PPP IP Config message.");
			goto out;
		}

		HANDLE_DICT_ITEM("interface", DBUS_TYPE_STRING, 0,
					  { if (strlen (entry.str_value)) *interface = g_strdup (entry.str_value); });
		/* IP specific options */
		HANDLE_DICT_ITEM("address", DBUS_TYPE_UINT32, 0,
					  { nm_ip4_config_set_address (*config, entry.uint32_value); });
		HANDLE_DICT_ITEM("netmask", DBUS_TYPE_UINT32, 0,
					  { nm_ip4_config_set_netmask (*config,  entry.uint32_value ? entry.uint32_value : 0x00FF); });
		HANDLE_DICT_ITEM("gateway", DBUS_TYPE_UINT32, 0,
					  { nm_ip4_config_set_gateway (*config, entry.uint32_value); });
		
		/* Multiple DNS servers are allowed */
		HANDLE_DICT_ITEM("dns_server", DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32,
					  {
						  for (i = 0; i < entry.array_len; i++)
							  nm_ip4_config_add_nameserver (*config, entry.uint32array_value[i]);
					  });

		/* FIXME: Ignoring WINS servers for now since IP4Config doesn't have a place for it */
		
	next:
		nmu_dbus_dict_entry_clear (&entry);
	}

	success = TRUE;

 out:
	if (!success && *config) {
		g_object_unref (*config);
		*config = NULL;
		g_free (*interface);
		*interface = NULL;
	}

	return success;
}

static gboolean
nm_ppp_manager_dbus_signal_handler (DBusConnection *connection,
							 DBusMessage *message,
							 gpointer user_data)
{
	NMPPPManager *manager = NM_PPP_MANAGER (user_data);
	gboolean handled = FALSE;

	if (dbus_message_is_signal (message, NM_DBUS_INTERFACE_PPP, "Status")) {
		guint32 state;

		if (dbus_message_get_args (message, NULL,
							  DBUS_TYPE_UINT32, &state,
							  DBUS_TYPE_INVALID)) {

			g_signal_emit (manager, signals[STATE_CHANGED], 0, state);
			handled = TRUE;
		}
	} else if (dbus_message_is_signal (message, NM_DBUS_INTERFACE_PPP, "IP4Config")) {
		char *iface = NULL;
		NMIP4Config *config = NULL;

		if (parse_ip4_config (message, &iface, &config)) {
			g_signal_emit (manager, signals[IP4_CONFIG], 0, iface, config);
			g_free (iface);
			g_object_unref (config);
			handled = TRUE;
		}
	}

	return handled;
}

static gboolean
pppd_timed_out (gpointer data)
{
	NMPPPManager *manager = NM_PPP_MANAGER (data);

	nm_warning ("Looks like pppd didn't initialize our dbus module");
	nm_ppp_manager_stop (manager);

	return FALSE;
}

static void
name_owner_changed (NMDBusManager *dbus_manager,
				const char *name,
				const char *old,
				const char *new,
				gpointer user_data)
{
	NMPPPManager *manager = NM_PPP_MANAGER (user_data);
	NMPPPManagerPrivate *priv = NM_PPP_MANAGER_GET_PRIVATE (manager);
	gboolean old_owner_good = (old && (strlen (old) > 0));
	gboolean new_owner_good = (new && (strlen (new) > 0));

	if (strcmp (name, NM_DBUS_SERVICE_PPP))
		return;

	if (!old_owner_good && new_owner_good) {
		if (priv->ppp_timeout_handler) {
			g_source_remove (priv->ppp_timeout_handler);
			priv->ppp_timeout_handler = 0;
		}

		priv->signal_handler = nm_dbus_manager_register_signal_handler (priv->dbus_manager,
														    NM_DBUS_INTERFACE_PPP,
														    NM_DBUS_SERVICE_PPP,
														    nm_ppp_manager_dbus_signal_handler,
														    manager);
	} else if (old_owner_good && !new_owner_good) {
		nm_ppp_manager_stop (manager);
	}
}

static void
start_dbus_watcher (NMPPPManager *manager)
{
	NMPPPManagerPrivate *priv = NM_PPP_MANAGER_GET_PRIVATE (manager);

	priv->ppp_timeout_handler = g_timeout_add (NM_PPP_WAIT_PPPD, pppd_timed_out, manager);

	priv->dbus_manager = nm_dbus_manager_get ();
	priv->name_owner_changed_handler = g_signal_connect (priv->dbus_manager, "name-owner-changed",
											   G_CALLBACK (name_owner_changed),
											   manager);
}

static NMCmdLine *
create_pppd_cmd_line (NMSettingPPP *setting, const char *device, GError **err)
{
	const char *ppp_binary;
	NMCmdLine *cmd;

	ppp_binary = nm_find_pppd ();
	if (!ppp_binary) {
		g_set_error (err, NM_PPP_MANAGER_ERROR, NM_PPP_MANAGER_ERROR,
				   "Could not find ppp binary.");
		return NULL;
	}

	/* Create pppd command line */
	cmd = nm_cmd_line_new ();
	nm_cmd_line_add_string (cmd, ppp_binary);

	nm_cmd_line_add_string (cmd, "nodetach");
	nm_cmd_line_add_string (cmd, "lock");
	nm_cmd_line_add_string (cmd, device);

	if (setting->baud)
		nm_cmd_line_add_int (cmd, setting->baud);

	if (setting->noauth)
		nm_cmd_line_add_string (cmd, "noauth");
	if (setting->noauth)
		nm_cmd_line_add_string (cmd, "refuse-eap");
	if (setting->refuse_eap)
		nm_cmd_line_add_string (cmd, "refuse-chap");
	if (setting->refuse_chap)
		nm_cmd_line_add_string (cmd, "refuse-mschap");
	if (setting->refuse_mschap)
		nm_cmd_line_add_string (cmd, "nobsdcomp");
	if (setting->nobsdcomp)
		nm_cmd_line_add_string (cmd, "nodeflate");
	if (setting->nodeflate)
		nm_cmd_line_add_string (cmd, "require-mppe");
	if (setting->require_mppe)
		nm_cmd_line_add_string (cmd, "require-mppe-128");
	if (setting->require_mppe_128)
		nm_cmd_line_add_string (cmd, "mppe-stateful");
	if (setting->mppe_stateful)
		nm_cmd_line_add_string (cmd, "require-mppc");
	if (setting->crtscts)
		nm_cmd_line_add_string (cmd, "crtscts");
	if (setting->usepeerdns)
		nm_cmd_line_add_string (cmd, "usepeerdns");

	if (setting->mru) {
		nm_cmd_line_add_string (cmd, "mru");
		nm_cmd_line_add_int (cmd, setting->mru);
	}

	if (setting->mtu) {
		nm_cmd_line_add_string (cmd, "mtu");
		nm_cmd_line_add_int (cmd, setting->mtu);
	}

	if (setting->lcp_echo_failure) {
		nm_cmd_line_add_string (cmd, "lcp-echo-failure");
		nm_cmd_line_add_int (cmd, setting->lcp_echo_failure);
	}

	if (setting->lcp_echo_interval) {
		nm_cmd_line_add_string (cmd, "lcp-echo-interval");
		nm_cmd_line_add_int (cmd, setting->lcp_echo_interval);
	}

	nm_cmd_line_add_string (cmd, "plugin");
	nm_cmd_line_add_string (cmd, NM_PPPD_PLUGIN);

	return cmd;
}

static void
pppd_child_setup (gpointer user_data G_GNUC_UNUSED)
{
	/* We are in the child process at this point */
	pid_t pid = getpid ();
	setpgid (pid, pid);
}

gboolean
nm_ppp_manager_start (NMPPPManager *manager,
				  const char *device,
				  NMSettingPPP *setting,
				  GError **err)
{
	NMPPPManagerPrivate *priv;
	NMCmdLine *ppp_cmd;
	char *cmd_str;
	GPid pid;
	GSource *ppp_watch;

	g_return_val_if_fail (NM_IS_PPP_MANAGER (manager), FALSE);
	g_return_val_if_fail (device != NULL, FALSE);
	g_return_val_if_fail (setting != NULL, FALSE);

	ppp_cmd = create_pppd_cmd_line (setting, device, err);
	if (!ppp_cmd)
		return FALSE;

	/* FIXME: This should come from NMSettingIP4Config */
	nm_cmd_line_add_string (ppp_cmd, "defaultroute");
	g_ptr_array_add (ppp_cmd->array, NULL);

	priv = NM_PPP_MANAGER_GET_PRIVATE (manager);

	nm_info ("Starting pppd connection");

	cmd_str = nm_cmd_line_to_str (ppp_cmd);
	nm_debug ("Command line: %s", cmd_str);
	g_free (cmd_str);

    priv->pid = 0;
	if (!g_spawn_async (NULL, (char **) ppp_cmd->array->pdata, NULL,
					G_SPAWN_DO_NOT_REAP_CHILD,
					pppd_child_setup,
					NULL, &priv->pid, err)) {
		goto out;
	}

	nm_debug ("ppp started with pid %d", priv->pid);

	ppp_watch = g_child_watch_source_new (pid);
	g_source_set_callback (ppp_watch, (GSourceFunc) ppp_watch_cb, manager, NULL);
	g_source_attach (ppp_watch, NULL);
	g_source_unref (ppp_watch);

	start_dbus_watcher (manager);

 out:
	if (ppp_cmd)
		nm_cmd_line_destroy (ppp_cmd);

	return priv->pid > 0;
}

void
nm_ppp_manager_stop (NMPPPManager *manager)
{
	NMPPPManagerPrivate *priv;

	g_return_if_fail (NM_IS_PPP_MANAGER (manager));

	priv = NM_PPP_MANAGER_GET_PRIVATE (manager);

	if (priv->ppp_timeout_handler) {
		g_source_remove (priv->ppp_timeout_handler);
		priv->ppp_timeout_handler = 0;
	}

	if (priv->signal_handler) {
		nm_dbus_manager_remove_signal_handler (priv->dbus_manager, priv->signal_handler);
		priv->signal_handler = 0;
	}

	if (priv->dbus_manager) {
		g_signal_handler_disconnect (priv->dbus_manager, priv->name_owner_changed_handler);
		g_object_unref (priv->dbus_manager);
		priv->dbus_manager = NULL;
	}

	if (priv->pid) {
		kill (priv->pid, SIGTERM);
		priv->pid = 0;
	}
}
