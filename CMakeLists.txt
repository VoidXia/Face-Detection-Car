cmake_minimum_required( VERSION 2.8 )
project( example )
find_package(X11 REQUIRED)

message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
message("OPENMP FOUND")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} 
${OpenMP_EXE_LINKER_FLAGS}")
endif()


find_package( OpenCV REQUIRED )
find_package(facedetection)
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")
message(STATUS "facelib? = ${facedetection_FOUND}")
message(STATUS "include_DIRS? = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "FACE_DIR = ${facedetection_DIR}")
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories(${X11_INCLUDE_DIR})

find_library(GPIOD_LIBRARY NAMES libgpiod.so)


add_executable( detect-image detect-image.cpp )
add_executable( detect-camera detect-camera.cpp )
add_executable( benchmark benchmark.cpp )
add_executable( test test.cpp )
add_executable( tracking tracking.cpp )
target_link_libraries( detect-image ${OpenCV_LIBS} )
target_link_libraries( benchmark ${OpenCV_LIBS} )
target_link_libraries( test ${OpenCV_LIBS} )
target_link_libraries( tracking ${OpenCV_LIBS} )
target_link_libraries( detect-image 
/home/pi/car/libfacedetection/build/install/lib/libfacedetection.so )
target_link_libraries( benchmark 
/home/pi/car/libfacedetection/build/install/lib/libfacedetection.so )
target_link_libraries( test 
/home/pi/car/libfacedetection/build/install/lib/libfacedetection.so )
target_link_libraries( tracking 
/home/pi/car/libfacedetection/build/install/lib/libfacedetection.so )
target_link_libraries( tracking ${X11_LIBRARIES})
target_link_libraries( tracking ${GPIOD_LIBRARY})


#target_link_libraries( detect-image 
/opencv/build/lib/libopencv_highgui.so )
#target_link_libraries( detect-image 
/opencv/build/lib/libopencv_imgproc.so )
#target_link_libraries( detect-image /opencv/build/lib/libopencv_core.so )
#target_link_libraries( detect-image 
/opencv/build/lib/libopencv_imgcodecs.so )
target_link_libraries( detect-camera ${OpenCV_LIBS} )
target_link_libraries( detect-camera 
/home/pi/car/libfacedetection/build/install/lib/libfacedetection.so )
#target_link_libraries( detect-camera 
/opencv/build/lib/libopencv_highgui.so )
#target_link_libraries( detect-camera /opencv/build/lib/libopencv_video.so 
)
#target_link_libraries( detect-camera 
/opencv/build/lib/libopencv_imgproc.so )
#target_link_libraries( detect-camera /opencv/build/lib/libopencv_core.so 
)
#target_link_libraries( detect-camera 
/opencv/build/lib/libopencv_videoio.so )
