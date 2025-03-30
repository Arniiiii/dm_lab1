# Codestyle

1. Include header sorting like in Python: ( use clang-format )
    1. System or compiler's headers, STL
    2. Other libs
    3. Our libs
2. Variables `snake_case`
	1. Global vars `g_snake_case`
	2. Static_vars `s_snake_case`  
3. Class's vars
	1. Private: snake_case_pri
	2. Protected: snake_case_pro (notice underscore in the end)
	3. Public: snake_case
4. Macros `SCREAMING_SNAKE_CASE`
5. Class names `PascalCase`
	1. CRTP interfaces
		1. Old CRTP : `CRTP_PascalCase`
		2. With deducing this : `CRTPDT_PascalCase`
	2. Virtual interfaces
		1. `I_PascalCase`
	3. Concepts
		1. `Con_PascalCase`
		2. Implementators of a concept `ConImpl_PascalCase`
    4. Old interfaces via std::enable_if and/or SFINAE
        1. `OI_PascalCase`
        2. `OImpl_PascalCase` for implementation.
6. Function names `camelCase`
	1. Class's method `cm_camelCase`
	2. Class's static method `scm_camelCase`
7. Enums `E_camelCase`
    1. Enum classes `Ec_camelCase`
8. Filenames
    1. Compilation units `snake_case.cpp`
    2. Header files `snake_case.hpp`
9. Types
    1. "Global" types `PascalCase_t`
