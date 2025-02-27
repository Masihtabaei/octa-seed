include("../../CreateApp.cmake")
set(SOURCES "./src/FruitsGenerator.cpp")
set(SHADERS "./shaders/Fruits.hlsl")
create_app(AXFruitsGenerator "${SOURCES}" "${SHADERS}")
