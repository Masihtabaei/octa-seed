include("../../CreateApp.cmake")
set(SOURCES "./src/TriangleRenderer.cpp")
set(SHADERS "./shaders/Triangle.hlsl")
create_app(TXTriangleMultiThreaded "${SOURCES}" "${SHADERS}")
