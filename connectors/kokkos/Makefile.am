AM_CPPFLAGS = -I$(top_srcdir)/include -I$(srcdir)/include
AM_CXXFLAGS = -Wall -Wextra -Wpedantic

if STRICT
AM_CXXFLAGS += -Werror
endif

pkglib_LTLIBRARIES = ccs-kokkos-connector.la

ccs_kokkos_connector_la_SOURCES = \
	ccs-kokkos-connector.cpp \
	include/impl/Kokkos_Profiling_Interface.hpp \
	include/impl/Kokkos_Profiling_C_Interface.h \
	include/impl/Kokkos_Profiling_DeviceInfo.hpp

ccs_kokkos_connector_la_LDFLAGS = -module -avoid-version ../../src/libcconfigspace.la
