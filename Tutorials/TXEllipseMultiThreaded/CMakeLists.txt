include("../../CreateApp.cmake")
set(SOURCES "./src/EllipseRenderer.cpp")
set(SHADERS "./shaders/Ellipse.hlsl")
create_app(TXEllipseMultiThreaded "${SOURCES}" "${SHADERS}")
