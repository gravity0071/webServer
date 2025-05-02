# CMakeList.txt : CMake project for webServer, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (webServer "webServer.cpp" "webServer.h" "buffer.h" "WorkerThread.cpp" "Iocp.h" "Iocp.cpp" "loadBalancer.h" "loadBlalancer.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET webServer PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
