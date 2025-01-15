include("../../CreateApp.cmake")
set(SOURCES "./src/OctaSphereRenderer.cpp")
set(SHADERS "./shaders/Octasphere.hlsl")
create_app(AXSimpleOctasphereWithBezierCurves "${SOURCES}" "${SHADERS}")
