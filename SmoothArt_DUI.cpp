#include "stdafx.h"
#include "SmoothArt_DUI.h"

SmoothArt_DUI::SmoothArt_DUI(ui_element_config::ptr cfg, ui_element_instance_callback_ptr callback)
	: m_callback(callback)
{
	set_configuration(cfg);
}

SmoothArt_DUI::~SmoothArt_DUI() {}


const GUID SmoothArt_DUI::s_guid = { 0xc9e9885f, 0x2c8a, 0x497f, { 0x88, 0xf0, 0x74, 0x70, 0xd9, 0x20, 0x59, 0xe4 } };

void SmoothArt_DUI::set_configuration(ui_element_config::ptr config)
{
	configuration = config;
}

ui_element_config::ptr SmoothArt_DUI::get_configuration()
{
	return configuration;
}

ui_element_config::ptr SmoothArt_DUI::g_get_default_configuration()
{
	return ui_element_config::g_create_empty(g_get_guid());
}

void SmoothArt_DUI::notify(const GUID & what, t_size param1, const void * param2, t_size param2size)
{
	if (what == ui_element_notify_colors_changed || what == ui_element_notify_font_changed)
	{
		Invalidate();
	}
}
