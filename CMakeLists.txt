cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -m64 -Wall -Wextra -lvulkan")

project(RayTrace)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${Vulkan_INCLUDE_DIRS})

file(GLOB_RECURSE PROJECT_SRC
	"./src/**.h"
	"./src/**.cpp"
)

string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
string(STRIP "${Vulkan_LIBRARIES}" Vulkan_LIBRARIES)

set(GLSLC_FLAGS --target-spv=spv1.5 --target-env=vulkan1.2)
macro(add_shader input_shader output_shader)
	add_custom_command(
		OUTPUT ${output_shader}
		COMMAND glslc ${GLSLC_FLAGS} ${input_shader} -o ${output_shader}
		DEPENDS ${input_shader}
	)
	list(APPEND SHADER ${output_shader})
endmacro()

add_shader(${CMAKE_CURRENT_SOURCE_DIR}/res/shader/prakt_raygen.rgen        prakt_raygen.spv     )
add_shader(${CMAKE_CURRENT_SOURCE_DIR}/res/shader/prakt_closesthit.rchit   prakt_closesthit.spv )
add_shader(${CMAKE_CURRENT_SOURCE_DIR}/res/shader/prakt_miss.rmiss         prakt_miss.spv       )
add_shader(${CMAKE_CURRENT_SOURCE_DIR}/res/shader/prakt_shadow.rmiss       prakt_shadow.spv     )

add_shader(${CMAKE_CURRENT_SOURCE_DIR}/res/shader/monte_carlo_light_raygen.rgen      monte_carlo_light_raygen.spv     )
add_shader(${CMAKE_CURRENT_SOURCE_DIR}/res/shader/monte_carlo_light_miss.rmiss       monte_carlo_light_miss.spv       )
add_shader(${CMAKE_CURRENT_SOURCE_DIR}/res/shader/monte_carlo_light_closesthit.rchit monte_carlo_light_closesthit.spv )
add_shader(${CMAKE_CURRENT_SOURCE_DIR}/res/shader/monte_carlo_kd_raygen.rgen         monte_carlo_kd_raygen.spv        )

add_executable(RayTrace ${PROJECT_SRC} ${SHADER})

target_link_libraries(RayTrace ${SDL2_LIBRARIES})
target_link_libraries(RayTrace ${Vulkan_LIBRARIES})
