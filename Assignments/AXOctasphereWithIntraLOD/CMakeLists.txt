include("../../CreateApp.cmake")
set(SOURCES "./src/OctaSphereRenderer.cpp")
set(SHADERS "./shaders/Octasphere.hlsl")
create_app(AXOctasphereWithIntraLOD "${SOURCES}" "${SHADERS}")
