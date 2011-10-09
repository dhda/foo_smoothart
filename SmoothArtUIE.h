#include "stdafx.h"
#include "ArtWindow.h"


struct SmoothArtUIE : ui_element_instance, ArtWindow
{
public:
	SmoothArtUIE(ui_element_config::ptr cfg, ui_element_instance_callback_ptr callback);
	~SmoothArtUIE();

	static GUID g_get_guid() { return s_guid; }
	static GUID g_get_subclass() { return ui_element_subclass_utility; }
	static void g_get_name(pfc::string_base & out) { out = "Smooth Album Art Viewer"; }
	static const char * g_get_description() { return "Displays album art with a nice background and smooth transitions."; }

	void set_configuration(ui_element_config::ptr config);
	ui_element_config::ptr get_configuration();
	static ui_element_config::ptr g_get_default_configuration();

	void initialize_window(HWND parent) { WIN32_OP( Create(parent, 0, 0, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_STATICEDGE) != 0 ); }

	void notify(const GUID & what, t_size param1, const void * param2, t_size param2size);

protected:
	static const GUID s_guid;
	const ui_element_instance_callback_ptr m_callback;

private:
	ui_element_config::ptr configuration;
};