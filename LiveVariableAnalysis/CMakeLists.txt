add_library(LVAPass MODULE
    LVA.cpp
)
set_target_properties(LVAPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti -std=c++14 -g"
)
if(APPLE)
	set_target_properties(LVAPass PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
