message(STATUS "downloading...
     src='https://googlemock.googlecode.com/files/gmock-1.7.0.zip'
     dst='/Users/anubhav/TextDB/cmake/gmock_ext-prefix/src/gmock-1.7.0.zip'
     timeout='none'")




file(DOWNLOAD
  "https://googlemock.googlecode.com/files/gmock-1.7.0.zip"
  "/Users/anubhav/TextDB/cmake/gmock_ext-prefix/src/gmock-1.7.0.zip"
  SHOW_PROGRESS
  # no TIMEOUT
  STATUS status
  LOG log)

list(GET status 0 status_code)
list(GET status 1 status_string)

if(NOT status_code EQUAL 0)
  message(FATAL_ERROR "error: downloading 'https://googlemock.googlecode.com/files/gmock-1.7.0.zip' failed
  status_code: ${status_code}
  status_string: ${status_string}
  log: ${log}
")
endif()

message(STATUS "downloading... done")
