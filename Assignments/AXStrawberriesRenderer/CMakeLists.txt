include("../../CreateApp.cmake")
set(SOURCES "./src/StrawberriesRenderer.cpp")
set(SHADERS "./shaders/Strawberries.hlsl")
create_app(AXStrawberriesRenderer "${SOURCES}" "${SHADERS}")
