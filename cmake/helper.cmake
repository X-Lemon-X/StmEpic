

# Function to read version from version.txt
function(get_version_from_file output_var)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/cmake/version.txt" version_content)
  string(STRIP "${version_content}" version_content) # Remove any extra whitespace
  set(${output_var} "${version_content}" PARENT_SCOPE)
endfunction()


# Function to get commit number and hash if the project is a submodule
function(get_git_commit_info output_commit output_hash)
  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
    execute_process(
      COMMAND git rev-list --count HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
      OUTPUT_VARIABLE commit_number
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND git rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
      OUTPUT_VARIABLE commit_hash
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${output_commit} "${commit_number}" PARENT_SCOPE)
    set(${output_hash} "${commit_hash}" PARENT_SCOPE)
  else()
    message(WARNING "The project does not appear to be a Git repository.")
    set(${output_commit} "N/A" PARENT_SCOPE)
    set(${output_hash} "N/A" PARENT_SCOPE)
  endif()
endfunction()