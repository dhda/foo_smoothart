#include "stdafx.h"
#include "SmoothArtUIE.h"

static service_factory_t<ui_element_impl_withpopup<SmoothArtUIE>> g_smoothart_uie_factory;


DECLARE_COMPONENT_VERSION("Smooth Album Art","0.0","Nice album art UI element");
VALIDATE_COMPONENT_FILENAME("foo_smoothart.dll");
