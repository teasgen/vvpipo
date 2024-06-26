add_executable(osm_tool_closed_way_filter osm_tool_closed_way_filter.cpp)
target_link_libraries(osm_tool_closed_way_filter ${OSMIUM_LIBRARIES})
set_pthread_on_target(osm_tool_closed_way_filter)
install(TARGETS osm_tool_closed_way_filter DESTINATION bin)

add_executable(osm_tool_closed_way_tags osm_tool_closed_way_tags.cpp)
target_link_libraries(osm_tool_closed_way_tags ${OSMIUM_LIBRARIES})
set_pthread_on_target(osm_tool_closed_way_tags)
install(TARGETS osm_tool_closed_way_tags DESTINATION bin)

add_executable(osm_tool_complex_areas osm_tool_complex_areas.cpp)
target_link_libraries(osm_tool_complex_areas ${OSMIUM_IO_LIBRARIES})
set_pthread_on_target(osm_tool_complex_areas)
install(TARGETS osm_tool_complex_areas DESTINATION bin)

add_executable(osm_tool_create_areas osm_tool_create_areas.cpp osm_tool.cpp)
target_link_libraries(osm_tool_create_areas ${OSMIUM_LIBRARIES})
set_pthread_on_target(osm_tool_create_areas)
install(TARGETS osm_tool_create_areas DESTINATION bin)

add_executable(osm_tool_failed_area_tags osm_tool_failed_area_tags.cpp osm_tool.cpp)
target_link_libraries(osm_tool_failed_area_tags ${OSMIUM_LIBRARIES})
set_pthread_on_target(osm_tool_failed_area_tags)
install(TARGETS osm_tool_failed_area_tags DESTINATION bin)

add_executable(osm_tool_find_problems osm_tool_find_problems.cpp)
target_link_libraries(osm_tool_find_problems ${OSMIUM_IO_LIBRARIES})
set_pthread_on_target(osm_tool_find_problems)
install(TARGETS osm_tool_find_problems DESTINATION bin)

add_executable(osm_tool_large_areas osm_tool_large_areas.cpp)
target_link_libraries(osm_tool_large_areas ${OSMIUM_IO_LIBRARIES} sqlite3)
set_pthread_on_target(osm_tool_large_areas)
install(TARGETS osm_tool_large_areas DESTINATION bin)

add_executable(osm_tool_mercator osm_tool_mercator.cpp osm_tool.cpp)
target_link_libraries(osm_tool_mercator ${OSMIUM_LIBRARIES})
set_pthread_on_target(osm_tool_mercator)
install(TARGETS osm_tool_mercator DESTINATION bin)

add_executable(osm_tool_problem_report osm_tool_problem_report.cpp osm_tool.cpp)
target_link_libraries(osm_tool_problem_report ${OSMIUM_LIBRARIES})
set_pthread_on_target(osm_tool_problem_report)
install(TARGETS osm_tool_problem_report DESTINATION bin)

add_executable(osm_tool_sizes osm_tool_sizes.cpp)
install(TARGETS osm_tool_sizes DESTINATION bin)

add_executable(osm_tool_stats osm_tool_stats.cpp)
target_link_libraries(osm_tool_stats ${OSMIUM_IO_LIBRARIES} sqlite3)
set_pthread_on_target(osm_tool_stats)
install(TARGETS osm_tool_stats DESTINATION bin)
