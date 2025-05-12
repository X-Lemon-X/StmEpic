

# Function to read version from version.txt
function(get_version_from_file output_var_major output_var_minor)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/version_major.txt" version_content)
  string(STRIP "${version_content}" version_content) # Remove any extra whitespace
  set(${output_var_major} "${version_content}" PARENT_SCOPE)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/version_minor.txt" version_content)
  string(STRIP "${version_content}" version_content) # Remove any extra whitespace
  set(${output_var_minor} "${version_content}" PARENT_SCOPE)
endfunction()


# Function to get commit number and hash if the project is a submodule
function(get_git_commit_info current_version)
  # if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
    execute_process(
      COMMAND git rev-list --count HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
      OUTPUT_VARIABLE current_version_val
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${current_version} "${current_version_val}" PARENT_SCOPE)
  # else()
  #   message(WARNING "StmEpic | The project does not appear to be a Git repository.")
  #   set(${current_version} "N/A" PARENT_SCOPE)
  # endif()
endfunction()


# Function to update version files if the new version is greater than the current version
function(update_version_files major_version minor_version)
  get_version_from_file(current_major current_minor)

  if((major_version GREATER current_major) OR
     (major_version EQUAL current_major AND minor_version GREATER current_minor))
    file(WRITE "${CMAKE_CURRENT_LIST_DIR}/version_major.txt" "${major_version}")
    file(WRITE "${CMAKE_CURRENT_LIST_DIR}/version_minor.txt" "${minor_version}")
    # message(STATUS "StmEpic | Version files updated to ${major_version}.${minor_version}")
    replace_project_number("${CMAKE_CURRENT_LIST_DIR}/../Doxyfile" "${major_version}.${minor_version}")
  endif()
  # else()
    # message(STATUS "StmEpic | Current version (${current_major}.${current_minor}) is greater or equal to the new version (${major_version}.${minor_version}). No update performed.")
endfunction()


# Function to replace a line in a file
function(replace_project_number file_path new_value)
  file(READ "${file_path}" file_content)
  string(REGEX REPLACE "PROJECT_NUMBER=[0-9]+\\.[0-9]+" "PROJECT_NUMBER=${new_value}" updated_content "${file_content}")
  file(WRITE "${file_path}" "${updated_content}")
endfunction()

function(check_min_version_required)
  if(STMEPIC_MIN_VERSION_REQUIRED_ENABLED)
    string(REPLACE "." ";" VERSION_PARTS "${STMEPIC_MIN_VERSION_REQUIRED}")
    list(GET VERSION_PARTS 0 MIN_VERSION_MAJOR)
    list(GET VERSION_PARTS 1 MIN_VERSION_MINOR)

    if((STMEPIC_VERSION_MAJOR LESS MIN_VERSION_MAJOR) OR
       (STMEPIC_VERSION_MAJOR EQUAL MIN_VERSION_MAJOR AND STMEPIC_VERSION_MINOR LESS MIN_VERSION_MINOR))
      message(FATAL_ERROR "StmEpic | StmEpic version ${STMEPIC_VERSION_MAJOR}.${STMEPIC_VERSION_MINOR} is less than the required version ${MIN_VERSION_MAJOR}.${MIN_VERSION_MINOR}.")
    else()
      message(STATUS "StmEpic | Version ${STMEPIC_VERSION_MAJOR}.${STMEPIC_VERSION_MINOR} meets the minimum required version.")
    endif()
  endif()
endfunction()


function(post_build_function_run )

  get_version_from_file(STMEPIC_VERSION_MAJOR STMEPIC_VERSION_MINOR)
  get_git_commit_info(STMEPIC_HEAD_VERSION)
  message(STATUS "StmEpic | Version (main): ${STMEPIC_VERSION_MAJOR}.${STMEPIC_VERSION_MINOR}")
  message(STATUS "StmEpic | Version (current): ${STMEPIC_VERSION_MAJOR}.${STMEPIC_HEAD_VERSION}")
  update_version_files(${STMEPIC_VERSION_MAJOR} ${STMEPIC_HEAD_VERSION})
  check_min_version_required()

endfunction(post_build_function_run )



post_build_function_run()