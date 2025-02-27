include("../../CreateApp.cmake")
set(SOURCES "./src/ApplesRenderer.cpp")
set(SHADERS "./shaders/Apples.hlsl")
create_app(AXApplesRenderer "${SOURCES}" "${SHADERS}")
