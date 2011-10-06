#include "stdafx.h"
#include "SmoothArtUIE.h"

SmoothArtUIE::SmoothArtUIE(ui_element_config::ptr cfg, ui_element_instance_callback_ptr callback)
	: m_callback(callback)
{
	set_configuration(cfg);
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
