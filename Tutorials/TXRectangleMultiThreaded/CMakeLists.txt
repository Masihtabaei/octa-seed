include("../../CreateApp.cmake")
set(SOURCES "./src/RectangleRenderer.cpp")
set(SHADERS "./shaders/Rectangle.hlsl")
create_app(TXRectangleMultiThreaded "${SOURCES}" "${SHADERS}")
