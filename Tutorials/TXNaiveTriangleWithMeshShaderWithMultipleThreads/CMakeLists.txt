include("../../CreateApp.cmake")
set(SOURCES "./src/NaiveTriangleWithMeshShaderWithMultipleThreads.cpp")
set(SHADERS "./shaders/NaiveTriangle.hlsl")
create_app(TXNaiveTriangleWithMeshShaderWithMultipleThreads "${SOURCES}" "${SHADERS}")
