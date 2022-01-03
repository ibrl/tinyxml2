project(IdNbr891)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(IdNbr891
        main.cpp
        )

target_link_libraries(
        IdNbr891
        tinyxml2
        )
