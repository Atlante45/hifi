set(TARGET_NAME shared)

# TODO: there isn't really a good reason to have Script linked here - let's get what is requiring it out (RegisteredMetaTypes.cpp)
setup_hifi_library(Gui Network Script Widgets)

if (WIN32)
    target_link_libraries(${TARGET_NAME} Wbemuuid.lib)
endif()

target_zlib()
target_nsight()

add_dependency_external_projects(tbb)
find_package(TBB REQUIRED)

# append OpenSSL to our list of libraries to link
target_link_libraries(${TARGET_NAME} ${TBB_LIBRARIES})

# append tbb includes to our list of includes to bubble
target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${TBB_INCLUDE_DIRS})
