include("../../CreateApp.cmake")
set(SOURCES "./src/PearsRenderer.cpp")
set(SHADERS "./shaders/Pears.hlsl")
create_app(AXPearsRenderer "${SOURCES}" "${SHADERS}")
