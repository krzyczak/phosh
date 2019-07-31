/*
 * Copyright (C) 2019 Purism SPC
 * SPDX-License-Identifier: GPL-3.0+
 * Author: Sebastian Krzyszkowiak <sebastian.krzyszkowiak@puri.sm>
 */

#define G_LOG_DOMAIN "phosh-toplevel"

#include "toplevel.h"
#include "phosh-wayland.h"
#include "shell.h"
#include "util.h"

#include <gdk/gdkwayland.h>

/**
 * SECTION:phosh-toplevel
 * @short_description: Represents a single toplevel surface.
 * @Title: PhoshToplevel
 */

enum {
  PHOSH_TOPLEVEL_PROP_0,
  PHOSH_TOPLEVEL_PROP_HANDLE,
  PHOSH_TOPLEVEL_PROP_CONFIGURED,
  PHOSH_TOPLEVEL_PROP_TITLE,
  PHOSH_TOPLEVEL_PROP_APP_ID,
  PHOSH_TOPLEVEL_PROP_LAST_PROP,
};
static GParamSpec *props[PHOSH_TOPLEVEL_PROP_LAST_PROP];

enum {
  SIGNAL_CLOSED,
  N_SIGNALS
};
static guint signals[N_SIGNALS] = { 0 };

struct _PhoshToplevel {
  GObject parent;
  struct zwlr_foreign_toplevel_handle_v1 *handle;
  gboolean configured;
  gchar *title;
  gchar *app_id;
};

G_DEFINE_TYPE (PhoshToplevel, phosh_toplevel, G_TYPE_OBJECT);


static void
handle_zwlr_foreign_toplevel_handle_title(
  void *data,
  struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
  const char* title)
{
  PhoshToplevel *self = data;
  g_return_if_fail (PHOSH_IS_TOPLEVEL (self));
  g_free (self->title);
  self->title = g_strdup (title);
  g_debug ("%p: Got title %s", zwlr_foreign_toplevel_handle_v1, title);
  g_object_notify_by_pspec (G_OBJECT (self), props[PHOSH_TOPLEVEL_PROP_TITLE]);
}


static void
handle_zwlr_foreign_toplevel_handle_app_id(
  void *data,
  struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
  const char* app_id)
{
  PhoshToplevel *self = data;
  g_return_if_fail (PHOSH_IS_TOPLEVEL (self));
  if (self->app_id)
    g_free (self->app_id);
  self->app_id = g_strdup (app_id);
  g_debug ("%p: Got app_id %s", zwlr_foreign_toplevel_handle_v1, app_id);
  g_object_notify_by_pspec (G_OBJECT (self), props[PHOSH_TOPLEVEL_PROP_APP_ID]);
}


static void
handle_zwlr_foreign_toplevel_handle_output_enter(
  void *data,
  struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
  struct wl_output *output)
{
}


static void
handle_zwlr_foreign_toplevel_handle_output_leave(
  void *data,
  struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
  struct wl_output *output)
{
}


static void
handle_zwlr_foreign_toplevel_handle_state(
  void *data,
  struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
  struct wl_array *state)
{
}


static void
handle_zwlr_foreign_toplevel_handle_done(
  void *data,
  struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1)
{
  PhoshToplevel *self = data;
  g_return_if_fail (PHOSH_IS_TOPLEVEL (self));
  self->configured = TRUE;
  g_object_notify_by_pspec (G_OBJECT (self), props[PHOSH_TOPLEVEL_PROP_CONFIGURED]);
}


static void
handle_zwlr_foreign_toplevel_handle_closed(
  void *data,
  struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1)
{
  PhoshToplevel *self = data;
  g_return_if_fail (PHOSH_IS_TOPLEVEL (self));
  g_signal_emit (self, signals[SIGNAL_CLOSED], 0);
}


static const struct zwlr_foreign_toplevel_handle_v1_listener zwlr_foreign_toplevel_handle_listener = {
  handle_zwlr_foreign_toplevel_handle_title,
  handle_zwlr_foreign_toplevel_handle_app_id,
  handle_zwlr_foreign_toplevel_handle_output_enter,
  handle_zwlr_foreign_toplevel_handle_output_leave,
  handle_zwlr_foreign_toplevel_handle_state,
  handle_zwlr_foreign_toplevel_handle_done,
  handle_zwlr_foreign_toplevel_handle_closed
};


static void
phosh_toplevel_dispose (GObject *object)
{
  PhoshToplevel *self = PHOSH_TOPLEVEL (object);
  zwlr_foreign_toplevel_handle_v1_destroy (self->handle);

  G_OBJECT_CLASS (phosh_toplevel_parent_class)->dispose (object);
}


