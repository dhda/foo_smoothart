#include "stdafx.h"
#include "SmoothArtUIE.h"

SmoothArtUIE::SmoothArtUIE(ui_element_config::ptr cfg, ui_element_instance_callback_ptr callback)
	: m_callback(callback)
{
	set_configuration(cfg);

	playlist = static_api_ptr_t<playlist_manager>();
	playback = static_api_ptr_t<playback_control>();
	art_loader = static_api_ptr_t<album_art_manager>()->instantiate();

	if (!RendererGL::isInitialized())
		RendererGL::Initialize();
}

SmoothArtUIE::~SmoothArtUIE() {}


const GUID SmoothArtUIE::s_guid = { 0xc9e9885f, 0x2c8a, 0x497f, { 0x88, 0xf0, 0x74, 0x70, 0xd9, 0x20, 0x59, 0xe4 } };

void SmoothArtUIE::set_configuration(ui_element_config::ptr config)
{
	configuration = config;
}

ui_element_config::ptr SmoothArtUIE::get_configuration()
{
	return configuration;
}

ui_element_config::ptr SmoothArtUIE::g_get_default_configuration()
{
	return ui_element_config::g_create_empty(g_get_guid());
}

void SmoothArtUIE::notify(const GUID & what, t_size param1, const void * param2, t_size param2size)
{
	if (what == ui_element_notify_colors_changed || what == ui_element_notify_font_changed)
	{
		Invalidate();
	}
}


void SmoothArtUIE::GetArt(const char * path)
{
	try
	{
		foobar2000_io::abort_callback_impl abort;
		if (art_loader->open(path, abort))
			renderer->LoadArt(art_loader->query(album_art_ids::cover_front, abort));
	}
	catch (const exception_album_art_not_found & e)
	{
		console::print(e.what());
	}
	catch (const pfc::exception & e)
	{
		console::print(e.what());
	}
}


void SmoothArtUIE::GetSelectedArt(t_size p_playlist)
{
	metadb_handle_ptr meta_handle;

	if (playback->get_now_playing(meta_handle))
		return;

	//t_size playing_list, playing_item;
	//playing_list = playlist->get_active_playlist();

	pfc::list_t<metadb_handle_ptr> selected = pfc::list_t<metadb_handle_ptr>();
	//	if (playlist->get_playing_item_location(&playing_list, &playing_item))

	playlist->playlist_get_selected_items(p_playlist, selected);
	if (selected.get_count() > 0)
	{
		//if (playlist->playlist_get_item_handle(meta_handle, playing_list, playing_item))
		meta_handle = selected.get_item(0);
		GetArt(meta_handle->get_path());
	}
}

void SmoothArtUIE::on_items_selection_change(t_size p_playlist, const bit_array & p_affected, const bit_array & p_state)
{
	GetSelectedArt(p_playlist);
}

void SmoothArtUIE::on_playback_new_track(metadb_handle_ptr p_track)
{
	GetArt(p_track->get_path());
}

void SmoothArtUIE::on_playback_stop(play_control::t_stop_reason p_reason)
{
	GetSelectedArt(playlist->get_active_playlist());
}