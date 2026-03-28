function(join output)
  set(result "")

  foreach(part IN LISTS ARGN)
    string(APPEND result "${part}")
  endforeach()

  set(${output} "${result}" PARENT_SCOPE)
endfunction()
