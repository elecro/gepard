include(OptionMacros)

ADD_CHOICE (BACKEND "Backend to use" "GLES2 SOFTWARE VULKAN" GLES2)
ADD_OPTION (GD_LOG_LEVEL "Print log messages during execution" 0)
ADD_OPTION (GD_DISABLE_LOG_COLORS "Do not color log messages" OFF)
