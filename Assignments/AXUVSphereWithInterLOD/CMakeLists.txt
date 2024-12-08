include("../../CreateApp.cmake")
set(SOURCES "./src/SphereRenderer.cpp")
set(SHADERS "./shaders/Sphere.hlsl")
create_app(AXUVSphereWithInterLOD "${SOURCES}" "${SHADERS}")
