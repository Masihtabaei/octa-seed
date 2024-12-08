include("../../CreateApp.cmake")
set(SOURCES "./src/BasicFramework.cpp")
set(SHADERS "")
create_app(TXBasicFrameworkForModernGeometryPipeline "${SOURCES}" "${SHADERS}")
