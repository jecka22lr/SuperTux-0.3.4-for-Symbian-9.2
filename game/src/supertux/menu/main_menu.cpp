//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "supertux/menu/main_menu.hpp"
#ifdef __SYMBIAN32__
void symbian_checkpoint(const char* what);
#define SYMBIAN_CP(x) symbian_checkpoint(x)
#else
#define SYMBIAN_CP(x)
#endif


#include "audio/sound_manager.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/addon_menu.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/menu/contrib_menu.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/textscroller.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"

MainMenu::MainMenu() :
  m_addon_menu(),
  m_contrib_menu(),
  m_main_world()
{
  set_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 35);
  add_entry(MNID_STARTGAME, _("Start Game"));
  add_entry(MNID_LEVELS_CONTRIB, _("Contrib Levels"));
#ifndef __SYMBIAN32__  /* the shipped add-ons are always on: nothing useful to toggle */
  add_entry(MNID_ADDONS, _("Add-ons"));
#endif
  add_submenu(_("Options"), MenuStorage::get_options_menu());
  add_entry(MNID_CREDITS, _("Credits"));
  add_entry(MNID_QUITMAINMENU, _("Quit"));
}

void
MainMenu::check_menu()
{
  switch (check())
  {
    case MNID_STARTGAME:
      if (m_main_world.get() == NULL)
      {
        m_main_world.reset(new World());
        SYMBIAN_CP("menu: loading world1 info");
        m_main_world->load("levels/world1/info");
        SYMBIAN_CP("menu: world1 info loaded");
      }
      TitleScreen::start_game(m_main_world.get());
      break;

    case MNID_LEVELS_CONTRIB:
      // Contrib Menu
      m_contrib_menu.reset(new ContribMenu());
      MenuManager::push_current(m_contrib_menu.get());
      break;

    case MNID_ADDONS:
      // Add-ons Menu
      m_addon_menu.reset(new AddonMenu());
      MenuManager::push_current(m_addon_menu.get());
      break;

    case MNID_CREDITS:
      MenuManager::set_current(NULL);
      g_screen_manager->push_screen(new TextScroller("credits.txt"),
                                    new FadeOut(0.5));
      break;

    case MNID_QUITMAINMENU:
      g_screen_manager->quit(new FadeOut(0.25));
      sound_manager->stop_music(0.25);
      break;
  }
}

/* EOF */
