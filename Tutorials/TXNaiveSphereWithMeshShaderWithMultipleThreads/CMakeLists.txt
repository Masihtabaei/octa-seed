include("../../CreateApp.cmake")
set(SOURCES "./src/NaiveSphereWithMeshShaderWithMultipleThreads.cpp")
set(SHADERS "./shaders/NaiveSphere.hlsl")
create_app(TXNaiveSphereWithMeshShaderWithMultipleThreads "${SOURCES}" "${SHADERS}")
