include("../../CreateApp.cmake")
set(SOURCES "./src/OctasphereRenderer.cpp")
set(SHADERS "./shaders/Octasphere.hlsl")
create_app(AXOctasphereWithInterLOD "${SOURCES}" "${SHADERS}")
