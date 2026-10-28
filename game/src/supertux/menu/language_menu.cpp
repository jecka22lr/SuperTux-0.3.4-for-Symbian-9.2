//  SuperTux
//  Copyright (C) 2004 Tobas Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "supertux/menu/language_menu.hpp"

#ifdef __SYMBIAN32__
// Bumped on every language change. The title screen and the pause menu rebuild
// their menus (in the new language) when they see a newer number.
int g_language_generation = 0;
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
// close Language and the old Options menu, then show a fresh Options menu
#define LANGUAGE_DONE() do { g_language_generation++; MenuStorage::reset_cached_menus(); MenuManager::pop_current(); MenuManager::pop_current(); MenuManager::push_current(MenuStorage::get_options_menu()); } while(0)
#else
#define LANGUAGE_DONE() MenuManager::pop_current()
#endif

extern "C" {
#include <findlocale.h>
}
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"

#include <set>
#include <stdio.h>

enum {
  MNID_LANGUAGE_AUTO_DETECT = 0,
  MNID_LANGUAGE_ENGLISH     = 1,
  MNID_LANGUAGE_NEXT        = 10
};

#ifdef __SYMBIAN32__
/* The full list doesn't fit on 320x240: show it in pages, with a "< 1/2 >"
 * selector (left/right flips the page). */
enum { MNID_LANGUAGE_PAGE = 2 };
static const int LANGUAGES_PER_PAGE = 12;
static int language_page = 0;

static void build_language_page(LanguageMenu* menu)
{
  std::set<tinygettext::Language> languages = dictionary_manager->get_languages();
  int total = 1 + (int) languages.size();   // English + translations
  int pages = (total + LANGUAGES_PER_PAGE - 1) / LANGUAGES_PER_PAGE;
  if(language_page >= pages) language_page = 0;
  if(language_page < 0) language_page = pages - 1;

  menu->clear();
  menu->add_label(_("Language"));
  menu->add_hl();
  menu->add_entry(MNID_LANGUAGE_AUTO_DETECT, _("<auto-detect>"));
  int first = language_page * LANGUAGES_PER_PAGE, n = 0;
  if(first == 0)
    menu->add_entry(MNID_LANGUAGE_ENGLISH, "English");
  n = 1;
  int mnid = MNID_LANGUAGE_NEXT;
  for(std::set<tinygettext::Language>::iterator i = languages.begin(); i != languages.end(); i++, mnid++, n++)
    if(n >= first && n < first + LANGUAGES_PER_PAGE)
      menu->add_entry(mnid, i->get_name());

  if(pages > 1) {
    MenuItem* pager = menu->add_string_select(MNID_LANGUAGE_PAGE, "");
    for(int p = 0; p < pages; p++) {
      char buf[16];
      sprintf(buf, "%d/%d", p + 1, pages);
      pager->list.push_back(buf);
    }
    pager->selected = language_page;
  }
  menu->add_hl();
  menu->add_back(_("Back"));
}
#endif

LanguageMenu::LanguageMenu() 
{
#ifdef __SYMBIAN32__
  language_page = 0;
  build_language_page(this);
  return;
#endif
  add_label(_("Language"));
  add_hl();
  add_entry(MNID_LANGUAGE_AUTO_DETECT, _("<auto-detect>"));
  add_entry(MNID_LANGUAGE_ENGLISH, "English");

  int mnid = MNID_LANGUAGE_NEXT;
  std::set<tinygettext::Language> languages = dictionary_manager->get_languages();
  for (std::set<tinygettext::Language>::iterator i = languages.begin(); i != languages.end(); i++) 
  {
    add_entry(mnid++, i->get_name());
  }

  add_hl();
  add_back(_("Back"));
}

void
LanguageMenu::menu_action(MenuItem* item) 
{
#ifdef __SYMBIAN32__
  if (item->id == MNID_LANGUAGE_PAGE) {
    language_page = item->selected;
    build_language_page(this);
    // keep the page selector highlighted after the rebuild
    for(int i = 0; i < (int) items.size(); i++)
      if(items[i]->id == MNID_LANGUAGE_PAGE)
        set_active_item(MNID_LANGUAGE_PAGE);
    return;
  }
#endif
  if (item->id == MNID_LANGUAGE_AUTO_DETECT) // auto detect
  {
    FL_Locale *locale;
    FL_FindLocale(&locale);
    tinygettext::Language language = tinygettext::Language::from_spec( locale->lang?locale->lang:"", locale->country?locale->country:"", locale->variant?locale->variant:"");
    FL_FreeLocale(&locale);

    dictionary_manager->set_language(language); // set currently detected language
    g_config->locale = ""; // do auto detect every time on startup
    g_config->save();
    LANGUAGE_DONE();
  }
  else if (item->id == MNID_LANGUAGE_ENGLISH) // english
  {
    g_config->locale = "en";
    dictionary_manager->set_language(tinygettext::Language::from_name(g_config->locale));
    g_config->save();
    LANGUAGE_DONE();
  }
  else
  {
    int mnid = MNID_LANGUAGE_NEXT;
    std::set<tinygettext::Language> languages = dictionary_manager->get_languages();

    for (std::set<tinygettext::Language>::iterator i = languages.begin(); i != languages.end(); i++) 
    {
      if (item->id == mnid++) 
      {
        g_config->locale = i->str();
        dictionary_manager->set_language(*i);
        g_config->save();
        LANGUAGE_DONE();
        break;
      }
    }
  }
}

/* EOF */
