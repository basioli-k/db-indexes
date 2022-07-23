

find_package( Boost 1.78 COMPONENTS REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )

add_executable(main main.cpp)

target_link_libraries( main LINK_PUBLIC ${Boost_LIBRARIES} )
add_compile_definitions(NOMINMAX)

set_target_properties(
    main
    PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS ON
    CXX_STANDARD_REQUIRED ON
  )


