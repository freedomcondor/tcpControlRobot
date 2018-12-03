cmake_minimum_required(VERSION 3.1)

project(speedTest)

#find_package(OpenGL REQUIRED)
#${OPENGL_INCLUDE_DIRS}
#${OPENGL_LIBRARIES}
#find_package(GLUT REQUIRED)
#${GLUT_INCLUDE_DIRS}
#${GLUT_LIBRARIES}


add_executable(tcpControl
				src/main.cpp
				src/function.cpp
               	src/packet_control_interface.cc
               	src/uart_socket.cc
				src/tcp_socket.cpp
              )

set_property(TARGET tcpControl PROPERTY CXX_STANDARD 11)

install(TARGETS tcpControl DESTINATION bin)




