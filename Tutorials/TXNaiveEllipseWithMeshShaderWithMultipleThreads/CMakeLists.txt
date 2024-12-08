include("../../CreateApp.cmake")
set(SOURCES "./src/NaiveEllipseWithMeshShaderWithMultipleThreads.cpp")
set(SHADERS "./shaders/NaiveEllipse.hlsl")
create_app(TXNaiveEllipseWithMeshShaderWithMultipleThreads "${SOURCES}" "${SHADERS}")
