include("../../CreateApp.cmake")
set(SOURCES "./src/LemonsRenderer.cpp")
set(SHADERS "./shaders/Lemons.hlsl")
create_app(AXLemonsRenderer "${SOURCES}" "${SHADERS}")
