include("../../CreateApp.cmake")
set(SOURCES "./src/OctaspheresRenderer.cpp")
set(SHADERS "./shaders/Octaspheres.hlsl")
create_app(AXTwoOctaspheres "${SOURCES}" "${SHADERS}")
