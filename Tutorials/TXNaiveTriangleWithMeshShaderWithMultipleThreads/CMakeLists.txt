include("../../CreateApp.cmake")
set(SOURCES "./src/NaiveTriangleWithMeshShader.cpp")
set(SHADERS "./shaders/NaiveTriangle.hlsl")
create_app(TXNaiveTriangleWithMeshShaderWithMultipleThreads "${SOURCES}" "${SHADERS}")
