include("../../CreateApp.cmake")
set(SOURCES "./src/OctahedronRenderer.cpp")
set(SHADERS "./shaders/Octahedron.hlsl")
create_app(TXOctahedronMultiThreadedImproved "${SOURCES}" "${SHADERS}")
