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

add_shader(monte_carlo_miss.spv          monte_carlo_miss.rmiss         monte_carlo.glsl )
add_shader(monte_carlo_closesthit.spv    monte_carlo_closesthit.rchit   monte_carlo.glsl )
add_shader(monte_carlo_light_raygen.spv  monte_carlo_light_raygen.rgen  monte_carlo.glsl )
add_shader(monte_carlo_kd_comp.spv       monte_carlo_kd_comp.comp       monte_carlo.glsl )
add_shader(monte_carlo_vision_raygen.spv monte_carlo_vision_raygen.rgen monte_carlo.glsl )
add_shader(monte_carlo_final_comp.spv    monte_carlo_final_comp.comp    monte_carlo.glsl )

add_shader(ddgi_miss.spv                ddgi_miss.rmiss               ddgi.glsl )
add_shader(ddgi_closesthit.spv          ddgi_closesthit.rchit         ddgi.glsl )
add_shader(ddgi_probe_raygen.spv        ddgi_probe_raygen.rgen        ddgi.glsl )
add_shader(ddgi_shading_update_comp.spv ddgi_shading_update_comp.comp ddgi.glsl )
add_shader(ddgi_final_raygen.spv        ddgi_final_raygen.rgen        ddgi.glsl )

add_shader(denoise_gauss_comp.spv  denoise_gauss_comp.comp  )
add_shader(denoise_median_comp.spv denoise_median_comp.comp )

add_shader(denoise_m17_temporal_comp.spv       denoise_m17_temporal_comp.comp       denoise_m17.glsl )
add_shader(denoise_m17_pre_bilateral_comp.spv  denoise_m17_pre_bilateral_comp.comp  denoise_m17.glsl )
add_shader(denoise_m17_firefly_comp.spv        denoise_m17_firefly_comp.comp        denoise_m17.glsl )
add_shader(denoise_m17_disocclusion_comp.spv   denoise_m17_disocclusion_comp.comp   denoise_m17.glsl )
add_shader(denoise_m17_post_bilateral_comp.spv denoise_m17_post_bilateral_comp.comp denoise_m17.glsl )

add_shader(meta_renderer_miss.spv       meta_renderer_miss.rmiss       meta_renderer.glsl )
add_shader(meta_renderer_closesthit.spv meta_renderer_closesthit.rchit meta_renderer.glsl )
add_shader(meta_renderer_raygen.spv     meta_renderer_raygen.rgen      meta_renderer.glsl )


add_executable(RayTrace ${PROJECT_SRC} ${SHADER})

target_link_libraries(RayTrace ${SDL2_LIBRARIES})
target_link_libraries(RayTrace ${Vulkan_LIBRARIES})
