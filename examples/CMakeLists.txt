# Examples configuration

# Basic example - standard Cayene LPP types
add_executable(basic_example
    basic_example.cpp
)

target_link_libraries(basic_example
    PRIVATE
        cayene::decoder
        cayene_warnings
)

# Advanced example - custom types
add_executable(advanced_example
    advanced_example.cpp
)

target_link_libraries(advanced_example
    PRIVATE
        cayene::decoder
        cayene_warnings
)
