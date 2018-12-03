SUMMARY = "control robot speed from PC"
LICENSE = "LGPLv2.1"
LIC_FILES_CHKSUM = "file://LICENSE;md5=171524cfe8840be5abf2e6ee7bebdc24"

inherit cmake


PR = "r0"
PV = "1.0"

SRC_URI = "file://src \
           file://CMakeLists.txt \
           file://LICENSE \
          "
          
S = "${WORKDIR}"
