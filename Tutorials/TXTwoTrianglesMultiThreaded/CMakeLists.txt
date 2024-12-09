include("../../CreateApp.cmake")
set(SOURCES "./src/TrianglesRenderer.cpp")
set(SHADERS "./shaders/Triangles.hlsl")
create_app(TXTwoTrianglesMultiThreaded "${SOURCES}" "${SHADERS}")
