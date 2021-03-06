SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "")

SET(CMAKE_CXX_FLAGS "/W3 /GR /EHsc /fp:strict /fp:except- /wd4503 /bigobj /MD" CACHE STRING "")

SET(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "")

SET(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DCGAL_NDEBUG" CACHE STRING "")

SET(WITH_CGAL_Core ON CACHE BOOL "")

SET(WITH_CGAL_ImageIO ON CACHE BOOL "")

SET(WITH_CGAL_Qt5 ON CACHE BOOL "")

SET(WITH_GMP ON CACHE BOOL "")

SET(WITH_GMPXX OFF CACHE BOOL "")

SET(WITH_MPFR ON CACHE BOOL "")

SET(WITH_NTL OFF CACHE BOOL "")

SET(WITH_demos ON CACHE BOOL "")

SET(CGAL_HEADER_ONLY ON CACHE BOOL "")
