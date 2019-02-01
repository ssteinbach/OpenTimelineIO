install(
    DIRECTORY contrib/opentimelineio_contrib/
    DESTINATION contrib/opentimelineio_contrib
    FILES_MATCHING PATTERN "*.py"
                   PATTERN "*.json")
