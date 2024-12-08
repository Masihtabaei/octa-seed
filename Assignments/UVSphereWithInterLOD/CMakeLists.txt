include("../../CreateApp.cmake")
set(SOURCES "./src/UVSphereWithInterLODApp.cpp")
set(SHADERS "./shaders/UVSphereWithInterLOD.hlsl")
create_app(UVSphereWithInterLOD "${SOURCES}" "${SHADERS}")
