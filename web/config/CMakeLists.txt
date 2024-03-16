aux_source_directory(../third_party THIRD_PARTY_LIST)
aux_source_directory(. CONFIG_LIST)

add_library(HConfig STATIC ${CONFIG_LIST})
