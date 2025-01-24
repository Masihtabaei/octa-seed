include("../../CreateApp.cmake")
set(SOURCES "./src/StrawberryRenderer.cpp")
set(SHADERS "./shaders/Strawberry.hlsl")
create_app(AXSimpleStrawberryWithIntraLOD "${SOURCES}" "${SHADERS}")
