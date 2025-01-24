include("../../CreateApp.cmake")
set(SOURCES "./src/AppleRenderer.cpp")
set(SHADERS "./shaders/Apple.hlsl")
create_app(AXSimpleAppleWithIntraLOD "${SOURCES}" "${SHADERS}")
