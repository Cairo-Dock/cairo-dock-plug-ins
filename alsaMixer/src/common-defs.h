/*
Copyright 2010 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License version 3, as published
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranties of
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __COMMON_DEFS_H__
#define __COMMON_DEFS_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

typedef enum {
  MUTED,
  ZERO_LEVEL,
  LOW_LEVEL,
  MEDIUM_LEVEL,
  HIGH_LEVEL,
  BLOCKED,
  UNAVAILABLE,
  AVAILABLE
}SoundState;

typedef enum {
  TRANSPORT_ACTION_PREVIOUS,
  TRANSPORT_ACTION_PLAY_PAUSE,
  TRANSPORT_ACTION_NEXT,
  TRANSPORT_ACTION_REWIND,
  TRANSPORT_ACTION_FORWIND,
  TRANSPORT_ACTION_NO_ACTION
}TransportAction;

typedef enum {
  TRANSPORT_STATE_PLAYING,
  TRANSPORT_STATE_PAUSED,
  TRANSPORT_STATE_LAUNCHING
}TransportState;

#define NOT_ACTIVE                              -1
#define DBUSMENU_PROPERTY_EMPTY                 -1

/* DBUS Custom Items */
#define DBUSMENU_VOLUME_MENUITEM_TYPE           "x-canonical-ido-volume-type"
#define DBUSMENU_VOLUME_MENUITEM_LEVEL          "x-canonical-ido-volume-level"
#define DBUSMENU_VOLUME_MENUITEM_MUTE           "x-canonical-ido-volume-mute"

#define DBUSMENU_VOIP_INPUT_MENUITEM_TYPE       "x-canonical-ido-voip-input-type"
#define DBUSMENU_VOIP_INPUT_MENUITEM_LEVEL      "x-canonical-ido-voip-input-level"
#define DBUSMENU_VOIP_INPUT_MENUITEM_MUTE       "x-canonical-ido-voip-input-mute"

#define DBUSMENU_MUTE_MENUITEM_TYPE             "x-canonical-sound-menu-mute-type"
#define DBUSMENU_MUTE_MENUITEM_VALUE            "x-canonical-sound-menu-mute-value"

#define DBUSMENU_TRANSPORT_MENUITEM_TYPE        "x-canonical-sound-menu-player-transport-type"
#define DBUSMENU_TRANSPORT_MENUITEM_PLAY_STATE  "x-canonical-sound-menu-player-transport-state"

#define DBUSMENU_TRACK_SPECIFIC_MENUITEM_TYPE   "x-canonical-sound-menu-player-track-specific-type"

#define DBUSMENU_METADATA_MENUITEM_TYPE                "x-canonical-sound-menu-player-metadata-type"
#define DBUSMENU_METADATA_MENUITEM_ARTIST              "x-canonical-sound-menu-player-metadata-xesam:artist"
#define DBUSMENU_METADATA_MENUITEM_TITLE               "x-canonical-sound-menu-player-metadata-xesam:title"
#define DBUSMENU_METADATA_MENUITEM_ALBUM               "x-canonical-sound-menu-player-metadata-xesam:album"
#define DBUSMENU_METADATA_MENUITEM_ARTURL              "x-canonical-sound-menu-player-metadata-mpris:artUrl"
#define DBUSMENU_METADATA_MENUITEM_PLAYER_NAME         "x-canonical-sound-menu-player-metadata-player-name"
#define DBUSMENU_METADATA_MENUITEM_PLAYER_ICON         "x-canonical-sound-menu-player-metadata-player-icon"
#define DBUSMENU_METADATA_MENUITEM_PLAYER_RUNNING      "x-canonical-sound-menu-player-metadata-player-running"
#define DBUSMENU_METADATA_MENUITEM_HIDE_TRACK_DETAILS  "x-canonical-sound-menu-player-metadata-hide-track-details"

#define DBUSMENU_SCRUB_MENUITEM_TYPE            "x-canonical-sound-menu-player-scrub-type"
#define DBUSMENU_SCRUB_MENUITEM_DURATION        "x-canonical-sound-menu-player-scrub-mpris:length"
#define DBUSMENU_SCRUB_MENUITEM_POSITION        "x-canonical-sound-menu-player-scrub-position"
#define DBUSMENU_SCRUB_MENUITEM_PLAY_STATE      "x-canonical-sound-menu-player-scrub-play-state"

#define DBUSMENU_PLAYLISTS_MENUITEM_TYPE        "x-canonical-sound-menu-player-playlists-type"
#define DBUSMENU_PLAYLISTS_MENUITEM_TITLE       "x-canonical-sound-menu-player-playlists-title"
#define DBUSMENU_PLAYLISTS_MENUITEM_PLAYLISTS   "x-canonical-sound-menu-player-playlists-playlists"

#define DBUSMENU_PLAYLIST_MENUITEM_PATH         "x-canonical-sound-menu-player-playlist-path"

#endif