static void
phosh_toplevel_constructed (GObject *object)
{
  PhoshToplevel *self = PHOSH_TOPLEVEL (object);
  zwlr_foreign_toplevel_handle_v1_add_listener (self->handle, &zwlr_foreign_toplevel_handle_listener, self);
  zwlr_foreign_toplevel_handle_v1_set_user_data (self->handle, self);

  G_OBJECT_CLASS (phosh_toplevel_parent_class)->constructed (object);
}


static void
phosh_toplevel_set_property (GObject *object,
                          guint property_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  PhoshToplevel *self = PHOSH_TOPLEVEL (object);

  switch (property_id) {
  case PHOSH_TOPLEVEL_PROP_HANDLE:
    self->handle = g_value_get_pointer (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
phosh_toplevel_get_property (GObject *object,
                          guint property_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  PhoshToplevel *self = PHOSH_TOPLEVEL (object);

  switch (property_id) {
  case PHOSH_TOPLEVEL_PROP_HANDLE:
    g_value_set_pointer (value, self->handle);
    break;
  case PHOSH_TOPLEVEL_PROP_CONFIGURED:
    g_value_set_boolean (value, self->configured);
    break;
  case PHOSH_TOPLEVEL_PROP_TITLE:
    g_value_set_string (value, self->title);
    break;
  case PHOSH_TOPLEVEL_PROP_APP_ID:
    g_value_set_string (value, self->app_id);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
phosh_toplevel_class_init (PhoshToplevelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = phosh_toplevel_set_property;
  object_class->get_property = phosh_toplevel_get_property;
  object_class->constructed = phosh_toplevel_constructed;
  object_class->dispose = phosh_toplevel_dispose;

  props[PHOSH_TOPLEVEL_PROP_HANDLE] =
    g_param_spec_pointer ("handle",
                          "handle",
                          "The zwlr_foreign_toplevel_handle_v1 object associated with this toplevel",
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS);

  props[PHOSH_TOPLEVEL_PROP_CONFIGURED] =
    g_param_spec_boolean ("configured",
                          "configured",
                          "Whether the toplevel has been already filled with all initial data",
                          FALSE,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

  props[PHOSH_TOPLEVEL_PROP_TITLE] =
    g_param_spec_string ("title",
                         "title",
                         "The window's title",
                         "",
                         G_PARAM_READABLE |
                         G_PARAM_STATIC_STRINGS);

  props[PHOSH_TOPLEVEL_PROP_APP_ID] =
    g_param_spec_string ("app-id",
                         "app-id",
                         "The application ID",
                         "",
                         G_PARAM_READABLE |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PHOSH_TOPLEVEL_PROP_LAST_PROP, props);

  /**
   * PhoshToplevel::closed:
   * @toplevel: The #PhoshToplevel emitting the signal.
   *
   * Emitted when a toplevel has been closed.
   */
  signals[SIGNAL_CLOSED] = g_signal_new (
    "closed",
    G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
    NULL, G_TYPE_NONE, 0);
}


static void
phosh_toplevel_init (PhoshToplevel *self)
{
  self->title = NULL;
  self->app_id = NULL;
  self->handle = NULL;
  self->configured = FALSE;
}


const gchar*
phosh_toplevel_get_title (PhoshToplevel *self) {
  g_return_val_if_fail (PHOSH_IS_TOPLEVEL (self), NULL);
  return self->title;
}


const gchar*
phosh_toplevel_get_app_id (PhoshToplevel *self) {
  g_return_val_if_fail (PHOSH_IS_TOPLEVEL (self), NULL);
  return self->app_id;
}


gboolean
phosh_toplevel_is_configured (PhoshToplevel *self) {
  g_return_val_if_fail (PHOSH_IS_TOPLEVEL (self), FALSE);
  return self->configured;
}


void
phosh_toplevel_raise (PhoshToplevel *self, struct wl_seat *seat) {
  g_return_if_fail (PHOSH_IS_TOPLEVEL (self));
  zwlr_foreign_toplevel_handle_v1_activate (self->handle, seat);
}


void
phosh_toplevel_close (PhoshToplevel *self) {
  g_return_if_fail (PHOSH_IS_TOPLEVEL (self));
  zwlr_foreign_toplevel_handle_v1_close (self->handle);
}


PhoshToplevel *
phosh_toplevel_new_from_handle (struct zwlr_foreign_toplevel_handle_v1 *handle)
{
  return g_object_new (PHOSH_TYPE_TOPLEVEL, "handle", handle, NULL);
}