

# Function to read version from version.txt
function(get_version_from_file output_var_major output_var_minor)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/cmake/version_major.txt" version_content)
  string(STRIP "${version_content}" version_content) # Remove any extra whitespace
  set(${output_var_major} "${version_content}" PARENT_SCOPE)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/cmake/version_minor.txt" version_content)
  string(STRIP "${version_content}" version_content) # Remove any extra whitespace
  set(${output_var_minor} "${version_content}" PARENT_SCOPE)
endfunction()


# Function to get commit number and hash if the project is a submodule
function(get_git_commit_info current_version)
  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
    execute_process(
      COMMAND git rev-list --count HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
      OUTPUT_VARIABLE current_version_val
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${current_version} "${current_version_val}" PARENT_SCOPE)
  else()
    message(WARNING "The project does not appear to be a Git repository.")
    set(${current_version} "N/A" PARENT_SCOPE)
  endif()
endfunction()


# Function to update version files if the new version is greater than the current version
function(update_version_files major_version minor_version)
  get_version_from_file(current_major current_minor)

  if((major_version GREATER current_major) OR
     (major_version EQUAL current_major AND minor_version GREATER current_minor))
    file(WRITE "${CMAKE_CURRENT_LIST_DIR}/cmake/version_major.txt" "${major_version}")
    file(WRITE "${CMAKE_CURRENT_LIST_DIR}/cmake/version_minor.txt" "${minor_version}")
    message(STATUS "Version files updated to ${major_version}.${minor_version}")
    replace_project_number("${CMAKE_CURRENT_LIST_DIR}/Doxyfile" "${major_version}.${minor_version}")
  else()
    message(STATUS "Current version (${current_major}.${current_minor}) is greater or equal to the new version (${major_version}.${minor_version}). No update performed.")
  endif()
endfunction()


# Function to replace a line in a file
function(replace_project_number file_path new_value)
  file(READ "${file_path}" file_content)
  string(REGEX REPLACE "PROJECT_NUMBER=[0-9]+\\.[0-9]+" "PROJECT_NUMBER=${new_value}" updated_content "${file_content}")
  file(WRITE "${file_path}" "${updated_content}")
endfunction()