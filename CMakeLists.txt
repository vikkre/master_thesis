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
macro(add_shader output_shader)
	set (input_files ${ARGN})
	list(TRANSFORM input_files PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/res/shader/)
	list(GET input_files 0 input_shader)

	add_custom_command(
		OUTPUT ${output_shader}
		COMMAND glslc ${GLSLC_FLAGS} ${input_shader} -o ${output_shader}
		DEPENDS ${input_files}
	)
	list(APPEND SHADER ${output_shader})
endmacro()

set(SHADER_INCLUDES helper.glsl v1.glsl)
add_shader(v1_miss.spv       v1_miss.rmiss       v1.glsl )
add_shader(v1_closesthit.spv v1_closesthit.rchit v1.glsl )

set(PATH_TRACER_INCLUDES ${SHADER_INCLUDES} path_tracer.glsl)
add_shader(path_tracer_light_raygen.spv  path_tracer_light_raygen.rgen  ${PATH_TRACER_INCLUDES} )
add_shader(path_tracer_kd_comp.spv       path_tracer_kd_comp.comp       ${PATH_TRACER_INCLUDES} )
add_shader(path_tracer_vision_raygen.spv path_tracer_vision_raygen.rgen ${PATH_TRACER_INCLUDES} )
add_shader(path_tracer_final_comp.spv    path_tracer_final_comp.comp    ${PATH_TRACER_INCLUDES} )

set(MAJERCIK2019_INCLUDES ${SHADER_INCLUDES} majercik2019.glsl)
add_shader(majercik2019_probe_raygen.spv        majercik2019_probe_raygen.rgen        ${MAJERCIK2019_INCLUDES} )
add_shader(majercik2019_shading_update_comp.spv majercik2019_shading_update_comp.comp ${MAJERCIK2019_INCLUDES} )
add_shader(majercik2019_final_raygen.spv        majercik2019_final_raygen.rgen        ${MAJERCIK2019_INCLUDES} )

set(META_RENDERER_INCLUDES ${SHADER_INCLUDES} meta_renderer.glsl)
add_shader(meta_renderer_raygen.spv meta_renderer_raygen.rgen ${META_RENDERER_INCLUDES} )

set(PHONG_RENDERER_INCLUDES ${SHADER_INCLUDES} phong_renderer.glsl)
add_shader(phong_renderer_raygen.spv phong_renderer_raygen.rgen ${PHONG_RENDERER_INCLUDES} )

set(BITTERLI2020_RENDERER_INCLUDES ${SHADER_INCLUDES} bitterli2020.glsl)
add_shader(bitterli2020_vision_raygen.spv bitterli2020_vision_raygen.rgen ${BITTERLI2020_RENDERER_INCLUDES} )

add_shader(denoise_gauss_comp.spv  denoise_gauss_comp.comp  )
add_shader(denoise_median_comp.spv denoise_median_comp.comp )


add_executable(RayTrace ${PROJECT_SRC} ${SHADER})

target_link_libraries(RayTrace ${SDL2_LIBRARIES})
target_link_libraries(RayTrace ${Vulkan_LIBRARIES})
